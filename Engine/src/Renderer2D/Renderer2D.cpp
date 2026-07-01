#include "Freely/Renderer2D/Renderer2D.h"
#include "Freely/Renderer2D/Font.h"
#include "Freely/Renderer2D/SpriteSheet.h"
#include "Freely/Renderer/Shader.h"
#include "Freely/Renderer/Texture.h"
#include "Freely/Renderer/VertexArray.h"
#include "Freely/Renderer/Buffer.h"
#include "Freely/Renderer/UniformBuffer.h"
#include "Freely/Scene/Camera.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cstring>

namespace Freely {

// ─── Internal constants ────────────────────────────────────────────────────
namespace R2D {

static constexpr uint32_t kMaxQuads    = 10'000;
static constexpr uint32_t kMaxVertices = kMaxQuads * 4;
static constexpr uint32_t kMaxIndices  = kMaxQuads * 6;
static constexpr uint32_t kMaxTexSlots = 32;  // driver minimum guarantee
static constexpr uint32_t kCameraBinding = 0;

// ── Vertex format (one struct, different quad types select via e_Type) ──────
// e_Type: 0=sprite, 1=circle, 2=SDF text
struct QuadVertex {
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoord;
    float     TexIndex;      // index into texture array
    float     TilingFactor;
    // For circles / SDF: reuse TexCoord as local-space UV in [-1,1]
    glm::vec2 LocalUV;
    float     Thickness;     // circle: 0..1; sdf: softness
    float     Fade;          // circle anti-alias; sdf: unused (0)
    int       EntityID;      // for mouse-picking (editor)
    int       QuadType;      // 0=sprite 1=circle 2=text
};

// ── Camera UBO ─────────────────────────────────────────────────────────────
struct CameraData {
    glm::mat4 ViewProjection;
};

// ── Per-quad default UV corners ────────────────────────────────────────────
static const glm::vec2 kQuadUV[4] = {
    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
};
static const glm::vec4 kQuadPositions[4] = {
    {-0.5f, -0.5f, 0.0f, 1.0f},
    { 0.5f, -0.5f, 0.0f, 1.0f},
    { 0.5f,  0.5f, 0.0f, 1.0f},
    {-0.5f,  0.5f, 0.0f, 1.0f},
};
static const glm::vec2 kCircleLocalUV[4] = {
    {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}
};

// ── GLSL embedded shaders ──────────────────────────────────────────────────
static const char* kQuad_Vert = R"GLSL(
#version 450 core
layout(location=0) in vec3  a_Position;
layout(location=1) in vec4  a_Color;
layout(location=2) in vec2  a_TexCoord;
layout(location=3) in float a_TexIndex;
layout(location=4) in float a_TilingFactor;
layout(location=5) in vec2  a_LocalUV;
layout(location=6) in float a_Thickness;
layout(location=7) in float a_Fade;
layout(location=8) in int   a_EntityID;
layout(location=9) in int   a_QuadType;

layout(std140, binding=0) uniform Camera2D { mat4 u_ViewProjection; };

out vec4  v_Color;
out vec2  v_TexCoord;
flat out float v_TexIndex;
out float v_TilingFactor;
out vec2  v_LocalUV;
out float v_Thickness;
out float v_Fade;
flat out int   v_EntityID;
flat out int   v_QuadType;

void main() {
    v_Color         = a_Color;
    v_TexCoord      = a_TexCoord;
    v_TexIndex      = a_TexIndex;
    v_TilingFactor  = a_TilingFactor;
    v_LocalUV       = a_LocalUV;
    v_Thickness     = a_Thickness;
    v_Fade          = a_Fade;
    v_EntityID      = a_EntityID;
    v_QuadType      = a_QuadType;
    gl_Position     = u_ViewProjection * vec4(a_Position, 1.0);
}
)GLSL";

static const char* kQuad_Frag = R"GLSL(
#version 450 core
out vec4 FragColor;
out int  o_EntityID;

in vec4  v_Color;
in vec2  v_TexCoord;
flat in float v_TexIndex;
in float v_TilingFactor;
in vec2  v_LocalUV;
in float v_Thickness;
in float v_Fade;
flat in int  v_EntityID;
flat in int  v_QuadType;

uniform sampler2D u_Textures[32];

vec4 SampleTexture(int index, vec2 uv) {
    switch (index) {
        case  0: return texture(u_Textures[ 0], uv);
        case  1: return texture(u_Textures[ 1], uv);
        case  2: return texture(u_Textures[ 2], uv);
        case  3: return texture(u_Textures[ 3], uv);
        case  4: return texture(u_Textures[ 4], uv);
        case  5: return texture(u_Textures[ 5], uv);
        case  6: return texture(u_Textures[ 6], uv);
        case  7: return texture(u_Textures[ 7], uv);
        case  8: return texture(u_Textures[ 8], uv);
        case  9: return texture(u_Textures[ 9], uv);
        case 10: return texture(u_Textures[10], uv);
        case 11: return texture(u_Textures[11], uv);
        case 12: return texture(u_Textures[12], uv);
        case 13: return texture(u_Textures[13], uv);
        case 14: return texture(u_Textures[14], uv);
        case 15: return texture(u_Textures[15], uv);
        case 16: return texture(u_Textures[16], uv);
        case 17: return texture(u_Textures[17], uv);
        case 18: return texture(u_Textures[18], uv);
        case 19: return texture(u_Textures[19], uv);
        case 20: return texture(u_Textures[20], uv);
        case 21: return texture(u_Textures[21], uv);
        case 22: return texture(u_Textures[22], uv);
        case 23: return texture(u_Textures[23], uv);
        case 24: return texture(u_Textures[24], uv);
        case 25: return texture(u_Textures[25], uv);
        case 26: return texture(u_Textures[26], uv);
        case 27: return texture(u_Textures[27], uv);
        case 28: return texture(u_Textures[28], uv);
        case 29: return texture(u_Textures[29], uv);
        case 30: return texture(u_Textures[30], uv);
        case 31: return texture(u_Textures[31], uv);
        default: return vec4(1.0);
    }
}

void main() {
    vec4 color = v_Color;

    if (v_QuadType == 0) {
        // ── Sprite ──────────────────────────────────────────────────────
        vec4 texColor = SampleTexture(int(v_TexIndex), v_TexCoord * v_TilingFactor);
        color *= texColor;

    } else if (v_QuadType == 1) {
        // ── Circle / Ring ────────────────────────────────────────────────
        float dist = 1.0 - length(v_LocalUV);
        float alpha = smoothstep(0.0, v_Fade, dist);
        alpha      *= smoothstep(v_Thickness + v_Fade, v_Thickness, dist);
        if (alpha < 0.001) discard;
        color.a *= alpha;

    } else if (v_QuadType == 2) {
        // ── SDF Text ─────────────────────────────────────────────────────
        float dist   = SampleTexture(int(v_TexIndex), v_TexCoord).r;
        float smooth_w = fwidth(dist) * 0.5 + v_Fade;
        float alpha  = smoothstep(0.5 - smooth_w, 0.5 + smooth_w, dist);
        if (alpha < 0.001) discard;
        color.a *= alpha;
    }

    if (color.a < 0.01) discard;
    FragColor   = color;
    o_EntityID  = v_EntityID;
}
)GLSL";

// ── Batch state ──────────────────────────────────────────────────────────────
struct BatchState {
    // Geometry
    std::shared_ptr<VertexArray>  VAO;
    std::shared_ptr<VertexBuffer> VBO;
    std::unique_ptr<QuadVertex[]> VertexBase;
    QuadVertex*                   VertexPtr = nullptr;
    uint32_t                      IndexCount = 0;

    // Textures
    std::array<uint32_t, kMaxTexSlots> TextureIDs{};
    uint32_t TextureSlotIndex = 1; // slot 0 = white

    // Shared
    std::shared_ptr<Shader>       QuadShader;
    std::shared_ptr<UniformBuffer> CameraUBO;
    std::shared_ptr<Texture2D>    WhiteTexture;

    RenderStats2D Stats;
    bool Initialized = false;
};

static BatchState s;

} // namespace R2D

// ─── Renderer2D implementation ────────────────────────────────────────────────

void Renderer2D::Init() {
    using namespace R2D;
    if (s.Initialized) return;

    // ── Compile shader ──────────────────────────────────────────────────
    s.QuadShader = Shader::Create(kQuad_Vert, kQuad_Frag);

    // ── UBO ─────────────────────────────────────────────────────────────
    s.CameraUBO = UniformBuffer::Create(sizeof(CameraData), kCameraBinding);
    {
        uint32_t idx = glGetUniformBlockIndex(s.QuadShader->GetID(), "Camera2D");
        if (idx != GL_INVALID_INDEX)
            glUniformBlockBinding(s.QuadShader->GetID(), idx, kCameraBinding);
    }

    // ── Texture sampler array uniforms ───────────────────────────────────
    s.QuadShader->Bind();
    for (uint32_t i = 0; i < kMaxTexSlots; i++) {
        s.QuadShader->SetInt("u_Textures[" + std::to_string(i) + "]", (int)i);
    }

    // ── Pre-build index buffer ───────────────────────────────────────────
    std::vector<uint32_t> indices(kMaxIndices);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < kMaxIndices; i += 6) {
        indices[i+0] = offset + 0;
        indices[i+1] = offset + 1;
        indices[i+2] = offset + 2;
        indices[i+3] = offset + 2;
        indices[i+4] = offset + 3;
        indices[i+5] = offset + 0;
        offset += 4;
    }

    // ── VAO / VBO ────────────────────────────────────────────────────────
    s.VAO = std::make_shared<VertexArray>();

    s.VBO = std::make_shared<VertexBuffer>(kMaxVertices * (uint32_t)sizeof(QuadVertex));
    s.VBO->SetLayout({
        { ShaderDataType::Float3, "a_Position"     },
        { ShaderDataType::Float4, "a_Color"        },
        { ShaderDataType::Float2, "a_TexCoord"     },
        { ShaderDataType::Float,  "a_TexIndex"     },
        { ShaderDataType::Float,  "a_TilingFactor" },
        { ShaderDataType::Float2, "a_LocalUV"      },
        { ShaderDataType::Float,  "a_Thickness"    },
        { ShaderDataType::Float,  "a_Fade"         },
        { ShaderDataType::Int,    "a_EntityID"     },
        { ShaderDataType::Int,    "a_QuadType"     },
    });
    s.VAO->AddVertexBuffer(s.VBO);

    auto ibo = std::make_shared<IndexBuffer>(indices.data(), kMaxIndices);
    s.VAO->SetIndexBuffer(ibo);

    // ── CPU vertex buffer ────────────────────────────────────────────────
    s.VertexBase = std::make_unique<QuadVertex[]>(kMaxVertices);

    // ── White 1×1 texture (slot 0) ───────────────────────────────────────
    TextureSpec spec;
    spec.Width = spec.Height = 1;
    spec.GenerateMipmaps = false;
    spec.MinFilter = TextureFilter::Nearest;
    spec.MagFilter = TextureFilter::Nearest;
    uint32_t whiteData = 0xFFFFFFFF;
    s.WhiteTexture = std::make_shared<Texture2D>(spec, &whiteData);
    s.TextureIDs[0] = s.WhiteTexture->GetID();

    s.Initialized = true;
    FL_ENGINE_INFO("Renderer2D initialized (max {} quads, {} tex slots).", kMaxQuads, kMaxTexSlots);
}

void Renderer2D::Shutdown() {
    using namespace R2D;
    s = BatchState{};
    FL_ENGINE_INFO("Renderer2D shut down.");
}

void Renderer2D::BeginScene(const Camera& camera) {
    using namespace R2D;
    CameraData cd;
    cd.ViewProjection = camera.GetViewProjectionMatrix();
    s.CameraUBO->SetData(&cd, sizeof(cd));
    StartBatch();
}

void Renderer2D::EndScene() {
    Flush();
}

void Renderer2D::StartBatch() {
    using namespace R2D;
    s.VertexPtr      = s.VertexBase.get();
    s.IndexCount     = 0;
    s.TextureSlotIndex = 1; // 0 = white
}

void Renderer2D::Flush() {
    using namespace R2D;
    if (s.IndexCount == 0) return;

    uint32_t dataSize = (uint32_t)((uint8_t*)s.VertexPtr - (uint8_t*)s.VertexBase.get());
    s.VBO->SetData(s.VertexBase.get(), dataSize);

    // Bind all textures
    for (uint32_t i = 0; i < s.TextureSlotIndex; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, s.TextureIDs[i]);
    }

    s.QuadShader->Bind();
    s.VAO->Bind();
    glDrawElements(GL_TRIANGLES, s.IndexCount, GL_UNSIGNED_INT, nullptr);

    s.Stats.DrawCalls++;
    s.Stats.QuadCount += s.IndexCount / 6;
}

void Renderer2D::NextBatch() {
    Flush();
    StartBatch();
}

// ─── Internal helper: push a quad ────────────────────────────────────────────
void PushQuad(const glm::mat4&        transform,
                     const glm::vec4         positions[4],
                     const glm::vec2         uvs[4],
                     const glm::vec4&        color,
                     float                   texIndex,
                     float                   tilingFactor,
                     const glm::vec2         localUV[4],
                     float                   thickness,
                     float                   fade,
                     int                     entityID,
                     int                     quadType)
{
    using namespace R2D;
    if (s.IndexCount >= kMaxIndices)
        Renderer2D::NextBatch(); // will call Flush() + StartBatch()

    for (int i = 0; i < 4; i++) {
        s.VertexPtr->Position     = glm::vec3(transform * positions[i]);
        s.VertexPtr->Color        = color;
        s.VertexPtr->TexCoord     = uvs[i];
        s.VertexPtr->TexIndex     = texIndex;
        s.VertexPtr->TilingFactor = tilingFactor;
        s.VertexPtr->LocalUV      = localUV ? localUV[i] : uvs[i];
        s.VertexPtr->Thickness    = thickness;
        s.VertexPtr->Fade         = fade;
        s.VertexPtr->EntityID     = entityID;
        s.VertexPtr->QuadType     = quadType;
        s.VertexPtr++;
    }
    s.IndexCount += 6;
}

// ─── DrawSprite ───────────────────────────────────────────────────────────────
void Renderer2D::DrawSprite(const glm::mat4& transform,
                             std::shared_ptr<Texture2D> texture,
                             const glm::vec4& tintColor,
                             float tilingFactor)
{
    using namespace R2D;
    float texIndex = 0.0f;
    if (texture) {
        uint32_t id = texture->GetID();
        // Find existing slot
        for (uint32_t i = 1; i < s.TextureSlotIndex; i++) {
            if (s.TextureIDs[i] == id) { texIndex = (float)i; break; }
        }
        if (texIndex == 0.0f) {
            if (s.TextureSlotIndex >= kMaxTexSlots) NextBatch();
            texIndex = (float)s.TextureSlotIndex;
            s.TextureIDs[s.TextureSlotIndex++] = id;
        }
    }
    PushQuad(transform, kQuadPositions, kQuadUV,
             tintColor, texIndex, tilingFactor,
             kQuadUV, 1.0f, 0.0f, -1, 0);
}

// ─── DrawRect (solid color) ──────────────────────────────────────────────────
void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color) {
    static const glm::vec2 uv[4] = {{0,0},{1,0},{1,1},{0,1}};
    PushQuad(transform, R2D::kQuadPositions, uv,
             color, 0.0f, 1.0f, uv, 1.0f, 0.0f, -1, 0);
}

// ─── DrawSubSprite ────────────────────────────────────────────────────────────
void Renderer2D::DrawSubSprite(const glm::mat4& transform,
                                std::shared_ptr<SpriteSheet> sheet,
                                const glm::vec2& coords,
                                const glm::vec4& tintColor)
{
    if (!sheet) return;
    auto uvs = sheet->GetUVs(coords);
    using namespace R2D;
    float texIndex = 0.0f;
    uint32_t id = sheet->GetTexture()->GetID();
    for (uint32_t i = 1; i < s.TextureSlotIndex; i++)
        if (s.TextureIDs[i] == id) { texIndex = (float)i; break; }
    if (texIndex == 0.0f) {
        if (s.TextureSlotIndex >= kMaxTexSlots) NextBatch();
        texIndex = (float)s.TextureSlotIndex;
        s.TextureIDs[s.TextureSlotIndex++] = id;
    }
    PushQuad(transform, kQuadPositions, uvs.data(),
             tintColor, texIndex, 1.0f, uvs.data(), 1.0f, 0.0f, -1, 0);
}

// ─── DrawCircle ──────────────────────────────────────────────────────────────
void Renderer2D::DrawCircle(const glm::mat4& transform,
                             const glm::vec4& color,
                             float thickness,
                             float fade)
{
    static const glm::vec2 uv[4] = {{0,0},{1,0},{1,1},{0,1}};
    PushQuad(transform, R2D::kQuadPositions, uv,
             color, 0.0f, 1.0f,
             R2D::kCircleLocalUV, thickness, fade, -1, 1);
}

// ─── DrawLine ────────────────────────────────────────────────────────────────
void Renderer2D::DrawLine(const glm::vec3& p0,
                           const glm::vec3& p1,
                           const glm::vec4& color,
                           float thickness)
{
    // Build a billboard quad aligned to the line
    glm::vec3 dir = glm::normalize(p1 - p0);
    glm::vec3 up  = glm::vec3(0.0f, 1.0f, 0.0f);
    // Use Z-forward if line is vertical
    if (std::abs(glm::dot(dir, up)) > 0.99f) up = glm::vec3(0, 0, 1);
    glm::vec3 right = glm::normalize(glm::cross(dir, up)) * (thickness * 0.5f);

    glm::vec4 positions[4] = {
        glm::vec4(p0 - right, 1.0f),
        glm::vec4(p1 - right, 1.0f),
        glm::vec4(p1 + right, 1.0f),
        glm::vec4(p0 + right, 1.0f),
    };
    static const glm::vec2 uv[4] = {{0,0},{1,0},{1,1},{0,1}};

    using namespace R2D;
    if (s.IndexCount >= kMaxIndices) Renderer2D::NextBatch();
    for (int i = 0; i < 4; i++) {
        s.VertexPtr->Position     = glm::vec3(positions[i]);
        s.VertexPtr->Color        = color;
        s.VertexPtr->TexCoord     = uv[i];
        s.VertexPtr->TexIndex     = 0.0f;
        s.VertexPtr->TilingFactor = 1.0f;
        s.VertexPtr->LocalUV      = uv[i];
        s.VertexPtr->Thickness    = 1.0f;
        s.VertexPtr->Fade         = 0.0f;
        s.VertexPtr->EntityID     = -1;
        s.VertexPtr->QuadType     = 0;
        s.VertexPtr++;
    }
    s.IndexCount += 6;
    s.Stats.LineCount++;
}

// ─── DrawRectOutline ─────────────────────────────────────────────────────────
void Renderer2D::DrawRectOutline(const glm::mat4& transform, const glm::vec4& color) {
    // Transform the four corners
    glm::vec3 corners[4];
    for (int i = 0; i < 4; i++)
        corners[i] = glm::vec3(transform * R2D::kQuadPositions[i]);
    float thick = 0.02f;
    DrawLine(corners[0], corners[1], color, thick);
    DrawLine(corners[1], corners[2], color, thick);
    DrawLine(corners[2], corners[3], color, thick);
    DrawLine(corners[3], corners[0], color, thick);
}

// ─── DrawString ──────────────────────────────────────────────────────────────
void Renderer2D::DrawString(const std::string& text,
                             std::shared_ptr<Font> font,
                             const glm::mat4&      transform,
                             const glm::vec4&      color,
                             float                 kerning,
                             float                 lineSpacing)
{
    if (!font || text.empty()) return;
    using namespace R2D;

    // Find or register font texture slot
    uint32_t fontTexID = font->GetAtlasTexture()->GetID();
    float texIndex = 0.0f;
    for (uint32_t i = 1; i < s.TextureSlotIndex; i++)
        if (s.TextureIDs[i] == fontTexID) { texIndex = (float)i; break; }
    if (texIndex == 0.0f) {
        if (s.TextureSlotIndex >= kMaxTexSlots) Renderer2D::Flush();
        texIndex = (float)s.TextureSlotIndex;
        s.TextureIDs[s.TextureSlotIndex++] = fontTexID;
    }

    glm::vec2 cursor{0.0f};
    float lineH = font->GetLineHeight() * lineSpacing;

    for (char c : text) {
        if (c == '\n') { cursor.x = 0; cursor.y -= lineH; continue; }
        if (c == '\r') continue;

        const GlyphMetrics* glyph = font->GetGlyph((uint32_t)c);
        if (!glyph) continue;

        if (s.IndexCount >= kMaxIndices) Renderer2D::NextBatch();

        // Build quad in local font space, then apply parent transform
        float x0 = cursor.x + glyph->BearingX;
        float y0 = cursor.y + glyph->BearingY - glyph->SizeY;
        float x1 = x0 + glyph->SizeX;
        float y1 = y0 + glyph->SizeY;

        glm::vec4 pos[4] = {
            {x0, y0, 0, 1}, {x1, y0, 0, 1},
            {x1, y1, 0, 1}, {x0, y1, 0, 1}
        };
        glm::vec2 uv[4] = {
            {glyph->U0, glyph->V0}, {glyph->U1, glyph->V0},
            {glyph->U1, glyph->V1}, {glyph->U0, glyph->V1}
        };

        for (int i = 0; i < 4; i++) {
            s.VertexPtr->Position     = glm::vec3(transform * pos[i]);
            s.VertexPtr->Color        = color;
            s.VertexPtr->TexCoord     = uv[i];
            s.VertexPtr->TexIndex     = texIndex;
            s.VertexPtr->TilingFactor = 1.0f;
            s.VertexPtr->LocalUV      = uv[i];
            s.VertexPtr->Thickness    = 0.01f; // SDF smoothing
            s.VertexPtr->Fade         = 0.0f;
            s.VertexPtr->EntityID     = -1;
            s.VertexPtr->QuadType     = 2;
            s.VertexPtr++;
        }
        s.IndexCount += 6;

        cursor.x += glyph->Advance + kerning;
    }
}

void Renderer2D::ResetStats() { R2D::s.Stats = {}; }
const RenderStats2D& Renderer2D::GetStats() { return R2D::s.Stats; }

} // namespace Freely
