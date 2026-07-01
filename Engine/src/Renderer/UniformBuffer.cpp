#include "Freely/Renderer/UniformBuffer.h"
#include <glad/glad.h>

namespace Freely {

UniformBuffer::UniformBuffer(uint32_t size, uint32_t bindingPoint)
    : m_BindingPoint(bindingPoint)
{
    glCreateBuffers(1, &m_ID);
    glNamedBufferData(m_ID, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_ID);
}

UniformBuffer::~UniformBuffer() {
    glDeleteBuffers(1, &m_ID);
}

void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
    glNamedBufferSubData(m_ID, offset, size, data);
}

std::shared_ptr<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t bindingPoint) {
    return std::make_shared<UniformBuffer>(size, bindingPoint);
}

} // namespace Freely
