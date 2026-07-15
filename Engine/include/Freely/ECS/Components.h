#pragma once

// Freely Engine - ECS Components
// All components are plain-old-data structs stored in an EnTT registry.

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <entt/entt.hpp>

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>

namespace Freely {

// --- UUID for stable entity identification across saves ---------------------
struct UUID {
    uint64_t Value = 0;

    UUID() : Value(0) {}
    explicit UUID(uint64_t v) : Value(v) {}

    bool operator==(const UUID& o) const { return Value == o.Value; }
    bool operator!=(const UUID& o) const { return Value != o.Value; }
    bool operator<(const UUID& o) const  { return Value < o.Value; }
    explicit operator bool() const       { return Value != 0; }
    operator uint64_t() const            { return Value; }

    static UUID Generate();
    std::string ToString() const { return std::to_string(Value); }
};

} // namespace Freely

// Hash for UUID so it can be used in unordered containers
namespace std {
template<> struct hash<Freely::UUID> {
    size_t operator()(const Freely::UUID& id) const noexcept {
        return std::hash<uint64_t>{}(id.Value);
    }
};
} // namespace std

namespace Freely {

// --- IDComponent ------------------------------------------------------------
struct IDComponent {
    UUID ID;
    IDComponent() : ID(UUID::Generate()) {}
    explicit IDComponent(UUID id) : ID(id) {}
};

// --- TagComponent -----------------------------------------------------------
struct TagComponent {
    std::string Name = "Entity";
    std::string Tag  = "Untagged";
    int Layer        = 0;
    bool Active      = true;
};

// --- TransformComponent ----------------------------------------------------
struct TransformComponent {
    glm::vec3 Position { 0.0f, 0.0f, 0.0f };
    glm::quat Rotation { 1.0f, 0.0f, 0.0f, 0.0f }; // identity
    glm::vec3 Scale    { 1.0f, 1.0f, 1.0f };

    // Cached matrices (updated by SceneGraph)
    glm::mat4 LocalMatrix { 1.0f };
    glm::mat4 WorldMatrix { 1.0f };
    bool      Dirty = true;

    glm::vec3 GetEulerAngles() const { return glm::degrees(glm::eulerAngles(Rotation)); }
    void SetEulerAngles(const glm::vec3& degrees) {
        Rotation = glm::quat(glm::radians(degrees));
        Dirty = true;
    }

    glm::vec3 GetForward() const { return glm::normalize(Rotation * glm::vec3(0, 0, -1)); }
    glm::vec3 GetRight()   const { return glm::normalize(Rotation * glm::vec3(1, 0, 0)); }
    glm::vec3 GetUp()      const { return glm::normalize(Rotation * glm::vec3(0, 1, 0)); }
};

// --- RelationshipComponent (scene hierarchy) -------------------------------
struct RelationshipComponent {
    entt::entity Parent      = entt::null;
    entt::entity FirstChild  = entt::null;
    entt::entity NextSibling = entt::null;
    entt::entity PrevSibling = entt::null;
    uint32_t     ChildCount  = 0;
};

// --- MeshRendererComponent -------------------------------------------------
struct MeshRendererComponent {
    std::string MeshAsset;     // Asset path/UUID for the mesh
    std::string MaterialAsset; // Asset path/UUID for the material

    // Runtime references (set by the asset system)
    uint64_t MeshHandle     = 0;
    uint64_t MaterialHandle = 0;

    bool CastShadows    = true;
    bool ReceiveShadows = true;
    bool Visible        = true;
};

// --- MeshFilterComponent ---------------------------------------------------
enum class PrimitiveMeshType : uint8_t {
    None,
    Cube,
    Sphere,
    Plane,
    Cylinder,
    Capsule,
    Cone,
    Quad,
    Custom
};

struct MeshFilterComponent {
    PrimitiveMeshType PrimitiveType = PrimitiveMeshType::None;
    std::string       CustomMeshPath;
};

// --- CameraComponent ------------------------------------------------------
enum class ProjectionType : uint8_t {
    Perspective,
    Orthographic
};

struct CameraComponent {
    ProjectionType Projection = ProjectionType::Perspective;
    float FOV         = 60.0f;
    float NearPlane   = 0.1f;
    float FarPlane    = 1000.0f;
    float OrthoSize   = 10.0f;
    bool  Primary     = false; // Main camera flag
    bool  FixedAspect = false;
    float AspectRatio = 16.0f / 9.0f;

    glm::mat4 GetProjectionMatrix(float viewportAspect) const {
        float aspect = FixedAspect ? AspectRatio : viewportAspect;
        if (Projection == ProjectionType::Perspective)
            return glm::perspective(glm::radians(FOV), aspect, NearPlane, FarPlane);
        float halfW = OrthoSize * aspect * 0.5f;
        float halfH = OrthoSize * 0.5f;
        return glm::ortho(-halfW, halfW, -halfH, halfH, NearPlane, FarPlane);
    }
};

// --- LightComponent -------------------------------------------------------
enum class LightType : uint8_t {
    Directional,
    Point,
    Spot
};

struct LightComponent {
    LightType Type      = LightType::Directional;
    glm::vec3 Color     { 1.0f, 1.0f, 1.0f };
    float     Intensity = 1.0f;

    // Attenuation (Point / Spot)
    float Range     = 10.0f;
    float Constant  = 1.0f;
    float Linear    = 0.09f;
    float Quadratic = 0.032f;

    // Spot light
    float InnerCutoff = 12.5f; // degrees
    float OuterCutoff = 17.5f; // degrees

    // Shadows
    bool  CastShadows       = true;
    int   ShadowMapSize     = 2048;
    float ShadowBias        = 0.005f;
    float ShadowNearPlane   = 0.1f;
};

// --- MaterialComponent ----------------------------------------------------
struct MaterialComponent {
    glm::vec3 Albedo    { 0.8f, 0.8f, 0.8f };
    float     Metallic   = 0.0f;
    float     Roughness  = 0.5f;
    float     AO         = 1.0f;
    glm::vec3 Emissive  { 0.0f };
    float     EmissiveStrength = 0.0f;

    std::string AlbedoMapPath;
    std::string NormalMapPath;
    std::string MetallicRoughnessMapPath;
    std::string AOMapPath;
    std::string EmissiveMapPath;

    bool TwoSided = false;
    float AlphaCutoff = 0.5f;
};

// --- RigidBodyComponent --------------------------------------------------
enum class BodyType : uint8_t {
    Static,
    Dynamic,
    Kinematic
};

struct RigidBodyComponent {
    BodyType Type = BodyType::Dynamic;
    float Mass           = 1.0f;
    float LinearDamping  = 0.01f;
    float AngularDamping = 0.01f;
    float Friction       = 0.5f;
    float Restitution    = 0.3f;
    bool  UseGravity     = true;
    bool  ContinuousCD   = false;
    bool  FreezeRotationX = false;
    bool  FreezeRotationY = false;
    bool  FreezeRotationZ = false;

    // Runtime physics handle
    uint32_t RuntimeBodyHandle = 0;
};

// --- ColliderComponent ---------------------------------------------------
enum class ColliderShape : uint8_t {
    Box,
    Sphere,
    Capsule,
    Cylinder,
    Plane,
    Mesh,
    Heightfield
};

struct ColliderComponent {
    ColliderShape Shape = ColliderShape::Box;

    // Shape params (union-like, use based on Shape)
    glm::vec3 BoxHalfExtents { 0.5f, 0.5f, 0.5f };
    float SphereRadius  = 0.5f;
    float CapsuleRadius = 0.5f;
    float CapsuleHeight = 1.0f;

    glm::vec3 Center { 0.0f };  // Local offset
    bool IsTrigger = false;

    // Runtime physics handle
    uint32_t RuntimeShapeHandle = 0;
};

// --- ScriptComponent ----------------------------------------------------
struct ScriptComponent {
    std::string ClassName;      // e.g., "PlayerController"
    std::string ScriptPath;     // e.g., "Scripts/PlayerController.lua"
    std::string Language = "lua"; // "lua" or "csharp"

    // Runtime script instance pointer (managed by scripting engine)
    void* RuntimeInstance = nullptr;
};

// --- NativeScriptComponent ----------------------------------------------
class NativeScript {
public:
    virtual ~NativeScript() = default;
    virtual void OnCreate() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnFixedUpdate(float dt) {}
    virtual void OnDestroy() {}

    entt::entity Entity = entt::null;
    class Scene* ScenePtr = nullptr;
};

struct NativeScriptComponent {
    NativeScript* Instance = nullptr;
    std::function<NativeScript*()> CreateScript;
    std::function<void(NativeScript*)> DestroyScript;

    template<typename T>
    void Bind() {
        CreateScript  = []() { return static_cast<NativeScript*>(new T()); };
        DestroyScript = [](NativeScript* s) { delete s; };
    }
};

// --- AudioSourceComponent -----------------------------------------------
struct AudioSourceComponent {
    std::string ClipPath;
    float Volume    = 1.0f;
    float Pitch     = 1.0f;
    float MinDistance = 1.0f;
    float MaxDistance = 100.0f;
    bool  Loop       = false;
    bool  PlayOnAwake = false;
    bool  Spatial     = true;  // 3D spatial audio

    // Runtime handle
    uint32_t RuntimeSourceHandle = 0;
};

// --- AudioListenerComponent ---------------------------------------------
struct AudioListenerComponent {
    bool Active = true;
};

// --- TerrainComponent ---------------------------------------------------
struct TerrainComponent {
    std::string HeightmapPath;
    float Width   = 256.0f;
    float Height  = 64.0f;  // max terrain height
    float Length  = 256.0f;
    int   Resolution = 256; // vertices per side

    // Texture splatting
    struct SplatLayer {
        std::string TexturePath;
        std::string NormalPath;
        float TileScale = 10.0f;
    };
    std::vector<SplatLayer> SplatLayers;

    // Runtime handle
    uint64_t RuntimeTerrainHandle = 0;
};

// --- SkyboxComponent ----------------------------------------------------
struct SkyboxComponent {
    std::string CubemapPath;    // 6-face cubemap or equirectangular HDR
    float       Intensity = 1.0f;
    float       Rotation  = 0.0f; // Y-axis rotation in degrees
};

// --- ParticleEmitterComponent (stub for future) ------------------------
struct ParticleEmitterComponent {
    std::string EffectPath;
    bool        Playing = false;
    uint32_t    MaxParticles = 1000;
};

// ════════════════════════════════════════════════════════════════════════
//  2D PIPELINE COMPONENTS
// ════════════════════════════════════════════════════════════════════════

// --- SpriteRendererComponent ---------------------------------------------
struct SpriteRendererComponent {
    glm::vec4   Color        { 1.0f, 1.0f, 1.0f, 1.0f };
    std::string TexturePath;                // asset path → AssetManager
    glm::vec2   TilingFactor { 1.0f, 1.0f };
    glm::vec2   Offset       { 0.0f, 0.0f };   // UV offset (scrolling)

    // Flip
    bool FlipX = false;
    bool FlipY = false;

    // Layer / sorting
    int  SortingLayer = 0;   // higher = drawn later (on top)
    int  OrderInLayer = 0;

    bool Visible = true;

    // Runtime (set by AssetManager)
    uint64_t TextureHandle = 0;
};

// --- SpriteAnimatorComponent ---------------------------------------------
// Holds Animator2D runtime state.  Include Animation2D.h to use.
struct SpriteAnimatorComponent {
    std::string DefaultClip;
    bool        AutoPlay = true;
    // Runtime animator (created by Render2DSystem on first frame)
    void* RuntimeAnimator = nullptr; // Animator2D*
};

// --- Text2DComponent ------------------------------------------------------
struct Text2DComponent {
    std::string Text;
    std::string FontPath;
    glm::vec4   Color        { 1.0f, 1.0f, 1.0f, 1.0f };
    float       FontSize     = 32.0f;
    float       Kerning      = 0.0f;
    float       LineSpacing  = 1.2f;
    // Horizontal align: 0=left, 0.5=center, 1=right
    float       AlignH       = 0.0f;
    bool        Visible      = true;

    // Runtime font handle
    uint64_t FontHandle = 0;
};

// --- Camera2DComponent ----------------------------------------------------
// Dedicated orthographic 2D camera (separate from the 3D CameraComponent).
struct Camera2DComponent {
    float    Size        = 5.0f;   // half-height in world units
    float    Near        = -1.0f;
    float    Far         = 1.0f;
    bool     Primary     = false;
    bool     FixedAspect = false;
    float    AspectRatio = 16.0f / 9.0f;
    glm::vec4 Background { 0.1f, 0.1f, 0.12f, 1.0f };
};

// --- Rigidbody2DComponent -------------------------------------------------
enum class BodyType2D : uint8_t { Static, Dynamic, Kinematic };

struct Rigidbody2DComponent {
    BodyType2D Type            = BodyType2D::Dynamic;
    float      Mass            = 1.0f;
    float      LinearDamping   = 0.0f;
    float      AngularDamping  = 0.01f;
    float      GravityScale    = 1.0f;
    bool       FixedRotation   = false;
    bool       Bullet          = false; // continuous collision detection

    // Runtime (Box2D body pointer cast to void*)
    void*      RuntimeBody = nullptr;
};

// --- BoxCollider2DComponent -----------------------------------------------
struct BoxCollider2DComponent {
    glm::vec2 Offset   { 0.0f, 0.0f };
    glm::vec2 Size     { 0.5f, 0.5f };   // half-extents
    float     Density    = 1.0f;
    float     Friction   = 0.3f;
    float     Restitution = 0.0f;
    bool      IsTrigger  = false;
    void*     RuntimeFixture = nullptr;
};

// --- CircleCollider2DComponent --------------------------------------------
struct CircleCollider2DComponent {
    glm::vec2 Offset   { 0.0f, 0.0f };
    float     Radius     = 0.5f;
    float     Density    = 1.0f;
    float     Friction   = 0.3f;
    float     Restitution = 0.0f;
    bool      IsTrigger  = false;
    void*     RuntimeFixture = nullptr;
};

} // namespace Freely
