#include "OpenGLResources.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferDesc& desc)
    : m_Desc(desc)
{
    Build();
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
    Destroy();
}

void OpenGLFramebuffer::Build() {
    glGenFramebuffers(1, &m_Handle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);

    m_Color.clear();
    m_Color.reserve(m_Desc.ColorAttachments.size());

    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(m_Desc.ColorAttachments.size());

    for (size_t i = 0; i < m_Desc.ColorAttachments.size(); ++i) {
        TextureDesc td{};
        td.Width  = m_Desc.Width;
        td.Height = m_Desc.Height;
        td.Format = m_Desc.ColorAttachments[i].Format;
        td.IsRenderTarget = true;
        td.MinFilter = TextureFilter::Linear;
        td.MagFilter = TextureFilter::Linear;
        td.WrapU = td.WrapV = TextureWrap::ClampToEdge;

        auto tex = std::make_shared<OpenGLTexture>(td);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                               GL_TEXTURE_2D, tex->Handle(), 0);
        m_Color.push_back(tex);
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i));
    }

    if (!drawBuffers.empty()) {
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (m_Desc.HasDepth) {
        TextureDesc td{};
        td.Width  = m_Desc.Width;
        td.Height = m_Desc.Height;
        td.Format = m_Desc.DepthAttachment.Format != TextureFormat::Unknown
                  ? m_Desc.DepthAttachment.Format
                  : TextureFormat::Depth24Stencil8;
        td.IsRenderTarget = true;
        td.MinFilter = TextureFilter::Nearest;
        td.MagFilter = TextureFilter::Nearest;
        td.WrapU = td.WrapV = TextureWrap::ClampToEdge;

        m_Depth = std::make_shared<OpenGLTexture>(td);
        GLenum attachment = (td.Format == TextureFormat::Depth24Stencil8)
                          ? GL_DEPTH_STENCIL_ATTACHMENT
                          : GL_DEPTH_ATTACHMENT;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_Depth->Handle(), 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        FL_ENGINE_ERROR("Framebuffer '{}' is incomplete!", m_Desc.DebugName);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::Destroy() {
    if (m_Handle) {
        glDeleteFramebuffers(1, &m_Handle);
        m_Handle = 0;
    }
    m_Color.clear();
    m_Depth.reset();
}

void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;
    if (width == m_Desc.Width && height == m_Desc.Height) return;
    m_Desc.Width = width;
    m_Desc.Height = height;
    Destroy();
    Build();
}

std::shared_ptr<ITexture> OpenGLFramebuffer::GetColorAttachment(uint32_t index) const {
    if (index >= m_Color.size()) return nullptr;
    return m_Color[index];
}

} // namespace Freely::RHI
