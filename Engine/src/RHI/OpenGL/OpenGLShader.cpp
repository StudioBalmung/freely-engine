#include "OpenGLResources.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {

OpenGLShader::OpenGLShader(const ShaderDesc& desc)
    : m_Stage(desc.Stage), m_DebugName(desc.DebugName)
{
    m_Handle = glCreateShader(GLShaderStage(desc.Stage));
    const char* src = desc.Source.c_str();
    glShaderSource(m_Handle, 1, &src, nullptr);
    glCompileShader(m_Handle);

    GLint status = 0;
    glGetShaderiv(m_Handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        char info[2048];
        GLsizei len = 0;
        glGetShaderInfoLog(m_Handle, sizeof(info), &len, info);
        FL_ENGINE_ERROR("Shader '{}' compile failed:\n{}", m_DebugName, info);
    }
}

OpenGLShader::~OpenGLShader() {
    if (m_Handle) glDeleteShader(m_Handle);
}

} // namespace Freely::RHI
