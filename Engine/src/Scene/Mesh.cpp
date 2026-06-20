#include "Freely/Scene/Mesh.h"
#include "Freely/Renderer/VertexArray.h"
#include "Freely/Renderer/Buffer.h"
#include "Freely/Core/Logger.h"

#include <tiny_obj_loader.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Freely {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : m_IndexCount(static_cast<uint32_t>(indices.size()))
{
    SetupMesh(vertices, indices);
}

void Mesh::SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    m_VAO = std::make_shared<VertexArray>();

    auto vbo = std::make_shared<VertexBuffer>(
        reinterpret_cast<const float*>(vertices.data()),
        static_cast<uint32_t>(vertices.size() * sizeof(Vertex))
    );

    vbo->SetLayout({
        {ShaderDataType::Float3, "a_Position"},
        {ShaderDataType::Float3, "a_Normal"},
        {ShaderDataType::Float2, "a_TexCoords"},
        {ShaderDataType::Float3, "a_Tangent"}
    });

    m_VAO->AddVertexBuffer(vbo);

    auto ibo = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
    m_VAO->SetIndexBuffer(ibo);

    m_VBO = vbo;
    m_IBO = ibo;
}

void Mesh::Draw() const {
    m_VAO->Bind();
    // Drawing is handled by the renderer
}

std::shared_ptr<Mesh> Mesh::CreateCube() {
    std::vector<Vertex> vertices = {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0}, {1, 0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1, 0}, {1, 0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1, 1}, {1, 0, 0}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 1}, {1, 0, 0}},
        // Back face
        {{ 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}, {-1, 0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}, {-1, 0, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1, 1}, {-1, 0, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 0, -1}, {0, 1}, {-1, 0, 0}},
        // Top face
        {{-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {0, 0}, {1, 0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1, 0}, {1, 0, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1, 1}, {1, 0, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {0, 1}, {1, 0, 0}},
        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0}, {1, 0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0}, {1, 0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {0, -1, 0}, {1, 1}, {1, 0, 0}},
        {{-0.5f, -0.5f,  0.5f}, {0, -1, 0}, {0, 1}, {1, 0, 0}},
        // Right face
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0, 0}, {0, 0, -1}},
        {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}, {0, 0, -1}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {1, 1}, {0, 0, -1}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}, {0, 1}, {0, 0, -1}},
        // Left face
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}, {0, 0, 1}},
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 0}, {0, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 1}, {0, 0, 1}},
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 1}, {0, 0, 1}},
    };

    std::vector<uint32_t> indices = {
        0,1,2, 2,3,0,       // front
        4,5,6, 6,7,4,       // back
        8,9,10, 10,11,8,    // top
        12,13,14, 14,15,12, // bottom
        16,17,18, 18,19,16, // right
        20,21,22, 22,23,20  // left
    };

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::CreateSphere(int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int y = 0; y <= rings; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(rings);

            float xPos = std::cos(xSegment * 2.0f * static_cast<float>(M_PI)) * std::sin(ySegment * static_cast<float>(M_PI));
            float yPos = std::cos(ySegment * static_cast<float>(M_PI));
            float zPos = std::sin(xSegment * 2.0f * static_cast<float>(M_PI)) * std::sin(ySegment * static_cast<float>(M_PI));

            Vertex v;
            v.Position = {xPos, yPos, zPos};
            v.Normal = {xPos, yPos, zPos};
            v.TexCoords = {xSegment, ySegment};
            v.Tangent = glm::normalize(glm::vec3(-std::sin(xSegment * 2.0f * static_cast<float>(M_PI)), 0.0f, std::cos(xSegment * 2.0f * static_cast<float>(M_PI))));
            vertices.push_back(v);
        }
    }

    for (int y = 0; y < rings; y++) {
        for (int x = 0; x < segments; x++) {
            uint32_t current = y * (segments + 1) + x;
            uint32_t next = current + segments + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::CreatePlane(float size) {
    float half = size * 0.5f;
    std::vector<Vertex> vertices = {
        {{-half, 0.0f, -half}, {0, 1, 0}, {0, 0}, {1, 0, 0}},
        {{ half, 0.0f, -half}, {0, 1, 0}, {1, 0}, {1, 0, 0}},
        {{ half, 0.0f,  half}, {0, 1, 0}, {1, 1}, {1, 0, 0}},
        {{-half, 0.0f,  half}, {0, 1, 0}, {0, 1}, {1, 0, 0}},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::LoadFromFile(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        FL_ENGINE_ERROR("Failed to load OBJ: {} {}", warn, err);
        return nullptr;
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.Position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0) {
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.TexCoords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            indices.push_back(static_cast<uint32_t>(vertices.size()));
            vertices.push_back(vertex);
        }
    }

    FL_ENGINE_INFO("Loaded mesh: {} ({} vertices, {} indices)", path, vertices.size(), indices.size());
    return std::make_shared<Mesh>(vertices, indices);
}

} // namespace Freely
