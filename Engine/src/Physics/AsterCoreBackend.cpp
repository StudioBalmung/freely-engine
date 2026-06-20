#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Core/Logger.h"

#include <AsterCore/PhysicsWorld.h>
#include <AsterCore/RigidBody.h>
#include <AsterCore/Collider.h>

#include <unordered_map>

namespace Freely::Physics {

class AsterCoreBackend3D final : public IPhysicsBackend3D {
public:
    bool Initialize(const PhysicsConfig& config) override {
        m_World = std::make_unique<AsterCore::PhysicsWorld>(
            glm::vec3(config.Gravity3DX, config.Gravity3DY, config.Gravity3DZ));
        m_World->SetSubSteps(config.MaxSubSteps);
        FL_ENGINE_INFO("Physics3D backend: AsterCore (in-house) initialized.");
        return true;
    }

    void Shutdown() override {
        if (m_World) m_World->Clear();
        m_Bodies.clear();
        m_Shapes.clear();
        m_World.reset();
    }

    const char*       GetName() const override { return "AsterCore"; }
    PhysicsBackend3D  GetType() const override { return PhysicsBackend3D::AsterCore; }

    void SetGravity(const glm::vec3& g) override { if (m_World) m_World->SetGravity(g); }
    glm::vec3 GetGravity() const override { return m_World ? m_World->GetGravity() : glm::vec3(0.0f); }

    void Step(float dt) override { if (m_World) m_World->Step(dt); }

    ShapeHandle CreateShape(const ShapeDesc& desc) override {
        std::shared_ptr<AsterCore::Collider> col;
        switch (desc.Type) {
            case ShapeType::Sphere:
                col = std::make_shared<AsterCore::SphereCollider>(desc.SphereRadius); break;
            case ShapeType::Box:
                col = std::make_shared<AsterCore::BoxCollider>(desc.BoxHalfExtents); break;
            case ShapeType::Plane:
                col = std::make_shared<AsterCore::PlaneCollider>(desc.PlaneNormal, desc.PlaneDistance); break;
            default:
                FL_ENGINE_WARN("AsterCore: unsupported shape type, defaulting to Box.");
                col = std::make_shared<AsterCore::BoxCollider>(); break;
        }
        ShapeHandle id = m_NextShape++;
        m_Shapes[id] = col;
        return id;
    }

    void DestroyShape(ShapeHandle h) override { m_Shapes.erase(h); }

    BodyHandle CreateBody(const BodyDesc& desc) override {
        AsterCore::BodyType type = AsterCore::BodyType::Dynamic;
        if (desc.Type == BodyType::Static)    type = AsterCore::BodyType::Static;
        if (desc.Type == BodyType::Kinematic) type = AsterCore::BodyType::Kinematic;

        auto body = std::make_shared<AsterCore::RigidBody>(type);
        body->SetPosition(desc.Position);
        body->SetRotation(desc.Rotation);
        body->SetLinearVelocity(desc.LinearVelocity);
        body->SetAngularVelocity(desc.AngularVelocity);
        body->SetMass(desc.Mass);
        body->SetRestitution(desc.Material.Restitution);
        body->SetFriction(desc.Material.Friction);
        body->SetLinearDamping(desc.LinearDamping);
        body->SetAngularDamping(desc.AngularDamping);

        auto sit = m_Shapes.find(desc.Shape);
        if (sit != m_Shapes.end()) body->SetCollider(sit->second);

        m_World->AddBody(body);
        BodyHandle id = m_NextBody++;
        m_Bodies[id] = body;
        return id;
    }

    void DestroyBody(BodyHandle h) override {
        auto it = m_Bodies.find(h);
        if (it == m_Bodies.end()) return;
        m_World->RemoveBody(it->second);
        m_Bodies.erase(it);
    }

    void Clear() override {
        if (m_World) m_World->Clear();
        m_Bodies.clear();
    }

    void SetBodyTransform(BodyHandle h, const glm::vec3& p, const glm::quat& r) override {
        if (auto b = Get(h)) { b->SetPosition(p); b->SetRotation(r); }
    }

    void GetBodyTransform(BodyHandle h, glm::vec3& p, glm::quat& r) const override {
        if (auto b = Get(h)) { p = b->GetPosition(); r = b->GetRotation(); }
    }

    void      SetBodyLinearVelocity(BodyHandle h, const glm::vec3& v) override { if (auto b=Get(h)) b->SetLinearVelocity(v); }
    glm::vec3 GetBodyLinearVelocity(BodyHandle h) const override { if (auto b=Get(h)) return b->GetLinearVelocity(); return {}; }
    void      SetBodyAngularVelocity(BodyHandle h, const glm::vec3& v) override { if (auto b=Get(h)) b->SetAngularVelocity(v); }
    glm::vec3 GetBodyAngularVelocity(BodyHandle h) const override { if (auto b=Get(h)) return b->GetAngularVelocity(); return {}; }

    void ApplyForce(BodyHandle h, const glm::vec3& f)   override { if (auto b=Get(h)) b->ApplyForce(f); }
    void ApplyImpulse(BodyHandle h, const glm::vec3& i) override { if (auto b=Get(h)) b->ApplyImpulse(i); }
    void ApplyTorque(BodyHandle h, const glm::vec3& t)  override { if (auto b=Get(h)) b->ApplyTorque(t); }

    RaycastHit Raycast(const glm::vec3&, const glm::vec3&, float) const override {
        // TODO: implement broadphase ray query in AsterCore.
        return {};
    }
    std::vector<ContactInfo> GetContacts() const override { return {}; }

private:
    std::shared_ptr<AsterCore::RigidBody> Get(BodyHandle h) const {
        auto it = m_Bodies.find(h);
        return (it == m_Bodies.end()) ? nullptr : it->second;
    }

    std::unique_ptr<AsterCore::PhysicsWorld> m_World;
    std::unordered_map<BodyHandle,  std::shared_ptr<AsterCore::RigidBody>> m_Bodies;
    std::unordered_map<ShapeHandle, std::shared_ptr<AsterCore::Collider>>  m_Shapes;
    BodyHandle  m_NextBody  = 1;
    ShapeHandle m_NextShape = 1;
};

std::unique_ptr<IPhysicsBackend3D> CreateAsterCoreBackend3D() {
    return std::make_unique<AsterCoreBackend3D>();
}

} // namespace Freely::Physics
