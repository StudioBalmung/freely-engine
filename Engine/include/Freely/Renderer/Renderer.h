#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Freely {

class Shader;
class VertexArray;
class Camera;

enum class ClearFlags : uint32_t {
    Color = 0x01,
    Depth = 0x02,
    Stencil = 0x04,
    All = Color | Depth | Stencil
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    void Init();
    void Shutdown();

    void SetViewport(int x, int y, int width, int height);
    void SetClearColor(const glm::vec4& color);
    void Clear(ClearFlags flags = ClearFlags::All);

    void BeginFrame(const Camera& camera);
    void EndFrame();

    void DrawArrays(const VertexArray& vao, uint32_t vertexCount);
    void DrawIndexed(const VertexArray& vao, uint32_t indexCount);

    void SetDepthTest(bool enabled);
    void SetBlending(bool enabled);
    void SetWireframe(bool enabled);
    void SetCullFace(bool enabled, bool backFace = true);

    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

private:
    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_ProjectionMatrix{1.0f};
    glm::vec4 m_ClearColor{0.1f, 0.1f, 0.12f, 1.0f};
};

} // namespace Freely
