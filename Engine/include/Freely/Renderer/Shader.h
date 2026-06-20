#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Freely {

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    Shader(const std::string& vertexPath, const std::string& fragmentPath, bool fromFile);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

    uint32_t GetID() const { return m_ID; }

    static std::shared_ptr<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
    static std::shared_ptr<Shader> CreateFromFile(const std::string& vertexPath, const std::string& fragmentPath);

private:
    uint32_t CompileShader(uint32_t type, const std::string& source);
    int GetUniformLocation(const std::string& name);
    static std::string ReadFile(const std::string& path);

    uint32_t m_ID = 0;
    std::unordered_map<std::string, int> m_UniformCache;
};

} // namespace Freely
