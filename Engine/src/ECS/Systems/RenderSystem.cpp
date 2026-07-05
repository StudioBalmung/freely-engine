#include "Freely/ECS/Systems/RenderSystem.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Renderer/Renderer3D.h"
#include "Freely/Scene/Mesh.h"
#include "Freely/Scene/Material.h"
#include "Freely/Scene/Camera.h"
#include "Freely/Renderer/Shader.h"
#include "Freely/Core/Engine.h"
#include "Freely/Core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Freely {

// ─── Mesh primitive cache (lazy-created) ─────────────────────────────────────
static std::shared_ptr<Mesh> GetPrimitive(PrimitiveMeshType type) {
    static std::shared_ptr<Mesh> sCube;
    static std::shared_ptr<Mesh> sSphere;
    static std::shared_ptr<Mesh> sPlane;
    switch (type) {
        case PrimitiveMeshType::Cube:
            if (!sCube)   sCube   = Mesh::CreateCube();   return sCube;
        case PrimitiveMeshType::Sphere:
            if (!sSphere) sSphere = Mesh::CreateSphere(); return sSphere;
        case PrimitiveMeshType::Plane:
            if (!sPlane)  sPlane  = Mesh::CreatePlane();  return sPlane;
        default: return nullptr;
    }
}

// ─── Default PBR material (white, mid roughness) ─────────────────────────────
static std::shared_ptr<Material> GetDefaultMaterial() {
    static std::shared_ptr<Material> sMat;
    if (!sMat) {
        sMat = std::make_shared<Material>();
        sMat->Albedo    = {0.8f, 0.8f, 0.8f};
        sMat->Metallic  = 0.0f;
        sMat->Roughness = 0.5f;
        sMat->AO        = 1.0f;
        // ShaderProgram is set by Renderer3D internally (PBR shader)
    }
    return sMat;
}

// ─── Build SceneLights from LightComponent entities ───────────────────────────
static SceneLights CollectLights(entt::registry& reg) {
    SceneLights lights;

    auto view = reg.view<TransformComponent, LightComponent>();
    for (auto e : view) {
        auto [tf, lc] = view.get<TransformComponent, LightComponent>(e);

        switch (lc.Type) {
        case LightType::Directional: {
            DirectionalLight dl;
            dl.Direction    = tf.GetForward();
            dl.Color        = lc.Color;
            dl.Intensity    = lc.Intensity;
            dl.CastShadow   = lc.CastShadows;
            dl.ShadowBias   = lc.ShadowBias;
            lights.Directional.push_back(dl);
            break;
        }
        case LightType::Point: {
            PointLight pl;
            pl.Position  = tf.Position;
            pl.Color     = lc.Color;
            pl.Intensity = lc.Intensity;
            pl.Range     = lc.Range;
            pl.Constant  = lc.Constant;
            pl.Linear    = lc.Linear;
            pl.Quadratic = lc.Quadratic;
            lights.Point.push_back(pl);
            break;
        }
        case LightType::Spot: {
            SpotLight sl;
            sl.Position     = tf.Position;
            sl.Direction    = tf.GetForward();
            sl.Color        = lc.Color;
            sl.Intensity    = lc.Intensity;
            sl.Range        = lc.Range;
            sl.InnerCutoff  = lc.InnerCutoff;
            sl.OuterCutoff  = lc.OuterCutoff;
            lights.Spot.push_back(sl);
            break;
        }
        }
    }

    lights.AmbientColor    = {0.03f, 0.03f, 0.03f};
    lights.AmbientStrength = 1.0f;
    return lights;
}

// ─── Main render dispatch ─────────────────────────────────────────────────────
void RenderSystem::OnLateUpdate(Scene& scene, float deltaTime) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();

    // ── 1. Resolve active camera ──────────────────────────────────────────
    Camera renderCamera;
    bool foundCamera = false;

    if (m_EditorCamera) {
        renderCamera = *m_EditorCamera;
        foundCamera  = true;
    } else {
        auto view = reg.view<TransformComponent, CameraComponent>();
        for (auto e : view) {
            auto [tf, cc] = view.get<TransformComponent, CameraComponent>(e);
            if (!cc.Primary) continue;

            float aspect = Engine::Get().GetWindow().GetAspectRatio();
            if (cc.Projection == ProjectionType::Perspective)
                renderCamera.SetPerspective(cc.FOV, aspect, cc.NearPlane, cc.FarPlane);
            else
                renderCamera.SetOrthographic(cc.OrthoSize, cc.NearPlane, cc.FarPlane);

            renderCamera.SetPosition(tf.Position);
            renderCamera.SetRotation(tf.GetEulerAngles());
            foundCamera = true;
            break;
        }
    }
    if (!foundCamera) return;

    // ── 2. Collect lights ─────────────────────────────────────────────────
    SceneLights lights = CollectLights(reg);

    // ── 3. Viewport size for Renderer3D ──────────────────────────────────
    auto [vpW, vpH] = Engine::Get().GetWindow().GetSize();
    Renderer3D::SetViewportSize(vpW, vpH);

    // ── 4. Begin 3D scene ─────────────────────────────────────────────────
    Renderer3D::BeginScene(renderCamera, lights);

    // ── 5. Submit all renderable entities ─────────────────────────────────
    {
        auto meshView = reg.view<TransformComponent, MeshRendererComponent>();
        for (auto e : meshView) {
            auto [tf, mr] = meshView.get<TransformComponent, MeshRendererComponent>(e);
            if (!mr.Visible) continue;

            std::shared_ptr<Mesh> mesh;

            // A. Primitive mesh via MeshFilterComponent
            if (reg.any_of<MeshFilterComponent>(e)) {
                auto& mf = reg.get<MeshFilterComponent>(e);
                mesh = GetPrimitive(mf.PrimitiveType);
                if (!mesh && !mf.CustomMeshPath.empty())
                    mesh = Mesh::LoadFromFile(mf.CustomMeshPath);
            }

            // TODO[RenderSystem]: Retrieve mesh from AssetManager via mr.MeshHandle and mr.MeshAsset path

            if (!mesh) continue;

            // Material from MaterialComponent or default
            std::shared_ptr<Material> mat;
            if (reg.any_of<MaterialComponent>(e)) {
                auto& mc = reg.get<MaterialComponent>(e);
                // TODO[RenderSystem]: Resolve texture handles from mc.xxxMapPath strings via AssetManager (ImportTexture)
                mat = std::make_shared<Material>();
                mat->Albedo    = mc.Albedo;
                mat->Metallic  = mc.Metallic;
                mat->Roughness = mc.Roughness;
                mat->AO        = mc.AO;
                mat->Emissive  = mc.Emissive;
                mat->EmissiveStrength = mc.EmissiveStrength;
            } else {
                mat = GetDefaultMaterial();
            }

            Renderer3D::Submit(mesh, mat, tf.WorldMatrix);
        }
    }

    // ── 6. Skybox ─────────────────────────────────────────────────────────
    {
        auto skyView = reg.view<SkyboxComponent>();
        for (auto e : skyView) {
            auto& sky = reg.get<SkyboxComponent>(e);
            if (sky.CubemapPath.empty()) continue;
            // TODO[RenderSystem]: Resolve cubemap asset via AssetManager and pass handle to Renderer3D::DrawSkybox
            // Renderer3D::DrawSkybox(cubemapID);
            break; // only one skybox
        }
    }

    // ── 7. Editor grid (only when editor camera is active) ────────────────
    if (m_EditorCamera && m_ShowGrid)
        Renderer3D::DrawGrid(1.0f);

    // ── 8. Flush all 3D draw calls ────────────────────────────────────────
    Renderer3D::EndScene();
}

} // namespace Freely
