#pragma once

// Freely Engine - macOS Build Target
// Produces a macOS .app bundle for x86_64, ARM64 (Apple Silicon), or a
// universal binary (lipo'd from both architectures).

#include "Freely/Build/BuildTarget.h"
#include "Freely/Core/Logger.h"
#include <cstdlib>

namespace Freely {

class macOSBuildTarget : public IBuildTarget {
public:
    const char* GetName() const override { return "macOS (Universal)"; }
    TargetPlatform GetPlatform() const override { return TargetPlatform::macOS; }
    TargetArchitecture GetArchitecture() const override { return TargetArchitecture::ARM64; }

    bool IsToolchainAvailable() const override {
#ifdef __APPLE__
        // Require clang++ and cmake
        int hasCmake = std::system("cmake --version > /dev/null 2>&1");
        int hasClang = std::system("clang++ --version > /dev/null 2>&1");
        return hasCmake == 0 && hasClang == 0;
#else
        // Cross-compilation to macOS from other platforms is not supported.
        return false;
#endif
    }

    std::string GetToolchainRequirements() const override {
        return "Requires macOS with Xcode Command Line Tools (clang++) and CMake.\n"
               "Install with: xcode-select --install && brew install cmake";
    }

    bool Configure(const BuildConfig& config, BuildProgress& progress) override {
#ifndef __APPLE__
        progress.ErrorMessage = "macOS builds can only be performed on a macOS host.";
        return false;
#endif

        m_BuildDir = config.OutputDirectory / "build_macos";
        std::filesystem::create_directories(m_BuildDir);

        // Build for both x86_64 and arm64 as a universal binary
        std::string cmakeCmd = "cmake";
        cmakeCmd += " -S \"" + config.ProjectRoot.string() + "\"";
        cmakeCmd += " -B \"" + m_BuildDir.string() + "\"";

        // Prefer Xcode generator on macOS for proper .app bundle support
        cmakeCmd += " -G \"Xcode\"";
        cmakeCmd += " -DCMAKE_OSX_ARCHITECTURES=\"arm64;x86_64\"";
        cmakeCmd += " -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0";
        cmakeCmd += " -DCMAKE_BUILD_TYPE=";
        cmakeCmd += (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        cmakeCmd += " -DFREELY_BUILD_EDITOR=OFF";
        cmakeCmd += " -DFREELY_BUILD_RUNTIME=ON";
        cmakeCmd += " -DFREELY_APP_NAME=\"" + config.AppName + "\"";

        for (auto& def : config.ExtraCMakeDefines) {
            cmakeCmd += " -D" + def;
        }

        FL_ENGINE_INFO("[macOS] CMake configure: {}", cmakeCmd);
        int result = std::system(cmakeCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "CMake configure failed for macOS (exit " + std::to_string(result) + ")";
            return false;
        }
        return true;
    }

    bool Build(const BuildConfig& config, BuildProgress& progress,
               BuildProgress::ProgressCallback callback) override {
#ifndef __APPLE__
        progress.ErrorMessage = "macOS builds require a macOS host.";
        return false;
#endif

        const char* buildCfg = (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        std::string buildCmd = "cmake --build \"" + m_BuildDir.string() + "\"";
        buildCmd += " --config ";
        buildCmd += buildCfg;
        buildCmd += " --parallel";

        FL_ENGINE_INFO("[macOS] Build: {}", buildCmd);
        int result = std::system(buildCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "macOS build failed (exit " + std::to_string(result) + ")";
            return false;
        }

        if (callback) {
            BuildProgress p;
            p.Percentage   = 1.0f;
            p.LastMessage  = "macOS build completed";
            callback(p);
        }
        return true;
    }

    bool Package(const BuildConfig& config, BuildProgress& progress,
                 BuildProgress::ProgressCallback callback) override {
#ifndef __APPLE__
        progress.ErrorMessage = "macOS packaging requires a macOS host.";
        return false;
#endif

        const char* buildCfg = (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";

        // Locate the .app bundle produced by CMake / Xcode
        auto appBundleName = config.AppName + ".app";
        auto appPath       = m_BuildDir / "bin" / buildCfg / appBundleName;
        if (!std::filesystem::exists(appPath)) {
            // Xcode may put it one level up
            appPath = m_BuildDir / appBundleName;
        }

        auto outputDir = config.OutputDirectory / config.AppName;
        std::filesystem::create_directories(outputDir);

        if (std::filesystem::exists(appPath)) {
            auto destApp = outputDir / appBundleName;
            std::filesystem::copy(appPath, destApp,
                                  std::filesystem::copy_options::recursive |
                                  std::filesystem::copy_options::overwrite_existing);
            FL_ENGINE_INFO("[macOS] Copied .app bundle to: {}", destApp.string());
        } else {
            FL_ENGINE_WARN("[macOS] .app bundle not found at expected path: {}", appPath.string());
        }

        // Codesign with ad-hoc signature so the app runs without a developer cert
        auto signCmd = "codesign --force --deep --sign - \"" + (outputDir / appBundleName).string() + "\"";
        int signResult = std::system(signCmd.c_str());
        if (signResult != 0) {
            FL_ENGINE_WARN("[macOS] Ad-hoc codesigning failed - app may not run on Gatekeeper-enabled systems.");
        }

        // Optional: create a distributable .dmg
        auto dmgPath = outputDir.parent_path() / (config.AppName + ".dmg");
        std::string dmgCmd = "hdiutil create";
        dmgCmd += " -volname \"" + config.AppName + "\"";
        dmgCmd += " -srcfolder \"" + (outputDir / appBundleName).string() + "\"";
        dmgCmd += " -ov -format UDZO";
        dmgCmd += " \"" + dmgPath.string() + "\"";
        int dmgResult = std::system(dmgCmd.c_str());
        if (dmgResult == 0) {
            FL_ENGINE_INFO("[macOS] Created DMG: {}", dmgPath.string());
        } else {
            FL_ENGINE_WARN("[macOS] DMG creation failed - distributing raw .app instead.");
        }

        if (callback) {
            BuildProgress p;
            p.Percentage  = 1.0f;
            p.LastMessage = "macOS packaging completed";
            callback(p);
        }
        return true;
    }

    void Clean(const BuildConfig& config) override {
        if (std::filesystem::exists(m_BuildDir))
            std::filesystem::remove_all(m_BuildDir);
    }

    std::filesystem::path GetOutputPath(const BuildConfig& config) const override {
        return config.OutputDirectory / config.AppName / (config.AppName + ".app");
    }

private:
    std::filesystem::path m_BuildDir;
};

} // namespace Freely
