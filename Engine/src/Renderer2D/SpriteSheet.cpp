#include "Freely/Renderer2D/SpriteSheet.h"
#include "Freely/Renderer/Texture.h"

namespace Freely {

SpriteSheet::SpriteSheet(const std::string& path,
                          const glm::vec2&   cellSize,
                          const glm::vec2&   spacing)
    : m_CellSize(cellSize), m_Spacing(spacing)
{
    m_Texture = std::make_shared<Texture2D>(path);
}

SpriteSheet::SpriteSheet(std::shared_ptr<Texture2D> texture,
                          const glm::vec2&           cellSize,
                          const glm::vec2&           spacing)
    : m_Texture(texture), m_CellSize(cellSize), m_Spacing(spacing)
{}

std::array<glm::vec2, 4> SpriteSheet::GetUVs(const glm::vec2& coords) const {
    float texW = (float)m_Texture->GetWidth();
    float texH = (float)m_Texture->GetHeight();

    float x0 = (coords.x * (m_CellSize.x + m_Spacing.x)) / texW;
    float y0 = (coords.y * (m_CellSize.y + m_Spacing.y)) / texH;
    float x1 = x0 + m_CellSize.x / texW;
    float y1 = y0 + m_CellSize.y / texH;

    // BL, BR, TR, TL  (flipped Y so row 0 = top of atlas)
    return {{
        {x0, 1.0f - y1},
        {x1, 1.0f - y1},
        {x1, 1.0f - y0},
        {x0, 1.0f - y0},
    }};
}

std::shared_ptr<SpriteSheet> SpriteSheet::Create(const std::string& path,
                                                   const glm::vec2&   cellSize,
                                                   const glm::vec2&   spacing) {
    return std::make_shared<SpriteSheet>(path, cellSize, spacing);
}

} // namespace Freely
