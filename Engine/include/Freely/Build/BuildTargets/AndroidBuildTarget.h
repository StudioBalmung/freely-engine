#pragma once

#include "Freely/Build/BuildTarget.h"
#include "Freely/Core/Logger.h"
#include <cstdlib>

namespace Freely {

class AndroidBuildTarget : public IBuildTarget {
public:
    const char* GetName() const override { return "Android ARM64"; }
    TargetPlatform GetPlatform() const override { return TargetPlatform::Android; }
    TargetArchitecture GetArchitecture() const override { return TargetArchitecture::ARM64; }

    bool IsToolchainAvailable() const override {
        // Check for ANDROID_NDK_HOME environment variable
        const char* ndkHome = std::getenv("ANDROID_NDK_HOME");
        return ndkHome != nullptr && std::filesystem::exists(ndkHome);
    }

    std::string GetToolchainRequirements() const override {
        return "Requires Android NDK. Set ANDROID_NDK_HOME environment variable.\n"
               "Download from https://developer.android.com/ndk/downloads";
    }

    bool Configure(const BuildConfig& config, BuildProgress& progress) override {
        const char* ndkHome = std::getenv("ANDROID_NDK_HOME");
        if (!ndkHome) {
            progress.ErrorMessage = "ANDROID_NDK_HOME not set";
            return false;
        }

        m_BuildDir = config.OutputDirectory / "build_android";
        std::filesystem::create_directories(m_BuildDir);

        auto toolchainFile = std::filesystem::path(ndkHome) / "build" / "cmake" / "android.toolchain.cmake";
        if (!std::filesystem::exists(toolchainFile)) {
            progress.ErrorMessage = "Android NDK toolchain file not found at: " + toolchainFile.string();
            return false;
        }

        std::string cmakeCmd = "cmake";
        cmakeCmd += " -S \"" + config.ProjectRoot.string() + "\"";
        cmakeCmd += " -B \"" + m_BuildDir.string() + "\"";
        cmakeCmd += " -DCMAKE_TOOLCHAIN_FILE=\"" + toolchainFile.string() + "\"";
        cmakeCmd += " -DANDROID_ABI=arm64-v8a";
        cmakeCmd += " -DANDROID_PLATFORM=android-26";
        cmakeCmd += " -DANDROID_STL=c++_shared";
        cmakeCmd += " -DCMAKE_BUILD_TYPE=";
        cmakeCmd += (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        cmakeCmd += " -DFREELY_BUILD_EDITOR=OFF -DFREELY_BUILD_RUNTIME=ON";

        FL_ENGINE_INFO("[Android] CMake configure: {}", cmakeCmd);
        int result = std::system(cmakeCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "CMake configure failed for Android";
            return false;
        }
        return true;
    }

    bool Build(const BuildConfig& config, BuildProgress& progress,
               BuildProgress::ProgressCallback callback) override {
        std::string buildCmd = "cmake --build \"" + m_BuildDir.string() + "\" --parallel";

        FL_ENGINE_INFO("[Android] Build: {}", buildCmd);
        int result = std::system(buildCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "Android build failed";
            return false;
        }

        if (callback) { BuildProgress p; p.Percentage = 1.0f; callback(p); }
        return true;
    }

    bool Package(const BuildConfig& config, BuildProgress& progress,
                 BuildProgress::ProgressCallback callback) override {
        // APK packaging requires Gradle and the Android SDK
        // For now, we just copy the .so library
        auto outputDir = config.OutputDirectory / (config.AppName + "_android");
        std::filesystem::create_directories(outputDir);
        std::filesystem::create_directories(outputDir / "lib" / "arm64-v8a");

        auto soPath = m_BuildDir / "lib" / ("lib" + config.AppName + ".so");
        if (std::filesystem::exists(soPath)) {
            std::filesystem::copy(soPath, outputDir / "lib" / "arm64-v8a" / ("lib" + config.AppName + ".so"),
                                  std::filesystem::copy_options::overwrite_existing);
        }

        FL_ENGINE_INFO("[Android] Packaged to: {}", outputDir.string());
        FL_ENGINE_WARN("[Android] Full APK packaging requires Gradle integration (not yet implemented)");

        if (callback) { BuildProgress p; p.Percentage = 1.0f; callback(p); }
        return true;
    }

    void Clean(const BuildConfig& config) override {
        auto buildDir = config.OutputDirectory / "build_android";
        if (std::filesystem::exists(buildDir)) std::filesystem::remove_all(buildDir);
    }

    std::filesystem::path GetOutputPath(const BuildConfig& config) const override {
        return config.OutputDirectory / (config.AppName + "_android") / "lib" / "arm64-v8a" / ("lib" + config.AppName + ".so");
    }

private:
    std::filesystem::path m_BuildDir;
};

} // namespace Freely
