#pragma once
// Freely Engine - Renderer3D
// Deferred-light, forward-render 3D pipeline.
//
// Usage (one frame):
//   Renderer3D::BeginScene(editorCamera, lights);
//   Renderer3D::Submit(mesh, material, transform);   // N times
//   Renderer3D::DrawSkybox(cubemap);                  // optional
//   Renderer3D::DrawGrid();                           // optional (editor)
//   Renderer3D::EndScene();
//
// Internally:
//   1. Shadow pass  - render depth for every shadow-casting directional light
//   2. Opaque pass  - PBR forward pass, sorted front-to-back
//   3. Skybox pass  - rendered after opaque, depth test = <=
//   4. Transparent  - sorted back-to-front, blended

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Freely {

class Shader;
class Mesh;
class Material;
class Texture2D;
class Camera;
class UniformBuffer;
class Framebuffer;

// --- Light descriptor (max 32 point + 4 directional + 8 spot) ---------------
struct DirectionalLight {
    glm::vec3 Direction  {-0.5f, -1.0f, -0.5f};
    float     Intensity  = 1.0f;
    glm::vec3 Color      {1.0f};
    bool      CastShadow = true;
    float     ShadowBias = 0.005f;
};

struct PointLight {
    glm::vec3 Position;
    float     Intensity  = 1.0f;
    glm::vec3 Color      {1.0f};
    float     Range      = 10.0f;
    float     Constant   = 1.0f;
    float     Linear     = 0.09f;
    float     Quadratic  = 0.032f;
};

struct SpotLight {
    glm::vec3 Position;
    glm::vec3 Direction;
    float     Intensity   = 1.0f;
    glm::vec3 Color       {1.0f};
    float     Range       = 20.0f;
    float     InnerCutoff = 12.5f; // degrees
    float     OuterCutoff = 17.5f; // degrees
};

struct SceneLights {
    std::vector<DirectionalLight> Directional;
    std::vector<PointLight>       Point;
    std::vector<SpotLight>        Spot;
    glm::vec3                     AmbientColor  {0.03f};
    float                         AmbientStrength = 1.0f;
};

// --- Draw call descriptor ----------------------------------------------------
struct DrawCall3D {
    std::shared_ptr<Mesh>     MeshPtr;
    std::shared_ptr<Material> MaterialPtr;
    glm::mat4                 Transform;
    float                     DistanceSq = 0.0f;
    bool                      IsTransparent = false;
};

// --- Statistics --------------------------------------------------------------
struct RenderStats3D {
    uint32_t DrawCalls      = 0;
    uint32_t VertexCount    = 0;
    uint32_t IndexCount     = 0;
    uint32_t ShadowPasses   = 0;
};

// --- Renderer3D (all-static interface) ---------------------------------------
class Renderer3D {
public:
    static void Init();
    static void Shutdown();

    /// Call once per frame with the active camera and scene lights.
    static void BeginScene(const Camera& camera, const SceneLights& lights = {});

    /// Queue a mesh for rendering.  Call between BeginScene / EndScene.
    static void Submit(std::shared_ptr<Mesh>     mesh,
                       std::shared_ptr<Material> material,
                       const glm::mat4&          transform);

    /// Draw an infinitely-tiling editor grid (opaque pass).
    static void DrawGrid(float scale = 1.0f);

    /// Draw a cube-map skybox (runs after the opaque pass).
    static void DrawSkybox(uint32_t cubemapID);

    /// Flush all queued draw calls.
    static void EndScene();

    /// Reset per-frame statistics.
    static void ResetStats();
    static const RenderStats3D& GetStats();

    // -- Utility --------------------------------------------------------------
    /// Override the viewport dimensions used for camera matrices.
    static void SetViewportSize(uint32_t width, uint32_t height);

private:
    // Shadow pass for a single directional light
    static void ShadowPass(const DirectionalLight& light);
    // Render all queued opaque meshes
    static void OpaquePass();
    // Render all queued transparent meshes
    static void TransparentPass();

    // Shader builders (embedded GLSL, no file I/O needed)
    static std::shared_ptr<Shader> BuildPBRShader();
    static std::shared_ptr<Shader> BuildShadowShader();
    static std::shared_ptr<Shader> BuildSkyboxShader();
    static std::shared_ptr<Shader> BuildGridShader();
};

} // namespace Freely
