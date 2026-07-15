#pragma once

#include "Freely/Build/BuildTarget.h"
#include "Freely/Core/Logger.h"
#include <cstdlib>

namespace Freely {

class WindowsBuildTarget : public IBuildTarget {
public:
    const char* GetName() const override { return "Windows x64"; }
    TargetPlatform GetPlatform() const override { return TargetPlatform::Windows; }
    TargetArchitecture GetArchitecture() const override { return TargetArchitecture::x86_64; }

    bool IsToolchainAvailable() const override {
#ifdef _WIN32
        // Check for MSVC or MinGW
        int result = std::system("cmake --version > NUL 2>&1");
        return result == 0;
#else
        // Cross-compilation from Linux/macOS would need MinGW
        int result = std::system("x86_64-w64-mingw32-g++ --version > /dev/null 2>&1");
        return result == 0;
#endif
    }

    std::string GetToolchainRequirements() const override {
        return "Requires CMake and either MSVC (Visual Studio Build Tools) or MinGW-w64.";
    }

    bool Configure(const BuildConfig& config, BuildProgress& progress) override {
        m_BuildDir = config.OutputDirectory / "build_windows";
        std::filesystem::create_directories(m_BuildDir);

        std::string cmakeCmd = "cmake";
        cmakeCmd += " -S \"" + config.ProjectRoot.string() + "\"";
        cmakeCmd += " -B \"" + m_BuildDir.string() + "\"";

#ifdef _WIN32
        cmakeCmd += " -G \"Visual Studio 17 2022\" -A x64";
#else
        cmakeCmd += " -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw-w64.cmake";
#endif

        cmakeCmd += " -DCMAKE_BUILD_TYPE=";
        cmakeCmd += (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";

        cmakeCmd += " -DFREELY_BUILD_EDITOR=OFF";
        cmakeCmd += " -DFREELY_BUILD_RUNTIME=ON";
        cmakeCmd += " -DFREELY_APP_NAME=\"" + config.AppName + "\"";

        for (auto& def : config.ExtraCMakeDefines) {
            cmakeCmd += " -D" + def;
        }

        FL_ENGINE_INFO("[Windows] CMake configure: {}", cmakeCmd);
        int result = std::system(cmakeCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "CMake configure failed (exit code " + std::to_string(result) + ")";
            return false;
        }

        return true;
    }

    bool Build(const BuildConfig& config, BuildProgress& progress,
               BuildProgress::ProgressCallback callback) override {
        std::string buildCmd = "cmake --build \"" + m_BuildDir.string() + "\"";
        buildCmd += " --config ";
        buildCmd += (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        buildCmd += " --parallel";

        FL_ENGINE_INFO("[Windows] Build: {}", buildCmd);
        int result = std::system(buildCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "Build failed (exit code " + std::to_string(result) + ")";
            return false;
        }

        if (callback) {
            BuildProgress p;
            p.Percentage = 1.0f;
            p.LastMessage = "Build completed";
            callback(p);
        }

        return true;
    }

    bool Package(const BuildConfig& config, BuildProgress& progress,
                 BuildProgress::ProgressCallback callback) override {
        // Copy executable + assets to output directory
        auto outputDir = config.OutputDirectory / config.AppName;
        std::filesystem::create_directories(outputDir);

        auto exeName = config.AppName + ".exe";
        auto buildConfig = (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        auto exePath = m_BuildDir / "bin" / buildConfig / exeName;

        if (std::filesystem::exists(exePath)) {
            std::filesystem::copy(exePath, outputDir / exeName,
                                  std::filesystem::copy_options::overwrite_existing);
        }

        // Copy cooked assets
        auto assetsSource = config.ProjectRoot / config.AppName;
        if (std::filesystem::exists(assetsSource)) {
            std::filesystem::copy(assetsSource, outputDir / "Assets",
                                  std::filesystem::copy_options::recursive |
                                  std::filesystem::copy_options::overwrite_existing);
        }

        FL_ENGINE_INFO("[Windows] Packaged to: {}", outputDir.string());

        if (callback) {
            BuildProgress p;
            p.Percentage = 1.0f;
            p.LastMessage = "Packaging completed";
            callback(p);
        }

        return true;
    }

    void Clean(const BuildConfig& config) override {
        auto buildDir = config.OutputDirectory / "build_windows";
        if (std::filesystem::exists(buildDir)) {
            std::filesystem::remove_all(buildDir);
        }
    }

    std::filesystem::path GetOutputPath(const BuildConfig& config) const override {
        return config.OutputDirectory / config.AppName / (config.AppName + ".exe");
    }

private:
    std::filesystem::path m_BuildDir;
};

} // namespace Freely
