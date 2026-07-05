#include "Freely/Renderer/Shader.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

namespace Freely {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
    glLinkProgram(m_ID);

    int success;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
        FL_ENGINE_ERROR("Shader linking failed: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, bool fromFile) {
    std::string vertexSource = ReadFile(vertexPath);
    std::string fragmentSource = ReadFile(fragmentPath);

    uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
    glLinkProgram(m_ID);

    int success;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
        FL_ENGINE_ERROR("Shader linking failed: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

void Shader::Bind() const {
    glUseProgram(m_ID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::SetInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

uint32_t Shader::CompileShader(uint32_t type, const std::string& source) {
    uint32_t shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        FL_ENGINE_ERROR("{} shader compilation failed: {}", typeStr, infoLog);
    }

    return shader;
}

int Shader::GetUniformLocation(const std::string& name) {
    if (m_UniformCache.find(name) != m_UniformCache.end())
        return m_UniformCache[name];

    int location = glGetUniformLocation(m_ID, name.c_str());
    if (location == -1) {
        FL_ENGINE_WARN("Uniform '{}' not found in shader.", name);
    }
    m_UniformCache[name] = location;
    return location;
}

std::string Shader::ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        FL_ENGINE_ERROR("Failed to open shader file: {}", path);
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::shared_ptr<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc) {
    return std::make_shared<Shader>(vertexSrc, fragmentSrc);
}

std::shared_ptr<Shader> Shader::CreateFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    return std::make_shared<Shader>(vertexPath, fragmentPath, true);
}

} // namespace Freely
