#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace Freely {

class VertexBuffer;
class IndexBuffer;

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddVertexBuffer(std::shared_ptr<VertexBuffer> buffer);
    void SetIndexBuffer(std::shared_ptr<IndexBuffer> buffer);

    const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

    uint32_t GetID() const { return m_ID; }

private:
    uint32_t m_ID = 0;
    uint32_t m_VertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
};

} // namespace Freely
