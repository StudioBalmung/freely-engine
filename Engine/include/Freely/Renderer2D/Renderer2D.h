#pragma once
// Freely Engine - Renderer2D
// Batched 2D renderer.  Accumulates up to kMaxQuads quads per batch,
// supporting up to kMaxTextures simultaneous texture bindings per batch.
// Shapes (circles, lines, rounded rects) ride the same vertex stream and
// are resolved entirely in the fragment shader.
//
// Usage (one frame):
//   Renderer2D::BeginScene(camera);
//   Renderer2D::DrawSprite(transform, texture, color, tiling);
//   Renderer2D::DrawCircle(transform, color, thickness, fade);
//   Renderer2D::DrawLine(p0, p1, color, thickness);
//   Renderer2D::DrawRect(transform, color);
//   Renderer2D::DrawString(text, font, transform, color, kerning);
//   Renderer2D::EndScene();

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Freely {

class Camera;
class Texture2D;
class Font;
class SpriteSheet;

// ─── Statistics ───────────────────────────────────────────────────────────────
struct RenderStats2D {
    uint32_t DrawCalls = 0;
    uint32_t QuadCount = 0;
    uint32_t LineCount = 0;
};

// ─── Renderer2D ──────────────────────────────────────────────────────────────
// Internal batch-renderer helper, defined in Renderer2D.cpp.
void PushQuad(const glm::mat4&, const glm::vec4*, const glm::vec2*,
              const glm::vec4&, float, float, const glm::vec2*,
              float, float, int, int);

class Renderer2D {
public:
    static void Init();
    static void Shutdown();

    /// Begin a frame with the given camera.
    static void BeginScene(const Camera& camera);

    /// Flush all queued geometry and end the frame.
    static void EndScene();

    // ── Sprite / Quad ─────────────────────────────────────────────────────
    /// Draw a full-texture quad.
    static void DrawSprite(const glm::mat4& transform,
                           std::shared_ptr<Texture2D> texture,
                           const glm::vec4& tintColor   = {1,1,1,1},
                           float            tilingFactor = 1.0f);

    /// Draw a solid-color quad (uses white 1�-1 texture).
    static void DrawRect(const glm::mat4& transform,
                         const glm::vec4& color = {1,1,1,1});

    /// Draw a sub-region of a sprite-sheet.
    static void DrawSubSprite(const glm::mat4&           transform,
                              std::shared_ptr<SpriteSheet> sheet,
                              const glm::vec2&           coords,    // tile col, row
                              const glm::vec4&           tintColor  = {1,1,1,1});

    // ── Shapes ────────────────────────────────────────────────────────────
    /// Draw a filled or hollow circle.
    ///   thickness = 1.0 → filled disc; < 1.0 → ring
    ///   fade = anti-alias width (0.01 is good)
    static void DrawCircle(const glm::mat4& transform,
                           const glm::vec4& color,
                           float            thickness = 1.0f,
                           float            fade      = 0.005f);

    /// Draw a thick line between two world-space points.
    static void DrawLine(const glm::vec3& p0,
                         const glm::vec3& p1,
                         const glm::vec4& color     = {1,1,1,1},
                         float            thickness = 0.02f);

    /// Draw a wire rectangle (axis-aligned in local space).
    static void DrawRectOutline(const glm::mat4& transform,
                                const glm::vec4& color = {1,1,1,1});

    // ── Text ──────────────────────────────────────────────────────────────
    /// Render a UTF-8 string using an SDF font atlas.
    static void DrawString(const std::string&        text,
                           std::shared_ptr<Font>     font,
                           const glm::mat4&          transform,
                           const glm::vec4&          color    = {1,1,1,1},
                           float                     kerning  = 0.0f,
                           float                     lineSpacing = 1.0f);

    // ── Stats ─────────────────────────────────────────────────────────────
    static void ResetStats();
    static const RenderStats2D& GetStats();

private:
    friend void PushQuad(const glm::mat4&, const glm::vec4*, const glm::vec2*,
                          const glm::vec4&, float, float, const glm::vec2*,
                          float, float, int, int);

    static void Flush();          // Submit the current batch
    static void FlushText();      // Submit the text batch separately
    static void StartBatch();
    static void NextBatch();
};

} // namespace Freely
