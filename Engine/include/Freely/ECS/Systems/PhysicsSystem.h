#pragma once
// Freely Engine - PhysicsSystem
// ECS ISystem that:
//   1. On Start: creates Jolt bodies for every RigidBodyComponent + ColliderComponent
//   2. On FixedUpdate: steps the simulation
//   3. After step: reads body transforms back into TransformComponent
//   4. On Stop: destroys all bodies

#include "Freely/ECS/System.h"
#include <memory>

namespace Freely {

namespace Physics { class IPhysicsBackend3D; }

class PhysicsSystem : public ISystem {
public:
    PhysicsSystem();
    ~PhysicsSystem() override;

    const char* GetName()     const override { return "PhysicsSystem"; }
    int         GetPriority() const override { return 1000; } // before render

    void OnStart(Scene& scene)                          override;
    void OnFixedUpdate(Scene& scene, float fixedDt)     override;
    void OnStop(Scene& scene)                           override;

    // Step rate control (default 60 Hz fixed timestep)
    void SetFixedTimestep(float dt) { m_FixedDt = dt; }

private:
    void SyncTransformsToPhysics(Scene& scene);
    void SyncPhysicsToTransforms(Scene& scene);

    std::unique_ptr<Physics::IPhysicsBackend3D> m_Backend;
    float   m_FixedDt    = 1.0f / 60.0f;
    float   m_Accumulator = 0.0f;
    bool    m_Started    = false;
};

} // namespace Freely
