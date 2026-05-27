// Jolt Physics backend stub. Compiled only when FREELY_PHYSICS_JOLT is defined.
// Provides a placeholder implementation. Full backend implementation pending.
#ifdef FREELY_PHYSICS_JOLT

#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Core/Logger.h"

namespace Freely::Physics {

class JoltBackend3D final : public IPhysicsBackend3D {
public:
    bool Initialize(const PhysicsConfig&) override {
        FL_ENGINE_WARN("Jolt backend registered but not yet fully implemented.");
        return false;
    }
    void Shutdown() override {}
    const char*       GetName() const override { return "Jolt"; }
    PhysicsBackend3D  GetType() const override { return PhysicsBackend3D::Jolt; }
    void              SetGravity(const glm::vec3&) override {}
    glm::vec3         GetGravity() const override { return {}; }
    void              Step(float) override {}
    ShapeHandle       CreateShape(const ShapeDesc&) override { return kInvalidShape; }
    void              DestroyShape(ShapeHandle) override {}
    BodyHandle        CreateBody(const BodyDesc&) override { return kInvalidBody; }
    void              DestroyBody(BodyHandle) override {}
    void              Clear() override {}
    void              SetBodyTransform(BodyHandle, const glm::vec3&, const glm::quat&) override {}
    void              GetBodyTransform(BodyHandle, glm::vec3&, glm::quat&) const override {}
    void              SetBodyLinearVelocity(BodyHandle, const glm::vec3&) override {}
    glm::vec3         GetBodyLinearVelocity(BodyHandle) const override { return {}; }
    void              SetBodyAngularVelocity(BodyHandle, const glm::vec3&) override {}
    glm::vec3         GetBodyAngularVelocity(BodyHandle) const override { return {}; }
    void              ApplyForce(BodyHandle, const glm::vec3&) override {}
    void              ApplyImpulse(BodyHandle, const glm::vec3&) override {}
    void              ApplyTorque(BodyHandle, const glm::vec3&) override {}
    RaycastHit        Raycast(const glm::vec3&, const glm::vec3&, float) const override { return {}; }
    std::vector<ContactInfo> GetContacts() const override { return {}; }
};

std::unique_ptr<IPhysicsBackend3D> CreateJoltBackend3D() {
    return std::make_unique<JoltBackend3D>();
}

} // namespace Freely::Physics

#endif // FREELY_PHYSICS_JOLT
