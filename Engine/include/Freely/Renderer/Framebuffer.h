#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace Freely {

struct FramebufferSpec {
    uint32_t Width = 1280;
    uint32_t Height = 720;
    uint32_t Samples = 1;
    bool SwapChainTarget = false;
};

class Framebuffer {
public:
    Framebuffer(const FramebufferSpec& spec);
    ~Framebuffer();

    void Bind() const;
    void Unbind() const;
    void Resize(uint32_t width, uint32_t height);

    uint32_t GetColorAttachment() const { return m_ColorAttachment; }
    uint32_t GetDepthAttachment() const { return m_DepthAttachment; }
    const FramebufferSpec& GetSpec() const { return m_Spec; }

    static std::shared_ptr<Framebuffer> Create(const FramebufferSpec& spec);

private:
    void Invalidate();

    uint32_t m_ID = 0;
    uint32_t m_ColorAttachment = 0;
    uint32_t m_DepthAttachment = 0;
    FramebufferSpec m_Spec;
};

} // namespace Freely
