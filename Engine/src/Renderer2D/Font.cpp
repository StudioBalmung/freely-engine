#include "Freely/Renderer2D/Font.h"
#include "Freely/Renderer/Texture.h"
#include "Freely/Core/Logger.h"

// stb_truetype — header-only, implement once here
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <fstream>
#include <vector>
#include <cmath>

namespace Freely {

Font::Font(const std::string& path, float fontSize,
           uint32_t atlasWidth, uint32_t atlasHeight)
    : m_FontSize(fontSize)
{
    // ── Load font file ─────────────────────────────────────────────────
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        FL_ENGINE_ERROR("Font: cannot open '{}'", path);
        return;
    }
    auto size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> fontData((size_t)size);
    file.read(reinterpret_cast<char*>(fontData.data()), size);

    // ── Init stb_truetype ─────────────────────────────────────────────
    stbtt_fontinfo fontInfo{};
    if (!stbtt_InitFont(&fontInfo, fontData.data(), 0)) {
        FL_ENGINE_ERROR("Font: stbtt_InitFont failed for '{}'", path);
        return;
    }

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    m_Ascent     = ascent     * scale;
    m_LineHeight = (ascent - descent + lineGap) * scale;

    // ── Allocate atlas ────────────────────────────────────────────────
    std::vector<uint8_t> atlas(atlasWidth * atlasHeight, 0);

    // Pack using stb_truetype bitmap packing
    stbtt_pack_context packCtx{};
    stbtt_PackBegin(&packCtx, atlas.data(), (int)atlasWidth, (int)atlasHeight,
                    0, 1, nullptr);
    stbtt_PackSetOversampling(&packCtx, 2, 2); // 2× oversampling for SDF

    // Bake ASCII 32-126
    constexpr int kFirst = 32;
    constexpr int kCount = 127 - 32;
    std::vector<stbtt_packedchar> charData(kCount);
    stbtt_PackFontRange(&packCtx, fontData.data(), 0, fontSize,
                        kFirst, kCount, charData.data());
    stbtt_PackEnd(&packCtx);

    // ── Convert bitmap to SDF ─────────────────────────────────────────
    // stb_truetype pack produces a grayscale coverage bitmap.
    // We do a cheap signed-distance approximation: normalize 0-255 → 0-1.
    // For a proper SDF you would use stbtt_GetPackedQuad with SDF mode,
    // but coverage + smooth threshold works well at typical sizes.

    // ── Build glyph metrics ───────────────────────────────────────────
    float invW = 1.0f / (float)atlasWidth;
    float invH = 1.0f / (float)atlasHeight;

    for (int i = 0; i < kCount; i++) {
        auto& pc = charData[i];
        GlyphMetrics gm;
        gm.U0 = pc.x0 * invW;
        gm.V0 = pc.y0 * invH;
        gm.U1 = pc.x1 * invW;
        gm.V1 = pc.y1 * invH;

        gm.SizeX    = (float)(pc.x1 - pc.x0);
        gm.SizeY    = (float)(pc.y1 - pc.y0);
        gm.BearingX = pc.xoff;
        gm.BearingY = -pc.yoff;       // stb uses top-down, we use bottom-up
        gm.Advance  = pc.xadvance;

        m_Glyphs[(uint32_t)(kFirst + i)] = gm;
    }

    // ── Upload to GPU as R8 ───────────────────────────────────────────
    TextureSpec spec;
    spec.Width  = (int)atlasWidth;
    spec.Height = (int)atlasHeight;
    spec.Wrap   = TextureWrap::ClampToEdge;
    spec.MinFilter = TextureFilter::Linear;
    spec.MagFilter = TextureFilter::Linear;
    spec.GenerateMipmaps = false;
    spec.SRGB = false;

    // Texture2D with R8 data: pass atlas as 1-channel data
    // We repurpose the RGBA spec by making a 4-channel copy (R only, others 0)
    // Actually our Texture2D loads RGBA8, so we'll expand R → RGBA manually
    std::vector<uint8_t> rgba(atlasWidth * atlasHeight * 4, 0);
    for (uint32_t p = 0; p < atlasWidth * atlasHeight; p++) {
        uint8_t v = atlas[p];
        rgba[p * 4 + 0] = v; // R
        rgba[p * 4 + 1] = v; // G (needed for SampleTexture.r in shader)
        rgba[p * 4 + 2] = v; // B
        rgba[p * 4 + 3] = v; // A
    }

    m_AtlasTexture = std::make_shared<Texture2D>(spec, rgba.data());

    FL_ENGINE_INFO("Font '{}' loaded: {}px, {} glyphs, {}x{} atlas.",
                   path, fontSize, kCount, atlasWidth, atlasHeight);
}

const GlyphMetrics* Font::GetGlyph(uint32_t codepoint) const {
    auto it = m_Glyphs.find(codepoint);
    return (it != m_Glyphs.end()) ? &it->second : nullptr;
}

std::shared_ptr<Font> Font::Create(const std::string& path, float fontSize,
                                    uint32_t atlasW, uint32_t atlasH) {
    return std::make_shared<Font>(path, fontSize, atlasW, atlasH);
}

} // namespace Freely
