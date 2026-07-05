#pragma once
// Freely Engine - SpriteSheet
// Wraps a texture atlas and exposes UV lookup by tile coordinate.

#include <memory>
#include <array>
#include <glm/glm.hpp>

namespace Freely {

class Texture2D;

class SpriteSheet {
public:
    /// path     : texture file
    /// cellSize : pixel dimensions of a single tile (e.g. {16,16})
    /// spacing  : pixel gap between tiles (default 0)
    SpriteSheet(const std::string& path,
                const glm::vec2&   cellSize,
                const glm::vec2&   spacing = {0,0});
    SpriteSheet(std::shared_ptr<Texture2D> texture,
                const glm::vec2&           cellSize,
                const glm::vec2&           spacing = {0,0});

    /// Returns the 4 UV corners for tile at (col, row), in VAO order:
    ///   [0]=BL [1]=BR [2]=TR [3]=TL
    std::array<glm::vec2, 4> GetUVs(const glm::vec2& coords) const;

    std::shared_ptr<Texture2D> GetTexture() const { return m_Texture; }
    const glm::vec2& GetCellSize() const { return m_CellSize; }

    static std::shared_ptr<SpriteSheet> Create(const std::string& path,
                                                const glm::vec2&   cellSize,
                                                const glm::vec2&   spacing = {0,0});

private:
    std::shared_ptr<Texture2D> m_Texture;
    glm::vec2 m_CellSize;
    glm::vec2 m_Spacing;
};

} // namespace Freely
