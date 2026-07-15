#pragma once

#include "Freely/Build/BuildTarget.h"
#include "Freely/Core/Logger.h"
#include <cstdlib>

namespace Freely {

class LinuxBuildTarget : public IBuildTarget {
public:
    const char* GetName() const override { return "Linux x64"; }
    TargetPlatform GetPlatform() const override { return TargetPlatform::Linux; }
    TargetArchitecture GetArchitecture() const override { return TargetArchitecture::x86_64; }

    bool IsToolchainAvailable() const override {
#ifdef __linux__
        int result = std::system("g++ --version > /dev/null 2>&1");
        return result == 0;
#elif defined(_WIN32)
        // WSL or cross-compilation
        int result = std::system("wsl g++ --version > NUL 2>&1");
        return result == 0;
#else
        return false;
#endif
    }

    std::string GetToolchainRequirements() const override {
        return "Requires CMake and GCC/Clang on Linux, or WSL with GCC on Windows.";
    }

    bool Configure(const BuildConfig& config, BuildProgress& progress) override {
        m_BuildDir = config.OutputDirectory / "build_linux";
        std::filesystem::create_directories(m_BuildDir);

        std::string cmakeCmd;
#ifdef _WIN32
        cmakeCmd = "wsl cmake";
#else
        cmakeCmd = "cmake";
#endif
        cmakeCmd += " -S \"" + config.ProjectRoot.string() + "\"";
        cmakeCmd += " -B \"" + m_BuildDir.string() + "\"";
        cmakeCmd += " -G \"Unix Makefiles\"";
        cmakeCmd += " -DCMAKE_BUILD_TYPE=";
        cmakeCmd += (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        cmakeCmd += " -DFREELY_BUILD_EDITOR=OFF -DFREELY_BUILD_RUNTIME=ON";

        FL_ENGINE_INFO("[Linux] CMake configure: {}", cmakeCmd);
        int result = std::system(cmakeCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "CMake configure failed";
            return false;
        }
        return true;
    }

    bool Build(const BuildConfig& config, BuildProgress& progress,
               BuildProgress::ProgressCallback callback) override {
        std::string buildCmd;
#ifdef _WIN32
        buildCmd = "wsl cmake";
#else
        buildCmd = "cmake";
#endif
        buildCmd += " --build \"" + m_BuildDir.string() + "\" --parallel";

        FL_ENGINE_INFO("[Linux] Build: {}", buildCmd);
        int result = std::system(buildCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "Build failed";
            return false;
        }

        if (callback) {
            BuildProgress p; p.Percentage = 1.0f;
            callback(p);
        }
        return true;
    }

    bool Package(const BuildConfig& config, BuildProgress& progress,
                 BuildProgress::ProgressCallback callback) override {
        auto outputDir = config.OutputDirectory / config.AppName;
        std::filesystem::create_directories(outputDir);

        auto exePath = m_BuildDir / "bin" / config.AppName;
        if (std::filesystem::exists(exePath)) {
            std::filesystem::copy(exePath, outputDir / config.AppName,
                                  std::filesystem::copy_options::overwrite_existing);
        }

        FL_ENGINE_INFO("[Linux] Packaged to: {}", outputDir.string());
        if (callback) { BuildProgress p; p.Percentage = 1.0f; callback(p); }
        return true;
    }

    void Clean(const BuildConfig& config) override {
        auto buildDir = config.OutputDirectory / "build_linux";
        if (std::filesystem::exists(buildDir)) std::filesystem::remove_all(buildDir);
    }

    std::filesystem::path GetOutputPath(const BuildConfig& config) const override {
        return config.OutputDirectory / config.AppName / config.AppName;
    }

private:
    std::filesystem::path m_BuildDir;
};

} // namespace Freely
