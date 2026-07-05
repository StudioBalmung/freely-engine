#include "OpenGLDevice.h"
#include "OpenGLResources.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Freely::RHI {

std::unique_ptr<IRenderDevice> CreateOpenGLDevice() {
    return std::make_unique<OpenGLDevice>();
}

OpenGLDevice::OpenGLDevice() = default;
OpenGLDevice::~OpenGLDevice() { Shutdown(); }

bool OpenGLDevice::Initialize(GLFWwindow* window) {
    m_Window = window;
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        FL_ENGINE_CRITICAL("Failed to initialize GLAD (OpenGL loader).");
        return false;
    }

    // Query capabilities.
    const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    m_Caps.VendorName    = vendor   ? vendor   : "Unknown";
    m_Caps.DeviceName    = renderer ? renderer : "Unknown";
    m_Caps.DriverVersion = version  ? version  : "Unknown";

    GLint v = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &v);          m_Caps.MaxTextureSize = static_cast<uint32_t>(v);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &v);     m_Caps.MaxColorAttachments = static_cast<uint32_t>(v);
    GLfloat aniso = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
    m_Caps.MaxAnisotropy = static_cast<uint32_t>(aniso);
    m_Caps.SupportsCompute = true;

    FL_ENGINE_INFO("OpenGL Device: {}", m_Caps.DeviceName);
    FL_ENGINE_INFO("  Vendor:  {}", m_Caps.VendorName);
    FL_ENGINE_INFO("  Version: {}", m_Caps.DriverVersion);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_Initialized = true;
    return true;
}

void OpenGLDevice::Shutdown() {
    if (!m_Initialized) return;
    if (m_VAO) { glDeleteVertexArrays(1, &m_VAO); m_VAO = 0; }
    m_Initialized = false;
}

void OpenGLDevice::BeginFrame() {}

void OpenGLDevice::EndFrame() {}

void OpenGLDevice::Present() {
    if (m_Window) glfwSwapBuffers(m_Window);
}

void OpenGLDevice::WaitIdle() {
    glFinish();
}

void OpenGLDevice::Resize(uint32_t width, uint32_t height) {
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void OpenGLDevice::SetViewport(const Viewport& vp) {
    glViewport(static_cast<GLint>(vp.X), static_cast<GLint>(vp.Y),
               static_cast<GLsizei>(vp.Width), static_cast<GLsizei>(vp.Height));
    glDepthRange(vp.MinDepth, vp.MaxDepth);
}

void OpenGLDevice::SetScissor(const Rect2D& rect) {
    glScissor(rect.X, rect.Y, static_cast<GLsizei>(rect.Width), static_cast<GLsizei>(rect.Height));
    glEnable(GL_SCISSOR_TEST);
}

void OpenGLDevice::SetClearColor(float r, float g, float b, float a) {
    m_ClearColor[0] = r; m_ClearColor[1] = g; m_ClearColor[2] = b; m_ClearColor[3] = a;
    glClearColor(r, g, b, a);
}

void OpenGLDevice::Clear(bool color, bool depth, bool stencil) {
    GLbitfield mask = 0;
    if (color)   mask |= GL_COLOR_BUFFER_BIT;
    if (depth)   mask |= GL_DEPTH_BUFFER_BIT;
    if (stencil) mask |= GL_STENCIL_BUFFER_BIT;
    glClear(mask);
}

std::shared_ptr<IBuffer> OpenGLDevice::CreateBuffer(const BufferDesc& desc) {
    return std::make_shared<OpenGLBuffer>(desc);
}

std::shared_ptr<ITexture> OpenGLDevice::CreateTexture(const TextureDesc& desc) {
    return std::make_shared<OpenGLTexture>(desc);
}

std::shared_ptr<IShader> OpenGLDevice::CreateShader(const ShaderDesc& desc) {
    return std::make_shared<OpenGLShader>(desc);
}

std::shared_ptr<IPipeline> OpenGLDevice::CreatePipeline(const PipelineDesc& desc) {
    return std::make_shared<OpenGLPipeline>(desc);
}

std::shared_ptr<IFramebuffer> OpenGLDevice::CreateFramebuffer(const FramebufferDesc& desc) {
    return std::make_shared<OpenGLFramebuffer>(desc);
}

void OpenGLDevice::BindPipeline(const std::shared_ptr<IPipeline>& pipeline) {
    auto* p = static_cast<OpenGLPipeline*>(pipeline.get());
    if (!p) return;
    m_BoundProgram = p->Program();
    glUseProgram(m_BoundProgram);

    const auto& d = p->Desc();

    // Depth
    if (d.DepthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glDepthMask(d.DepthWrite ? GL_TRUE : GL_FALSE);
    glDepthFunc(GLCompareOp(d.DepthCompare));

    // Cull
    if (d.Culling == CullMode::None) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GLCullMode(d.Culling));
    }

    // Blend
    if (d.BlendEnable) {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GLBlendFactor(d.SrcColor), GLBlendFactor(d.DstColor),
                            GLBlendFactor(d.SrcAlpha), GLBlendFactor(d.DstAlpha));
    } else {
        glDisable(GL_BLEND);
    }

    glPolygonMode(GL_FRONT_AND_BACK, d.Wireframe ? GL_LINE : GL_FILL);
}

void OpenGLDevice::BindVertexBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t /*binding*/) {
    auto* b = static_cast<OpenGLBuffer*>(buffer.get());
    if (!b) return;
    glBindBuffer(GL_ARRAY_BUFFER, b->Handle());
    // NOTE: caller is expected to configure vertex attribute layout via shader / VAO.
}

void OpenGLDevice::BindIndexBuffer(const std::shared_ptr<IBuffer>& buffer, IndexType type) {
    auto* b = static_cast<OpenGLBuffer*>(buffer.get());
    if (!b) return;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->Handle());
    m_IndexType = type;
}

void OpenGLDevice::BindTexture(const std::shared_ptr<ITexture>& texture, uint32_t slot) {
    auto* t = static_cast<OpenGLTexture*>(texture.get());
    if (!t) return;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, t->Handle());
}

void OpenGLDevice::BindUniformBuffer(const std::shared_ptr<IBuffer>& buffer, uint32_t binding) {
    auto* b = static_cast<OpenGLBuffer*>(buffer.get());
    if (!b) return;
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, b->Handle());
}

void OpenGLDevice::BindFramebuffer(const std::shared_ptr<IFramebuffer>& framebuffer) {
    auto* fb = static_cast<OpenGLFramebuffer*>(framebuffer.get());
    if (!fb) return;
    glBindFramebuffer(GL_FRAMEBUFFER, fb->Handle());
    glViewport(0, 0, static_cast<GLsizei>(fb->GetWidth()), static_cast<GLsizei>(fb->GetHeight()));
}

void OpenGLDevice::UnbindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLDevice::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex) {
    if (instanceCount <= 1) {
        glDrawArrays(GL_TRIANGLES, static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount));
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, static_cast<GLint>(firstVertex),
                              static_cast<GLsizei>(vertexCount), static_cast<GLsizei>(instanceCount));
    }
}

void OpenGLDevice::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) {
    GLenum t = GLIndexType(m_IndexType);
    const size_t stride = (m_IndexType == IndexType::UInt16) ? 2 : 4;
    const void* offset = reinterpret_cast<const void*>(static_cast<uintptr_t>(firstIndex * stride));
    if (instanceCount <= 1) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), t, offset);
    } else {
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indexCount), t, offset,
                                static_cast<GLsizei>(instanceCount));
    }
}

void OpenGLDevice::Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ) {
    glDispatchCompute(groupX, groupY, groupZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

} // namespace Freely::RHI
