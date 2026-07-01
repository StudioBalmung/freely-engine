#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Freely {

enum class ShaderDataType {
    None = 0,
    Float, Float2, Float3, Float4,
    Int, Int2, Int3, Int4,
    Mat3, Mat4,
    Bool
};

uint32_t ShaderDataTypeSize(ShaderDataType type);

struct BufferElement {
    std::string Name;
    ShaderDataType Type;
    uint32_t Size;
    uint32_t Offset;
    bool Normalized;

    BufferElement(ShaderDataType type, const std::string& name, bool normalized = false);
    uint32_t GetComponentCount() const;
};

class BufferLayout {
public:
    BufferLayout() = default;
    BufferLayout(std::initializer_list<BufferElement> elements);

    const std::vector<BufferElement>& GetElements() const { return m_Elements; }
    uint32_t GetStride() const { return m_Stride; }

    auto begin() { return m_Elements.begin(); }
    auto end() { return m_Elements.end(); }
    auto begin() const { return m_Elements.begin(); }
    auto end() const { return m_Elements.end(); }

private:
    void CalculateOffsetsAndStride();
    std::vector<BufferElement> m_Elements;
    uint32_t m_Stride = 0;
};

class VertexBuffer {
public:
    VertexBuffer(const float* vertices, uint32_t size);
    VertexBuffer(uint32_t size); // Dynamic buffer
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void SetData(const void* data, uint32_t size);

    void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
    const BufferLayout& GetLayout() const { return m_Layout; }

    uint32_t GetID() const { return m_ID; }

private:
    uint32_t m_ID = 0;
    BufferLayout m_Layout;
};

class IndexBuffer {
public:
    IndexBuffer(const uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const { return m_Count; }
    uint32_t GetID() const { return m_ID; }

private:
    uint32_t m_ID = 0;
    uint32_t m_Count = 0;
};

} // namespace Freely
