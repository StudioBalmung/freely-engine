#include "Freely/ECS/Systems/Render2DSystem.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Renderer2D/Renderer2D.h"
#include "Freely/Renderer2D/Animation2D.h"
#include "Freely/Renderer2D/SpriteSheet.h"
#include "Freely/Renderer2D/Font.h"
#include "Freely/Renderer/Texture.h"
#include "Freely/Scene/Camera.h"
#include "Freely/Core/Engine.h"
#include "Freely/Core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <vector>

namespace Freely {

// ─── Sortable sprite entry ────────────────────────────────────────────────
struct SpriteEntry {
    entt::entity Entity;
    int          SortingLayer;
    int          OrderInLayer;
    float        ZDepth;   // tie-break by world Z
};

void Render2DSystem::OnLateUpdate(Scene& scene, float deltaTime) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();

    // ── 1. Find primary 2D camera ─────────────────────────────────────────
    Camera renderCamera;
    bool foundCamera = false;

    auto camView = reg.view<TransformComponent, Camera2DComponent>();
    for (auto e : camView) {
        auto [tf, cam2d] = camView.get<TransformComponent, Camera2DComponent>(e);
        if (!cam2d.Primary) continue;

        // Build orthographic camera from Camera2DComponent
        float aspect = cam2d.FixedAspect
                       ? cam2d.AspectRatio
                       : Engine::Get().GetWindow().GetAspectRatio();
        renderCamera.SetOrthographic(cam2d.Size * 2.0f, cam2d.Near, cam2d.Far);
        renderCamera.SetPosition(tf.Position);
        renderCamera.SetRotation(glm::degrees(glm::eulerAngles(tf.Rotation)));
        foundCamera = true;
        break;
    }

    // Fall back to the engine's main camera if no 2D camera is set
    if (!foundCamera) {
        auto camView3d = reg.view<TransformComponent, CameraComponent>();
        for (auto e : camView3d) {
            auto [tf, cam] = camView3d.get<TransformComponent, CameraComponent>(e);
            if (!cam.Primary) continue;
            float aspect = Engine::Get().GetWindow().GetAspectRatio();
            renderCamera.SetOrthographic(cam.OrthoSize * 2.0f, cam.NearPlane, cam.FarPlane);
            renderCamera.SetPosition(tf.Position);
            renderCamera.SetRotation(tf.GetEulerAngles());
            foundCamera = true;
            break;
        }
    }
    if (!foundCamera) return;

    // ── 2. Tick animators ─────────────────────────────────────────────────
    TickAnimators(scene, deltaTime);

    // ── 3. Collect + sort sprites ─────────────────────────────────────────
    Renderer2D::BeginScene(renderCamera);
    FlushSortedSprites(scene);
    FlushText(scene);
    Renderer2D::EndScene();
}

// ─── Tick all sprite animators ───────────────────────────────────────────────
void Render2DSystem::TickAnimators(Scene& scene, float deltaTime) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<SpriteRendererComponent, SpriteAnimatorComponent>();
    for (auto e : view) {
        auto [sprite, anim] = view.get<SpriteRendererComponent, SpriteAnimatorComponent>(e);

        // Lazy-init animator
        if (!anim.RuntimeAnimator) continue;

        auto* animator = reinterpret_cast<Animator2D*>(anim.RuntimeAnimator);
        bool dirty = animator->Tick(deltaTime);
        if (dirty) {
            // The sheet/frame is read in FlushSortedSprites each draw - nothing extra needed
        }
    }
}

// ─── Sort and flush sprites ───────────────────────────────────────────────────
void Render2DSystem::FlushSortedSprites(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();

    // Gather
    std::vector<SpriteEntry> entries;
    auto view = reg.view<TransformComponent, SpriteRendererComponent>();
    for (auto e : view) {
        auto [tf, spr] = view.get<TransformComponent, SpriteRendererComponent>(e);
        if (!spr.Visible) continue;
        entries.push_back({ e, spr.SortingLayer, spr.OrderInLayer, tf.Position.z });
    }

    // Sort
    std::stable_sort(entries.begin(), entries.end(), [](const SpriteEntry& a, const SpriteEntry& b) {
        if (a.SortingLayer != b.SortingLayer) return a.SortingLayer < b.SortingLayer;
        if (a.OrderInLayer != b.OrderInLayer) return a.OrderInLayer < b.OrderInLayer;
        return a.ZDepth < b.ZDepth;
    });

    // Draw
    for (auto& entry : entries) {
        auto& tf  = reg.get<TransformComponent>(entry.Entity);
        auto& spr = reg.get<SpriteRendererComponent>(entry.Entity);

        // Build transform matrix from TransformComponent
        glm::mat4 t = tf.WorldMatrix;

        // Check if entity has a SpriteAnimatorComponent with a live animator
        if (reg.any_of<SpriteAnimatorComponent>(entry.Entity)) {
            auto& animComp = reg.get<SpriteAnimatorComponent>(entry.Entity);
            if (animComp.RuntimeAnimator) {
                auto* animator = reinterpret_cast<Animator2D*>(animComp.RuntimeAnimator);
                auto sheet = animator->GetCurrentSheet();
                if (sheet) {
                    auto uvCoord = animator->GetCurrentTileCoord();
                    Renderer2D::DrawSubSprite(t, sheet, uvCoord, spr.Color);
                    continue;
                }
            }
        }

        // Static sprite
        if (spr.TextureHandle != 0) {
            // TODO[Render2DSystem]: Resolve texture handle from AssetManager (GetTexture2D) and call Renderer2D::DrawSprite
            // For now fall through to flat color rect
        }

        Renderer2D::DrawRect(t, spr.Color);
    }
}

// ─── Flush text ───────────────────────────────────────────────────────────────
void Render2DSystem::FlushText(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, Text2DComponent>();
    for (auto e : view) {
        auto [tf, txt] = view.get<TransformComponent, Text2DComponent>(e);
        if (!txt.Visible || txt.Text.empty()) continue;
        // Font resolved via AssetManager (placeholder: skip if no handle)
        if (txt.FontHandle == 0) continue;
        // TODO[Render2DSystem]: Resolve font from AssetManager (GetFont) and call Renderer2D::DrawString
        // Renderer2D::DrawString(txt.Text, font, tf.WorldMatrix, txt.Color, txt.Kerning, txt.LineSpacing);
    }
}

} // namespace Freely
