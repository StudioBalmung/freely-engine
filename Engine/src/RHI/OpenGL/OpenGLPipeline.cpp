#include "OpenGLResources.h"
#include "Freely/Core/Logger.h"

namespace Freely::RHI {

OpenGLPipeline::OpenGLPipeline(const PipelineDesc& desc)
    : m_Desc(desc), m_DebugName(desc.DebugName)
{
    m_Program = glCreateProgram();
    for (const auto& shader : desc.Shaders) {
        auto* glShader = static_cast<OpenGLShader*>(shader.get());
        if (glShader) {
            glAttachShader(m_Program, glShader->Handle());
        }
    }
    glLinkProgram(m_Program);

    GLint status = 0;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        char info[2048];
        GLsizei len = 0;
        glGetProgramInfoLog(m_Program, sizeof(info), &len, info);
        FL_ENGINE_ERROR("Pipeline '{}' link failed:\n{}", m_DebugName, info);
    }

    for (const auto& shader : desc.Shaders) {
        auto* glShader = static_cast<OpenGLShader*>(shader.get());
        if (glShader) {
            glDetachShader(m_Program, glShader->Handle());
        }
    }
}

OpenGLPipeline::~OpenGLPipeline() {
    if (m_Program) glDeleteProgram(m_Program);
}

} // namespace Freely::RHI
