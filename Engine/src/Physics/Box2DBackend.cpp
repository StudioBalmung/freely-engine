// Box2D backend stub. Compiled only when FREELY_PHYSICS_BOX2D is defined.
// Provides minimal scaffolding for 2D physics integration.
#ifdef FREELY_PHYSICS_BOX2D

#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Core/Logger.h"

#include <box2d/box2d.h>

namespace Freely::Physics {

class Box2DBackend2D final : public IPhysicsBackend2D {
public:
    bool Initialize(const PhysicsConfig& cfg) override {
        b2WorldDef def = b2DefaultWorldDef();
        def.gravity = { cfg.Gravity2DX, cfg.Gravity2DY };
        m_World = b2CreateWorld(&def);
        m_SubSteps = (cfg.MaxSubSteps > 0) ? cfg.MaxSubSteps : 4;
        FL_ENGINE_INFO("Physics2D backend: Box2D initialized.");
        return true;
    }
    void Shutdown() override {
        if (B2_IS_NON_NULL(m_World)) { b2DestroyWorld(m_World); m_World = b2_nullWorldId; }
    }
    const char* GetName() const override { return "Box2D"; }
    PhysicsBackend2D GetType() const override { return PhysicsBackend2D::Box2D; }
    void SetGravity(const glm::vec2& g) override { if (B2_IS_NON_NULL(m_World)) b2World_SetGravity(m_World, {g.x, g.y}); }
    glm::vec2 GetGravity() const override {
        if (B2_IS_NULL(m_World)) return {};
        b2Vec2 g = b2World_GetGravity(m_World);
        return { g.x, g.y };
    }
    void Step(float dt) override { if (B2_IS_NON_NULL(m_World)) b2World_Step(m_World, dt, m_SubSteps); }
    void Clear() override {}
private:
    b2WorldId m_World = b2_nullWorldId;
    int m_SubSteps = 4;
};

std::unique_ptr<IPhysicsBackend2D> CreateBox2DBackend2D() {
    return std::make_unique<Box2DBackend2D>();
}

} // namespace Freely::Physics

#endif // FREELY_PHYSICS_BOX2D
