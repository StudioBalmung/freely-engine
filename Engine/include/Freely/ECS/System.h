#pragma once

// Freely Engine - System Interface & Scheduler
// Systems process components in the ECS each frame.

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <cstdint>

namespace Freely {

class Scene;

// ─── ISystem ────────────────────────────────────────────────────────────────
/// Base interface for all engine/game systems.
class ISystem {
public:
    virtual ~ISystem() = default;

    virtual const char* GetName() const = 0;

    /// Priority determines execution order (lower = earlier). Default 1000.
    virtual int GetPriority() const { return 1000; }

    /// Called once when the system is added to a scene.
    virtual void OnCreate(Scene& scene) {}

    /// Called every frame.
    virtual void OnUpdate(Scene& scene, float deltaTime) {}

    /// Called at fixed intervals for physics/deterministic logic.
    virtual void OnFixedUpdate(Scene& scene, float fixedDeltaTime) {}

    /// Called after all systems have been updated (for rendering).
    virtual void OnLateUpdate(Scene& scene, float deltaTime) {}

    /// Called when the system is removed from the scene.
    virtual void OnDestroy(Scene& scene) {}

    /// Enable/disable the system at runtime.
    bool Enabled = true;
};

// ─── SystemScheduler ────────────────────────────────────────────────────────
/// Manages an ordered list of systems and dispatches lifecycle calls.
class SystemScheduler {
public:
    SystemScheduler() = default;
    ~SystemScheduler() { m_Systems.clear(); }

    /// Add a system. Systems are sorted by priority after insertion.
    template<typename T, typename... Args>
    T* AddSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        m_Systems.push_back(std::move(system));
        SortSystems();
        return ptr;
    }

    /// Add a pre-constructed system.
    ISystem* AddSystem(std::unique_ptr<ISystem> system) {
        ISystem* ptr = system.get();
        m_Systems.push_back(std::move(system));
        SortSystems();
        return ptr;
    }

    /// Remove a system by pointer.
    void RemoveSystem(ISystem* system) {
        m_Systems.erase(
            std::remove_if(m_Systems.begin(), m_Systems.end(),
                [system](const auto& s) { return s.get() == system; }),
            m_Systems.end());
    }

    /// Find a system by type.
    template<typename T>
    T* GetSystem() {
        for (auto& s : m_Systems) {
            auto* typed = dynamic_cast<T*>(s.get());
            if (typed) return typed;
        }
        return nullptr;
    }

    /// Called once after all systems are added.
    void Initialize(Scene& scene) {
        for (auto& s : m_Systems) {
            s->OnCreate(scene);
        }
    }

    /// Called every frame.
    void Update(Scene& scene, float deltaTime) {
        for (auto& s : m_Systems) {
            if (s->Enabled) s->OnUpdate(scene, deltaTime);
        }
    }

    /// Called at fixed intervals.
    void FixedUpdate(Scene& scene, float fixedDeltaTime) {
        for (auto& s : m_Systems) {
            if (s->Enabled) s->OnFixedUpdate(scene, fixedDeltaTime);
        }
    }

    /// Called after Update.
    void LateUpdate(Scene& scene, float deltaTime) {
        for (auto& s : m_Systems) {
            if (s->Enabled) s->OnLateUpdate(scene, deltaTime);
        }
    }

    /// Called on shutdown.
    void Shutdown(Scene& scene) {
        // Reverse order for shutdown
        for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it) {
            (*it)->OnDestroy(scene);
        }
        m_Systems.clear();
    }

    const std::vector<std::unique_ptr<ISystem>>& GetSystems() const { return m_Systems; }

private:
    void SortSystems() {
        std::stable_sort(m_Systems.begin(), m_Systems.end(),
            [](const auto& a, const auto& b) { return a->GetPriority() < b->GetPriority(); });
    }

    std::vector<std::unique_ptr<ISystem>> m_Systems;
};

} // namespace Freely
