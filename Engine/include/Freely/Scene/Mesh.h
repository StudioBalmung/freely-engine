#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Freely {

class VertexArray;
class VertexBuffer;
class IndexBuffer;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh() = default;

    void Draw() const;

    const std::shared_ptr<VertexArray>& GetVAO() const { return m_VAO; }
    uint32_t GetIndexCount() const { return m_IndexCount; }

    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreateSphere(int segments = 32, int rings = 16);
    static std::shared_ptr<Mesh> CreatePlane(float size = 10.0f);
    static std::shared_ptr<Mesh> LoadFromFile(const std::string& path);

private:
    void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VBO;
    std::shared_ptr<IndexBuffer> m_IBO;
    uint32_t m_IndexCount = 0;
};

} // namespace Freely
