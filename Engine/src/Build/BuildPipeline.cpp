#include "Freely/Build/BuildPipeline.h"
#include "Freely/Build/BuildTargets/WindowsBuildTarget.h"
#include "Freely/Build/BuildTargets/LinuxBuildTarget.h"
#include "Freely/Build/BuildTargets/AndroidBuildTarget.h"
#include "Freely/Build/BuildTargets/WebBuildTarget.h"
#include "Freely/Build/BuildTargets/macOSBuildTarget.h"
#include "Freely/Core/Logger.h"

namespace Freely {

BuildPipeline::~BuildPipeline() {
    CancelBuild();
    if (m_BuildThread && m_BuildThread->joinable()) {
        m_BuildThread->join();
    }
}

void BuildPipeline::Init() {
    auto& registry = BuildTargetRegistry::Get();
    registry.Register(std::make_unique<WindowsBuildTarget>());
    registry.Register(std::make_unique<LinuxBuildTarget>());
    registry.Register(std::make_unique<macOSBuildTarget>());
    registry.Register(std::make_unique<AndroidBuildTarget>());
    registry.Register(std::make_unique<WebBuildTarget>());
    FL_ENGINE_INFO("Build pipeline initialized with {} targets", registry.GetAll().size());
}

void BuildPipeline::StartBuild(IBuildTarget* target, const BuildConfig& config) {
    if (m_IsBuilding.load()) {
        FL_ENGINE_WARN("A build is already in progress");
        return;
    }

    if (m_BuildThread && m_BuildThread->joinable()) {
        m_BuildThread->join();
    }

    m_CancelRequested.store(false);
    m_IsBuilding.store(true);
    m_LastBuildSucceeded = false;

    {
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress = BuildProgress{};
        m_Progress.CurrentStep = "Starting build...";
    }

    m_BuildThread = std::make_unique<std::thread>(&BuildPipeline::BuildThread, this, target, config);
}

void BuildPipeline::CancelBuild() {
    m_CancelRequested.store(true);
}

BuildProgress BuildPipeline::GetProgress() const {
    std::lock_guard<std::mutex> lock(m_ProgressMutex);
    return m_Progress;
}

void BuildPipeline::SetProgressCallback(BuildProgress::ProgressCallback callback) {
    m_ProgressCallback = std::move(callback);
}

void BuildPipeline::BuildThread(IBuildTarget* target, BuildConfig config) {
    auto updateProgress = [&](float pct, const std::string& step, const std::string& msg = "") {
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress.Percentage = pct;
        m_Progress.CurrentStep = step;
        if (!msg.empty()) m_Progress.LastMessage = msg;
        if (m_ProgressCallback) m_ProgressCallback(m_Progress);
    };

    FL_ENGINE_INFO("Build started: {} [{}]", target->GetName(),
                   config.Configuration == BuildConfiguration::Debug ? "Debug" :
                   config.Configuration == BuildConfiguration::Release ? "Release" : "Shipping");

    // Step 1: Configure
    updateProgress(0.05f, "Configuring...");
    if (!target->Configure(config, m_Progress)) {
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress.HasError = true;
        m_Progress.ErrorMessage = "Configuration failed: " + m_Progress.ErrorMessage;
        m_IsBuilding.store(false);
        return;
    }

    if (m_CancelRequested.load()) { m_IsBuilding.store(false); return; }

    // Step 2: Cook assets
    if (config.CookAssets) {
        updateProgress(0.15f, "Cooking assets...");
        if (!CookAssets(config, m_Progress)) {
            std::lock_guard<std::mutex> lock(m_ProgressMutex);
            m_Progress.HasError = true;
            m_IsBuilding.store(false);
            return;
        }
    }

    if (m_CancelRequested.load()) { m_IsBuilding.store(false); return; }

    // Step 3: Compile scripts (AOT)
    if (config.CompileScriptsAOT) {
        updateProgress(0.35f, "Compiling scripts (AOT)...");
        if (!CompileScripts(config, m_Progress)) {
            std::lock_guard<std::mutex> lock(m_ProgressMutex);
            m_Progress.HasError = true;
            m_IsBuilding.store(false);
            return;
        }
    }

    if (m_CancelRequested.load()) { m_IsBuilding.store(false); return; }

    // Step 4: Build
    updateProgress(0.50f, "Building...");
    if (!target->Build(config, m_Progress, [&](const BuildProgress& p) {
        // Remap build progress to 50-80%
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress.Percentage = 0.50f + p.Percentage * 0.30f;
        m_Progress.LastMessage = p.LastMessage;
        if (m_ProgressCallback) m_ProgressCallback(m_Progress);
    })) {
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress.HasError = true;
        m_IsBuilding.store(false);
        return;
    }

    if (m_CancelRequested.load()) { m_IsBuilding.store(false); return; }

    // Step 5: Package
    updateProgress(0.85f, "Packaging...");
    if (!target->Package(config, m_Progress, [&](const BuildProgress& p) {
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress.Percentage = 0.85f + p.Percentage * 0.10f;
        m_Progress.LastMessage = p.LastMessage;
        if (m_ProgressCallback) m_ProgressCallback(m_Progress);
    })) {
        std::lock_guard<std::mutex> lock(m_ProgressMutex);
        m_Progress.HasError = true;
        m_IsBuilding.store(false);
        return;
    }

    // Done
    updateProgress(1.0f, "Build complete!");
    m_LastBuildSucceeded = true;
    FL_ENGINE_INFO("Build completed successfully: {}", target->GetOutputPath(config).string());

    m_IsBuilding.store(false);
}

bool BuildPipeline::CookAssets(const BuildConfig& config, BuildProgress& progress) {
    FL_ENGINE_INFO("Cooking assets from: {}", config.ProjectRoot.string());
    // Delegate to AssetCooker (Phase 5) - for now, just pass through
    progress.LastMessage = "Asset cooking complete (passthrough)";
    return true;
}

bool BuildPipeline::CompileScripts(const BuildConfig& config, BuildProgress& progress) {
    FL_ENGINE_INFO("Compiling scripts AOT...");
    // Delegate to Lua2CPPEngine (Phase 4) - for now, just pass through
    progress.LastMessage = "Script compilation complete (passthrough)";
    return true;
}

} // namespace Freely
