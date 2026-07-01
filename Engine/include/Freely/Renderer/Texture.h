#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace Freely {

enum class TextureWrap {
    Repeat,
    ClampToEdge,
    MirroredRepeat
};

enum class TextureFilter {
    Nearest,
    Linear,
    LinearMipmapLinear
};

struct TextureSpec {
    int Width = 1;
    int Height = 1;
    TextureWrap Wrap = TextureWrap::Repeat;
    TextureFilter MinFilter = TextureFilter::LinearMipmapLinear;
    TextureFilter MagFilter = TextureFilter::Linear;
    bool GenerateMipmaps = true;
    bool SRGB = true;
};

class Texture2D {
public:
    Texture2D(const std::string& path);
    Texture2D(const TextureSpec& spec, const void* data = nullptr);
    ~Texture2D();

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    uint32_t GetID() const { return m_ID; }

    void SetData(const void* data, uint32_t size);

    static std::shared_ptr<Texture2D> Create(const std::string& path);

private:
    uint32_t m_ID = 0;
    int m_Width = 0;
    int m_Height = 0;
    int m_Channels = 0;
    uint32_t m_InternalFormat = 0;
    uint32_t m_DataFormat = 0;
};

} // namespace Freely
