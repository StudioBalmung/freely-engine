#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>
#include <string>

namespace Freely::Physics {

using BodyHandle  = uint32_t;
using ShapeHandle = uint32_t;
constexpr BodyHandle  kInvalidBody  = 0;
constexpr ShapeHandle kInvalidShape = 0;

enum class BodyType : uint8_t {
    Static,
    Dynamic,
    Kinematic
};

enum class ShapeType : uint8_t {
    Sphere,
    Box,
    Capsule,
    Cylinder,
    Plane,
    ConvexHull,
    Mesh,
    Heightfield
};

struct MaterialDesc {
    float Friction    = 0.5f;
    float Restitution = 0.3f;
    float Density     = 1000.0f;
};

struct ShapeDesc {
    ShapeType Type;
    // Per-type parameters
    float       SphereRadius   = 0.5f;
    glm::vec3   BoxHalfExtents{0.5f, 0.5f, 0.5f};
    float       CapsuleRadius  = 0.5f;
    float       CapsuleHeight  = 1.0f;
    float       CylinderRadius = 0.5f;
    float       CylinderHeight = 1.0f;
    glm::vec3   PlaneNormal{0.0f, 1.0f, 0.0f};
    float       PlaneDistance  = 0.0f;
    const float* MeshVertices  = nullptr; // packed xyz
    uint32_t     MeshVertexCount= 0;
    const uint32_t* MeshIndices = nullptr;
    uint32_t     MeshIndexCount = 0;
};

struct BodyDesc {
    BodyType    Type       = BodyType::Dynamic;
    glm::vec3   Position   {0.0f};
    glm::quat   Rotation   {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3   LinearVelocity {0.0f};
    glm::vec3   AngularVelocity{0.0f};
    float       Mass        = 1.0f;
    float       LinearDamping  = 0.01f;
    float       AngularDamping = 0.01f;
    bool        UseGravity     = true;
    bool        CCD            = false;
    MaterialDesc Material;
    ShapeHandle Shape = kInvalidShape;
    std::string DebugName;
};

struct ContactInfo {
    BodyHandle  BodyA   = kInvalidBody;
    BodyHandle  BodyB   = kInvalidBody;
    glm::vec3   Position{0.0f};
    glm::vec3   Normal  {0.0f, 1.0f, 0.0f};
    float       Depth   = 0.0f;
};

struct RaycastHit {
    bool        HasHit  = false;
    BodyHandle  Body    = kInvalidBody;
    glm::vec3   Position{0.0f};
    glm::vec3   Normal  {0.0f, 1.0f, 0.0f};
    float       Distance= 0.0f;
};

} // namespace Freely::Physics
