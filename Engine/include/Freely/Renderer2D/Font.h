#pragma once
// Freely Engine 0.4.2 — Font
// Bakes a TrueType / OpenType font into an SDF glyph atlas at load-time
// using stb_truetype.  The atlas is a single R8 texture; the fragment shader
// thresholds the signed-distance field for crisp, resolution-independent text.

#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace Freely {

class Texture2D;

struct GlyphMetrics {
    // Atlas UV corners
    float U0, V0, U1, V1;
    // Layout metrics in font units (scaled to fontSize pixels)
    float SizeX, SizeY;
    float BearingX, BearingY;   // offset from cursor to glyph origin
    float Advance;               // cursor advance after this glyph
};

class Font {
public:
    /// Load a .ttf / .otf file and bake glyphs at the given pixel size.
    /// Bakes ASCII 32-126 by default.
    explicit Font(const std::string& path, float fontSize = 32.0f,
                  uint32_t atlasWidth = 1024, uint32_t atlasHeight = 1024);
    ~Font() = default;

    /// Returns nullptr if the glyph was not baked.
    const GlyphMetrics* GetGlyph(uint32_t codepoint) const;

    float GetLineHeight()  const { return m_LineHeight; }
    float GetAscent()      const { return m_Ascent; }
    float GetFontSize()    const { return m_FontSize; }

    std::shared_ptr<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

    static std::shared_ptr<Font> Create(const std::string& path,
                                        float fontSize  = 32.0f,
                                        uint32_t atlasW = 1024,
                                        uint32_t atlasH = 1024);

private:
    std::unordered_map<uint32_t, GlyphMetrics> m_Glyphs;
    std::shared_ptr<Texture2D> m_AtlasTexture;
    float m_LineHeight = 0.0f;
    float m_Ascent     = 0.0f;
    float m_FontSize   = 32.0f;
};

} // namespace Freely
