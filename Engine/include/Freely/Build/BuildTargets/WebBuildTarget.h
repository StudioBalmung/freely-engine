#pragma once

#include "Freely/Build/BuildTarget.h"
#include "Freely/Core/Logger.h"
#include <cstdlib>

namespace Freely {

class WebBuildTarget : public IBuildTarget {
public:
    const char* GetName() const override { return "Web (WASM)"; }
    TargetPlatform GetPlatform() const override { return TargetPlatform::Web; }
    TargetArchitecture GetArchitecture() const override { return TargetArchitecture::WASM; }

    bool IsToolchainAvailable() const override {
        // Check for emcc (Emscripten compiler)
#ifdef _WIN32
        int result = std::system("emcc --version > NUL 2>&1");
#else
        int result = std::system("emcc --version > /dev/null 2>&1");
#endif
        return result == 0;
    }

    std::string GetToolchainRequirements() const override {
        return "Requires Emscripten SDK (emsdk). Install from https://emscripten.org/\n"
               "Ensure 'emcc' is on your PATH (run 'emsdk activate latest').";
    }

    bool Configure(const BuildConfig& config, BuildProgress& progress) override {
        m_BuildDir = config.OutputDirectory / "build_web";
        std::filesystem::create_directories(m_BuildDir);

        // Find emcmake
        std::string emcmake = "emcmake cmake";

        std::string cmakeCmd = emcmake;
        cmakeCmd += " -S \"" + config.ProjectRoot.string() + "\"";
        cmakeCmd += " -B \"" + m_BuildDir.string() + "\"";
        cmakeCmd += " -DCMAKE_BUILD_TYPE=";
        cmakeCmd += (config.Configuration == BuildConfiguration::Debug) ? "Debug" : "Release";
        cmakeCmd += " -DFREELY_BUILD_EDITOR=OFF -DFREELY_BUILD_RUNTIME=ON";

        // Emscripten-specific flags
        cmakeCmd += " -DCMAKE_EXECUTABLE_SUFFIX=\".html\"";

        FL_ENGINE_INFO("[Web] CMake configure: {}", cmakeCmd);
        int result = std::system(cmakeCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "Emscripten CMake configure failed";
            return false;
        }
        return true;
    }

    bool Build(const BuildConfig& config, BuildProgress& progress,
               BuildProgress::ProgressCallback callback) override {
        std::string buildCmd = "cmake --build \"" + m_BuildDir.string() + "\" --parallel";

        FL_ENGINE_INFO("[Web] Build: {}", buildCmd);
        int result = std::system(buildCmd.c_str());
        if (result != 0) {
            progress.ErrorMessage = "Web build failed";
            return false;
        }

        if (callback) { BuildProgress p; p.Percentage = 1.0f; callback(p); }
        return true;
    }

    bool Package(const BuildConfig& config, BuildProgress& progress,
                 BuildProgress::ProgressCallback callback) override {
        auto outputDir = config.OutputDirectory / (config.AppName + "_web");
        std::filesystem::create_directories(outputDir);

        // Emscripten outputs: .html, .js, .wasm, .data
        std::vector<std::string> extensions = {".html", ".js", ".wasm", ".data"};
        for (auto& ext : extensions) {
            auto src = m_BuildDir / "bin" / (config.AppName + ext);
            if (std::filesystem::exists(src)) {
                std::filesystem::copy(src, outputDir / (config.AppName + ext),
                                      std::filesystem::copy_options::overwrite_existing);
            }
        }

        FL_ENGINE_INFO("[Web] Packaged to: {}", outputDir.string());
        if (callback) { BuildProgress p; p.Percentage = 1.0f; callback(p); }
        return true;
    }

    void Clean(const BuildConfig& config) override {
        auto buildDir = config.OutputDirectory / "build_web";
        if (std::filesystem::exists(buildDir)) std::filesystem::remove_all(buildDir);
    }

    std::filesystem::path GetOutputPath(const BuildConfig& config) const override {
        return config.OutputDirectory / (config.AppName + "_web") / (config.AppName + ".html");
    }

private:
    std::filesystem::path m_BuildDir;
};

} // namespace Freely
