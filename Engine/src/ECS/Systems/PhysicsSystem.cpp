#include "Freely/ECS/Systems/PhysicsSystem.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Physics/PhysicsTypes.h"
#include "Freely/Core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Freely {

PhysicsSystem::PhysicsSystem() = default;
PhysicsSystem::~PhysicsSystem() = default;

// --- Create physics bodies from ECS components (OnCreate) --------------------
void PhysicsSystem::OnCreate(Scene& scene) {
    if (m_Started) return;

    // Initialize the physics backend (AsterCore = Jolt fork)
    Physics::PhysicsConfig cfg;
    cfg.Backend3D = Physics::PhysicsBackend3D::AsterCore;
    cfg.Backend2D = Physics::PhysicsBackend2D::None;
    cfg.Gravity3DX = 0.0f;
    cfg.Gravity3DY = -9.81f;
    cfg.Gravity3DZ = 0.0f;
    cfg.MaxSubSteps = 4;
    cfg.FixedTimeStep = 1.0f / 60.0f;
    cfg.WorkerThreads = 0;

    m_Backend = Physics::PhysicsBackendFactory::Create3D(Physics::PhysicsBackend3D::AsterCore);
    if (!m_Backend || !m_Backend->Initialize(cfg)) {
        FL_ENGINE_ERROR("PhysicsSystem: failed to initialize backend.");
        m_Backend = nullptr;
        return;
    }

    auto& reg = scene.GetRegistry().GetEnttRegistry();

    // Create shapes + bodies for entities with RigidBody + Collider
    auto view = reg.view<TransformComponent, RigidBodyComponent, ColliderComponent>();
    for (auto e : view) {
        auto [tf, rb, col] = view.get<TransformComponent, RigidBodyComponent, ColliderComponent>(e);

        // -- Build shape ---------------------------------------------------
        Physics::ShapeDesc sd{};
        switch (col.Shape) {
        case ColliderShape::Box:
            sd.Type = Physics::ShapeType::Box;
            sd.BoxHalfExtents = col.BoxHalfExtents * tf.Scale;
            break;
        case ColliderShape::Sphere:
            sd.Type = Physics::ShapeType::Sphere;
            sd.SphereRadius = col.SphereRadius * std::max({tf.Scale.x, tf.Scale.y, tf.Scale.z});
            break;
        case ColliderShape::Capsule:
            sd.Type = Physics::ShapeType::Capsule;
            sd.CapsuleRadius = col.CapsuleRadius;
            sd.CapsuleHeight = col.CapsuleHeight;
            break;
        default:
            sd.Type = Physics::ShapeType::Box;
            sd.BoxHalfExtents = col.BoxHalfExtents;
            break;
        }
        Physics::ShapeHandle shapeH = m_Backend->CreateShape(sd);
        col.RuntimeShapeHandle = shapeH;

        // -- Build body ----------------------------------------------------
        Physics::BodyDesc bd{};
        bd.Position = tf.Position + col.Center;
        bd.Rotation = tf.Rotation;
        bd.Mass     = rb.Mass;
        bd.LinearDamping  = rb.LinearDamping;
        bd.AngularDamping = rb.AngularDamping;
        bd.UseGravity = rb.UseGravity;
        bd.CCD        = rb.ContinuousCD;
        bd.Shape      = shapeH;
        bd.Material.Friction    = rb.Friction;
        bd.Material.Restitution = rb.Restitution;

        switch (rb.Type) {
        case BodyType::Static:    bd.Type = Physics::BodyType::Static;    break;
        case BodyType::Dynamic:   bd.Type = Physics::BodyType::Dynamic;   break;
        case BodyType::Kinematic: bd.Type = Physics::BodyType::Kinematic; break;
        }

        Physics::BodyHandle bodyH = m_Backend->CreateBody(bd);
        rb.RuntimeBodyHandle = bodyH;
    }

    m_Started = true;
    FL_ENGINE_INFO("PhysicsSystem started ({} bodies created).", (int)view.size_hint());
}

// --- FixedUpdate: advance simulation -----------------------------------------
void PhysicsSystem::OnFixedUpdate(Scene& scene, float fixedDt) {
    if (!m_Backend || !m_Started) return;

    // Write ECS kinematic transforms -> physics (for kinematic bodies only)
    SyncTransformsToPhysics(scene);

    m_Backend->Step(fixedDt);

    // Read physics results -> ECS transforms
    SyncPhysicsToTransforms(scene);
}

// --- Destroy physics bodies (OnDestroy) -------------------------------------------------
void PhysicsSystem::OnDestroy(Scene& scene) {
    if (!m_Backend) return;
    m_Backend->Clear();
    m_Backend->Shutdown();
    m_Backend.reset();
    m_Started = false;
    FL_ENGINE_INFO("PhysicsSystem stopped.");
}

// --- Sync kinematic ECS -> physics --------------------------------------------
void PhysicsSystem::SyncTransformsToPhysics(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, RigidBodyComponent>();
    for (auto e : view) {
        auto [tf, rb] = view.get<TransformComponent, RigidBodyComponent>(e);
        if (rb.Type != BodyType::Kinematic) continue;
        if (!rb.RuntimeBodyHandle) continue;
        m_Backend->SetBodyTransform((Physics::BodyHandle)rb.RuntimeBodyHandle, tf.Position, tf.Rotation);
    }
}

// --- Sync physics -> ECS transforms ------------------------------------------
void PhysicsSystem::SyncPhysicsToTransforms(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, RigidBodyComponent, ColliderComponent>();
    for (auto e : view) {
        auto [tf, rb, col] = view.get<TransformComponent, RigidBodyComponent, ColliderComponent>(e);
        if (rb.Type == BodyType::Static) continue;
        if (!rb.RuntimeBodyHandle) continue;

        glm::vec3 pos; glm::quat rot;
        m_Backend->GetBodyTransform((Physics::BodyHandle)rb.RuntimeBodyHandle, pos, rot);
        tf.Position = pos - col.Center;
        tf.Rotation = rot;
        tf.Dirty    = true;
    }
}

} // namespace Freely


// --- Create physics bodies from ECS components (OnCreate) --------------------
void PhysicsSystem::OnCreate(Scene& scene) {
    if (m_Started) return;

    // Initialize the physics backend (AsterCore = Jolt fork)
    PhysicsConfig cfg;
    cfg.Backend3D = PhysicsBackend3D::AsterCore;
    cfg.Backend2D = PhysicsBackend2D::None;
    cfg.Gravity3DX = 0.0f;
    cfg.Gravity3DY = -9.81f;
    cfg.Gravity3DZ = 0.0f;
    cfg.MaxSubSteps = 4;
    cfg.FixedTimeStep = 1.0f / 60.0f;
    cfg.WorkerThreads = 0;

    m_Backend = PhysicsBackendFactory::Create3D(PhysicsBackend3D::AsterCore);
    if (!m_Backend || !m_Backend->Initialize(cfg)) {
        FL_ENGINE_ERROR("PhysicsSystem: failed to initialize backend.");
        m_Backend = nullptr;
        return;
    }

    auto& reg = scene.GetRegistry().GetEnttRegistry();

    // Create shapes + bodies for entities with RigidBody + Collider
    auto view = reg.view<TransformComponent, RigidBodyComponent, ColliderComponent>();
    for (auto e : view) {
        auto [tf, rb, col] = view.get<TransformComponent, RigidBodyComponent, ColliderComponent>(e);

        // -- Build shape ---------------------------------------------------
        ShapeDesc sd{};
        switch (col.Shape) {
        case ColliderShape::Box:
            sd.Type = ShapeType::Box;
            sd.BoxHalfExtents = col.BoxHalfExtents * tf.Scale;
            break;
        case ColliderShape::Sphere:
            sd.Type = ShapeType::Sphere;
            sd.SphereRadius = col.SphereRadius * std::max({tf.Scale.x, tf.Scale.y, tf.Scale.z});
            break;
        case ColliderShape::Capsule:
            sd.Type = ShapeType::Capsule;
            sd.CapsuleRadius = col.CapsuleRadius;
            sd.CapsuleHeight = col.CapsuleHeight;
            break;
        default:
            sd.Type = ShapeType::Box;
            sd.BoxHalfExtents = col.BoxHalfExtents;
            break;
        }
        ShapeHandle shapeH = m_Backend->CreateShape(sd);
        col.RuntimeShapeHandle = shapeH;

        // -- Build body ----------------------------------------------------
        BodyDesc bd{};
        bd.Position = tf.Position + col.Center;
        bd.Rotation = tf.Rotation;
        bd.Mass     = rb.Mass;
        bd.LinearDamping  = rb.LinearDamping;
        bd.AngularDamping = rb.AngularDamping;
        bd.UseGravity = rb.UseGravity;
        bd.CCD        = rb.ContinuousCD;
        bd.Shape      = shapeH;
        bd.Material.Friction    = rb.Friction;
        bd.Material.Restitution = rb.Restitution;

        switch (rb.Type) {
        case Freely::BodyType::Static:    bd.Type = Physics::BodyType::Static;    break;
        case Freely::BodyType::Dynamic:   bd.Type = Physics::BodyType::Dynamic;   break;
        case Freely::BodyType::Kinematic: bd.Type = Physics::BodyType::Kinematic; break;
        }

        BodyHandle bodyH = m_Backend->CreateBody(bd);
        rb.RuntimeBodyHandle = bodyH;
    }

    m_Started = true;
    FL_ENGINE_INFO("PhysicsSystem started ({} bodies created).", (int)view.size_hint());
}

// --- FixedUpdate: advance simulation -----------------------------------------
void PhysicsSystem::OnFixedUpdate(Scene& scene, float fixedDt) {
    if (!m_Backend || !m_Started) return;

    // Write ECS kinematic transforms → physics (for kinematic bodies only)
    SyncTransformsToPhysics(scene);

    m_Backend->Step(fixedDt);

    // Read physics results → ECS transforms
    SyncPhysicsToTransforms(scene);
}

// --- Destroy physics bodies (OnDestroy) -------------------------------------------------
void PhysicsSystem::OnDestroy(Scene& scene) {
    if (!m_Backend) return;
    m_Backend->Clear();
    m_Backend->Shutdown();
    m_Backend.reset();
    m_Started = false;
    FL_ENGINE_INFO("PhysicsSystem stopped.");
}

// --- Sync kinematic ECS → physics --------------------------------------------
void PhysicsSystem::SyncTransformsToPhysics(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, RigidBodyComponent>();
    for (auto e : view) {
        auto [tf, rb] = view.get<TransformComponent, RigidBodyComponent>(e);
        if (rb.Type != Freely::BodyType::Kinematic) continue;
        if (!rb.RuntimeBodyHandle) continue;
        m_Backend->SetBodyTransform((BodyHandle)rb.RuntimeBodyHandle, tf.Position, tf.Rotation);
    }
}

// --- Sync physics → ECS transforms ------------------------------------------
void PhysicsSystem::SyncPhysicsToTransforms(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, RigidBodyComponent, ColliderComponent>();
    for (auto e : view) {
        auto [tf, rb, col] = view.get<TransformComponent, RigidBodyComponent, ColliderComponent>(e);
        if (rb.Type == Freely::BodyType::Static) continue;
        if (!rb.RuntimeBodyHandle) continue;

        glm::vec3 pos; glm::quat rot;
        m_Backend->GetBodyTransform((BodyHandle)rb.RuntimeBodyHandle, pos, rot);
        tf.Position = pos - col.Center;
        tf.Rotation = rot;
        tf.Dirty    = true;
    }
}

} // namespace Freely
