#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace Freely {

class Shader;
class Texture2D;

struct Material {
    std::shared_ptr<Shader> ShaderProgram;
    std::shared_ptr<Texture2D> AlbedoMap;
    std::shared_ptr<Texture2D> NormalMap;
    std::shared_ptr<Texture2D> MetallicRoughnessMap;

    glm::vec3 Albedo{1.0f, 1.0f, 1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float AO = 1.0f;

    void Bind() const;
    void Unbind() const;

    static std::shared_ptr<Material> Create(std::shared_ptr<Shader> shader);
};

} // namespace Freely
