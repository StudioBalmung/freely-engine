#pragma once

#include "PhysicsTypes.h"
#include "Freely/Config/EngineConfig.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace Freely::Physics {

/// Abstract 3D physics backend. Concrete implementations:
///   - AsterCorePhysicsBackend  (default, in-house)
///   - JoltPhysicsBackend       (optional)
///   - PhysXPhysicsBackend      (optional)
class IPhysicsBackend3D {
public:
    virtual ~IPhysicsBackend3D() = default;

    virtual bool Initialize(const PhysicsConfig& config) = 0;
    virtual void Shutdown() = 0;

    virtual const char* GetName() const = 0;
    virtual PhysicsBackend3D GetType() const = 0;

    virtual void SetGravity(const glm::vec3& gravity) = 0;
    virtual glm::vec3 GetGravity() const = 0;

    virtual void Step(float deltaTime) = 0;

    // Shapes
    virtual ShapeHandle CreateShape(const ShapeDesc& desc) = 0;
    virtual void        DestroyShape(ShapeHandle handle) = 0;

    // Bodies
    virtual BodyHandle  CreateBody(const BodyDesc& desc) = 0;
    virtual void        DestroyBody(BodyHandle handle) = 0;
    virtual void        Clear() = 0;

    virtual void        SetBodyTransform(BodyHandle handle, const glm::vec3& position, const glm::quat& rotation) = 0;
    virtual void        GetBodyTransform(BodyHandle handle, glm::vec3& position, glm::quat& rotation) const = 0;
    virtual void        SetBodyLinearVelocity(BodyHandle handle, const glm::vec3& v) = 0;
    virtual glm::vec3   GetBodyLinearVelocity(BodyHandle handle) const = 0;
    virtual void        SetBodyAngularVelocity(BodyHandle handle, const glm::vec3& v) = 0;
    virtual glm::vec3   GetBodyAngularVelocity(BodyHandle handle) const = 0;

    virtual void        ApplyForce(BodyHandle handle, const glm::vec3& force) = 0;
    virtual void        ApplyImpulse(BodyHandle handle, const glm::vec3& impulse) = 0;
    virtual void        ApplyTorque(BodyHandle handle, const glm::vec3& torque) = 0;

    // Queries
    virtual RaycastHit  Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) const = 0;
    virtual std::vector<ContactInfo> GetContacts() const = 0;
};

/// Abstract 2D physics backend. Concrete implementations:
///   - Box2DPhysicsBackend (default)
class IPhysicsBackend2D {
public:
    virtual ~IPhysicsBackend2D() = default;
    virtual bool Initialize(const PhysicsConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual const char* GetName() const = 0;
    virtual PhysicsBackend2D GetType() const = 0;
    virtual void SetGravity(const glm::vec2& gravity) = 0;
    virtual glm::vec2 GetGravity() const = 0;
    virtual void Step(float deltaTime) = 0;
    virtual void Clear() = 0;
};

/// Factory for selecting physics backends at runtime.
class PhysicsBackendFactory {
public:
    static std::unique_ptr<IPhysicsBackend3D> Create3D(PhysicsBackend3D type);
    static std::unique_ptr<IPhysicsBackend2D> Create2D(PhysicsBackend2D type);

    /// Returns a list of physics backends compiled into this build.
    static std::vector<PhysicsBackend3D> AvailableBackends3D();
    static std::vector<PhysicsBackend2D> AvailableBackends2D();
};

} // namespace Freely::Physics
