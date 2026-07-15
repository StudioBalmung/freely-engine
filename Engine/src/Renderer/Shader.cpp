#include "Freely/Renderer/Shader.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif

namespace Freely {

namespace fs = std::filesystem;

struct Shader::ShaderFileInfo {
    fs::path VertexPath;
    fs::path FragmentPath;
    fs::file_time_type LastVertexTime;
    fs::file_time_type LastFragmentTime;
};

#ifdef FREELY_ENABLE_SHADER_HOTRELOAD
static std::vector<std::weak_ptr<Shader>> s_ActiveShaders;
#endif

static fs::path ResolveShaderPath(const std::string& path) {
    if (fs::path(path).is_absolute()) return path;

    fs::path p = fs::path(path);
    std::string filename = p.filename().string();
    std::string relPath = p.relative_path().string();

#ifdef _WIN32
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) != 0) {
        fs::path exeDir = fs::path(exePath).parent_path();
        fs::path p1 = exeDir / relPath;
        if (fs::exists(p1)) return p1;
        fs::path p2 = exeDir / "shaders" / filename;
        if (fs::exists(p2)) return p2;
    }
#endif

    fs::path p3 = fs::current_path() / relPath;
    if (fs::exists(p3)) return p3;
    fs::path p4 = fs::current_path() / "shaders" / filename;
    if (fs::exists(p4)) return p4;

    fs::path p5 = fs::current_path() / "bin" / relPath;
    if (fs::exists(p5)) return p5;
    fs::path p6 = fs::current_path() / "bin" / "shaders" / filename;
    if (fs::exists(p6)) return p6;
    fs::path p7 = fs::current_path() / "build" / "bin" / "shaders" / filename;
    if (fs::exists(p7)) return p7;

    fs::path p8 = fs::current_path() / "Engine" / relPath;
    if (fs::exists(p8)) return p8;
    fs::path p9 = fs::current_path() / "Engine" / "shaders" / filename;
    if (fs::exists(p9)) return p9;

    return p;
}

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
    fs::path resolvedVert = ResolveShaderPath(vertexPath);
    fs::path resolvedFrag = ResolveShaderPath(fragmentPath);

    std::string vertexSource = ReadFile(resolvedVert.string());
    std::string fragmentSource = ReadFile(resolvedFrag.string());

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

    m_FileInfo = std::make_shared<ShaderFileInfo>();
    m_FileInfo->VertexPath = resolvedVert;
    m_FileInfo->FragmentPath = resolvedFrag;

    std::error_code ec;
    m_FileInfo->LastVertexTime = fs::last_write_time(resolvedVert, ec);
    if (ec) m_FileInfo->LastVertexTime = fs::file_time_type::min();
    m_FileInfo->LastFragmentTime = fs::last_write_time(resolvedFrag, ec);
    if (ec) m_FileInfo->LastFragmentTime = fs::file_time_type::min();
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
    fs::path resolved = ResolveShaderPath(path);
    std::ifstream file(resolved);
    if (!file.is_open()) {
        FL_ENGINE_ERROR("Failed to open shader file: {} (resolved: {})", path, resolved.string());
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
    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath, true);
#ifdef FREELY_ENABLE_SHADER_HOTRELOAD
    s_ActiveShaders.push_back(shader);
#endif
    return shader;
}

std::shared_ptr<Shader> Shader::CreateFromFileWithFallback(
    const std::string& vertexPath, const std::string& fragmentPath,
    const std::string& vertexFallback, const std::string& fragmentFallback) {

    fs::path resolvedVert = ResolveShaderPath(vertexPath);
    fs::path resolvedFrag = ResolveShaderPath(fragmentPath);

    std::string vertexSource = "";
    std::string fragmentSource = "";

    bool vertLoaded = false;
    std::ifstream vf(resolvedVert);
    if (vf) {
        std::stringstream ss; ss << vf.rdbuf();
        vertexSource = ss.str();
        vertLoaded = true;
    } else {
        vertexSource = vertexFallback;
    }

    bool fragLoaded = false;
    std::ifstream ff(resolvedFrag);
    if (ff) {
        std::stringstream ss; ss << ff.rdbuf();
        fragmentSource = ss.str();
        fragLoaded = true;
    } else {
        fragmentSource = fragmentFallback;
    }

    auto shader = std::make_shared<Shader>(vertexSource, fragmentSource);

    shader->m_FileInfo = std::make_shared<ShaderFileInfo>();
    shader->m_FileInfo->VertexPath = resolvedVert;
    shader->m_FileInfo->FragmentPath = resolvedFrag;

    std::error_code ec;
    shader->m_FileInfo->LastVertexTime = fs::last_write_time(resolvedVert, ec);
    if (ec) shader->m_FileInfo->LastVertexTime = fs::file_time_type::min();
    shader->m_FileInfo->LastFragmentTime = fs::last_write_time(resolvedFrag, ec);
    if (ec) shader->m_FileInfo->LastFragmentTime = fs::file_time_type::min();

#ifdef FREELY_ENABLE_SHADER_HOTRELOAD
    s_ActiveShaders.push_back(shader);
#endif

    return shader;
}

bool Shader::Reload(const std::string& vertexSource, const std::string& fragmentSource) {
    uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // Verify compile status first before creating program
    int vSuccess, fSuccess;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vSuccess);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fSuccess);
    if (!vSuccess || !fSuccess) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    uint32_t newProgram = glCreateProgram();
    glAttachShader(newProgram, vertexShader);
    glAttachShader(newProgram, fragmentShader);
    glLinkProgram(newProgram);

    int success;
    glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(newProgram, 512, nullptr, infoLog);
        FL_ENGINE_ERROR("Shader hot-reload: Program linking failed: {}", infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(newProgram);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (m_ID != 0) {
        glDeleteProgram(m_ID);
    }
    m_ID = newProgram;
    m_UniformCache.clear();
    return true;
}

bool Shader::ReloadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = ReadFile(vertexPath);
    std::string fragmentSource = ReadFile(fragmentPath);
    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }
    return Reload(vertexSource, fragmentSource);
}

void Shader::UpdateHotReload() {
#ifdef FREELY_ENABLE_SHADER_HOTRELOAD
    for (auto it = s_ActiveShaders.begin(); it != s_ActiveShaders.end(); ) {
        if (auto shader = it->lock()) {
            shader->CheckAndReload();
            ++it;
        } else {
            it = s_ActiveShaders.erase(it);
        }
    }
#endif
}

void Shader::CheckAndReload() {
#ifdef FREELY_ENABLE_SHADER_HOTRELOAD
    if (!m_FileInfo) return;

    std::error_code ec;
    auto vTime = fs::last_write_time(m_FileInfo->VertexPath, ec);
    if (ec) return;
    auto fTime = fs::last_write_time(m_FileInfo->FragmentPath, ec);
    if (ec) return;

    if (vTime != m_FileInfo->LastVertexTime || fTime != m_FileInfo->LastFragmentTime) {
        FL_ENGINE_INFO("Shader change detected. Reloading: {} and {}", m_FileInfo->VertexPath.string(), m_FileInfo->FragmentPath.string());
        if (ReloadFromFile(m_FileInfo->VertexPath.string(), m_FileInfo->FragmentPath.string())) {
            m_FileInfo->LastVertexTime = vTime;
            m_FileInfo->LastFragmentTime = fTime;
            FL_ENGINE_INFO("Shader reloaded successfully!");
        }
    }
#endif
}

} // namespace Freely
