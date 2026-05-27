#include "Freely/Renderer/Renderer.h"
#include "Freely/Renderer/VertexArray.h"
#include "Freely/Renderer/Shader.h"
#include "Freely/Scene/Camera.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>

namespace Freely {

Renderer::Renderer() {}
Renderer::~Renderer() {}

void Renderer::Init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    FL_ENGINE_INFO("Renderer initialized.");
}

void Renderer::Shutdown() {
    FL_ENGINE_INFO("Renderer shutting down.");
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4& color) {
    m_ClearColor = color;
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear(ClearFlags flags) {
    GLbitfield mask = 0;
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(ClearFlags::Color))
        mask |= GL_COLOR_BUFFER_BIT;
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(ClearFlags::Depth))
        mask |= GL_DEPTH_BUFFER_BIT;
    if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(ClearFlags::Stencil))
        mask |= GL_STENCIL_BUFFER_BIT;
    glClear(mask);
}

void Renderer::BeginFrame(const Camera& camera) {
    m_ViewMatrix = camera.GetViewMatrix();
    m_ProjectionMatrix = camera.GetProjectionMatrix();
    SetClearColor(m_ClearColor);
    Clear();
}

void Renderer::EndFrame() {
    // Post-processing or frame finalization goes here
}

void Renderer::DrawArrays(const VertexArray& vao, uint32_t vertexCount) {
    vao.Bind();
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Renderer::DrawIndexed(const VertexArray& vao, uint32_t indexCount) {
    vao.Bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void Renderer::SetDepthTest(bool enabled) {
    if (enabled) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
}

void Renderer::SetBlending(bool enabled) {
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

void Renderer::SetWireframe(bool enabled) {
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

void Renderer::SetCullFace(bool enabled, bool backFace) {
    if (enabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(backFace ? GL_BACK : GL_FRONT);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

} // namespace Freely
