#include "Freely/Renderer/Texture.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Freely {

static GLenum WrapToGL(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        default: return GL_REPEAT;
    }
}

static GLenum FilterToGL(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_LINEAR;
    }
}

Texture2D::Texture2D(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);

    if (!data) {
        FL_ENGINE_ERROR("Failed to load texture: {}", path);
        return;
    }

    if (m_Channels == 4) {
        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;
    } else if (m_Channels == 3) {
        m_InternalFormat = GL_RGB8;
        m_DataFormat = GL_RGB;
    } else if (m_Channels == 1) {
        m_InternalFormat = GL_R8;
        m_DataFormat = GL_RED;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
    glTextureStorage2D(m_ID, 1, m_InternalFormat, m_Width, m_Height);

    glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(m_ID);

    stbi_image_free(data);
    FL_ENGINE_INFO("Loaded texture: {} ({}x{}, {} channels)", path, m_Width, m_Height, m_Channels);
}

Texture2D::Texture2D(const TextureSpec& spec, const void* data) {
    m_Width = spec.Width;
    m_Height = spec.Height;
    m_InternalFormat = GL_RGBA8;
    m_DataFormat = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
    glTextureStorage2D(m_ID, 1, m_InternalFormat, m_Width, m_Height);

    glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, FilterToGL(spec.MinFilter));
    glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, FilterToGL(spec.MagFilter));
    glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, WrapToGL(spec.Wrap));
    glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, WrapToGL(spec.Wrap));

    if (data) {
        glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    }
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &m_ID);
}

void Texture2D::Bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_ID);
}

void Texture2D::Unbind() const {
    glBindTextureUnit(0, 0);
}

void Texture2D::SetData(const void* data, uint32_t size) {
    glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path) {
    return std::make_shared<Texture2D>(path);
}

} // namespace Freely
