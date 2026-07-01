#include "Freely/Physics/IPhysicsBackend.h"
#include "Freely/Core/Logger.h"

namespace Freely::Physics {

// Forward declarations of backend creators.
std::unique_ptr<IPhysicsBackend3D> CreateAsterCoreBackend3D();
#ifdef FREELY_PHYSICS_JOLT
std::unique_ptr<IPhysicsBackend3D> CreateJoltBackend3D();
#endif
#ifdef FREELY_PHYSICS_PHYSX
std::unique_ptr<IPhysicsBackend3D> CreatePhysXBackend3D();
#endif
#ifdef FREELY_PHYSICS_BOX2D
std::unique_ptr<IPhysicsBackend2D> CreateBox2DBackend2D();
#endif

std::unique_ptr<IPhysicsBackend3D> PhysicsBackendFactory::Create3D(PhysicsBackend3D type) {
    switch (type) {
        case PhysicsBackend3D::AsterCore: return CreateAsterCoreBackend3D();
        case PhysicsBackend3D::Jolt:
#ifdef FREELY_PHYSICS_JOLT
            return CreateJoltBackend3D();
#else
            FL_ENGINE_WARN("Jolt backend not compiled in. Falling back to AsterCore.");
            return CreateAsterCoreBackend3D();
#endif
        case PhysicsBackend3D::PhysX:
#ifdef FREELY_PHYSICS_PHYSX
            return CreatePhysXBackend3D();
#else
            FL_ENGINE_WARN("PhysX backend not compiled in. Falling back to AsterCore.");
            return CreateAsterCoreBackend3D();
#endif
        case PhysicsBackend3D::None:
        default:
            return nullptr;
    }
}

std::unique_ptr<IPhysicsBackend2D> PhysicsBackendFactory::Create2D(PhysicsBackend2D type) {
    if (type == PhysicsBackend2D::Box2D) {
#ifdef FREELY_PHYSICS_BOX2D
        return CreateBox2DBackend2D();
#else
        FL_ENGINE_WARN("Box2D backend not compiled in.");
#endif
    }
    return nullptr;
}

std::vector<PhysicsBackend3D> PhysicsBackendFactory::AvailableBackends3D() {
    std::vector<PhysicsBackend3D> list = { PhysicsBackend3D::AsterCore };
#ifdef FREELY_PHYSICS_JOLT
    list.push_back(PhysicsBackend3D::Jolt);
#endif
#ifdef FREELY_PHYSICS_PHYSX
    list.push_back(PhysicsBackend3D::PhysX);
#endif
    return list;
}

std::vector<PhysicsBackend2D> PhysicsBackendFactory::AvailableBackends2D() {
    std::vector<PhysicsBackend2D> list;
#ifdef FREELY_PHYSICS_BOX2D
    list.push_back(PhysicsBackend2D::Box2D);
#endif
    return list;
}

} // namespace Freely::Physics
