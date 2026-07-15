#pragma once

// Freely Engine - Build Pipeline
// Orchestrates: Asset Cook → Compile Scripts → Link Runtime → Package

#include "BuildTarget.h"
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace Freely {

class BuildPipeline {
public:
    BuildPipeline() = default;
    ~BuildPipeline();

    /// Initialize pipeline and register all built-in targets.
    void Init();

    /// Start a build asynchronously. Returns immediately.
    void StartBuild(IBuildTarget* target, const BuildConfig& config);

    /// Check if a build is currently running.
    bool IsBuilding() const { return m_IsBuilding.load(); }

    /// Cancel the current build.
    void CancelBuild();

    /// Get the current build progress (thread-safe).
    BuildProgress GetProgress() const;

    /// Set a callback for progress updates.
    void SetProgressCallback(BuildProgress::ProgressCallback callback);

    /// Get the last build result.
    bool LastBuildSucceeded() const { return m_LastBuildSucceeded; }

private:
    void BuildThread(IBuildTarget* target, BuildConfig config);

    // Asset cooking step
    bool CookAssets(const BuildConfig& config, BuildProgress& progress);

    // Script compilation step
    bool CompileScripts(const BuildConfig& config, BuildProgress& progress);

    std::atomic<bool> m_IsBuilding{false};
    std::atomic<bool> m_CancelRequested{false};
    bool m_LastBuildSucceeded = false;

    mutable std::mutex m_ProgressMutex;
    BuildProgress m_Progress;
    BuildProgress::ProgressCallback m_ProgressCallback;

    std::unique_ptr<std::thread> m_BuildThread;
};

} // namespace Freely
