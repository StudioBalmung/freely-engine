#pragma once
// Freely Engine - UniformBuffer
// OpenGL Uniform Buffer Object wrapper.  Bind to a named binding point so
// multiple shaders can share the same camera/lights data without per-draw
// uniform uploads.

#include <cstdint>
#include <memory>

namespace Freely {

class UniformBuffer {
public:
    /// Allocate a UBO of `size` bytes and bind it to `bindingPoint`.
    UniformBuffer(uint32_t size, uint32_t bindingPoint);
    ~UniformBuffer();

    /// Upload `size` bytes from `data` starting at byte `offset`.
    void SetData(const void* data, uint32_t size, uint32_t offset = 0);

    uint32_t GetID()           const { return m_ID; }
    uint32_t GetBindingPoint() const { return m_BindingPoint; }

    static std::shared_ptr<UniformBuffer> Create(uint32_t size, uint32_t bindingPoint);

private:
    uint32_t m_ID           = 0;
    uint32_t m_BindingPoint = 0;
};

} // namespace Freely
