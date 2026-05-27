#include "OpenGLResources.h"

namespace Freely::RHI {

OpenGLBuffer::OpenGLBuffer(const BufferDesc& desc)
    : m_Target(GLBufferTarget(desc.Type)),
      m_Usage(GLBufferUsage(desc.Usage)),
      m_Size(desc.Size),
      m_Type(desc.Type),
      m_DebugName(desc.DebugName)
{
    glGenBuffers(1, &m_Handle);
    glBindBuffer(m_Target, m_Handle);
    glBufferData(m_Target, static_cast<GLsizeiptr>(desc.Size), desc.Data, m_Usage);
}

OpenGLBuffer::~OpenGLBuffer() {
    if (m_Handle) glDeleteBuffers(1, &m_Handle);
}

void OpenGLBuffer::Update(const void* data, uint64_t size, uint64_t offset) {
    glBindBuffer(m_Target, m_Handle);
    glBufferSubData(m_Target,
                    static_cast<GLintptr>(offset),
                    static_cast<GLsizeiptr>(size),
                    data);
}

} // namespace Freely::RHI
