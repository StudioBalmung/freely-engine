#include "OpenGLResources.h"

namespace Freely::RHI {

OpenGLTexture::OpenGLTexture(const TextureDesc& desc)
    : m_Desc(desc)
{
    glGenTextures(1, &m_Handle);
    glBindTexture(GL_TEXTURE_2D, m_Handle);

    GLenum internalFmt = GLTextureFormat(desc.Format, true);
    GLenum dataFmt     = GLTextureFormat(desc.Format, false);
    GLenum dataType    = GLTextureType(desc.Format);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFmt,
                 static_cast<GLsizei>(desc.Width),
                 static_cast<GLsizei>(desc.Height), 0,
                 dataFmt, dataType, desc.InitialData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GLTextureFilter(desc.MinFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GLTextureFilter(desc.MagFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLTextureWrap(desc.WrapU));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLTextureWrap(desc.WrapV));

    if (desc.GenerateMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

OpenGLTexture::~OpenGLTexture() {
    if (m_Handle) glDeleteTextures(1, &m_Handle);
}

} // namespace Freely::RHI
