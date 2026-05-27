#pragma once

#include "RHITypes.h"

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace Freely::RHI {

/// Vertex attribute specification (used by IPipeline / vertex layouts).
struct VertexAttribute {
    std::string Name;
    VertexAttributeType Type;
    uint32_t Location  = 0;
    uint32_t Offset    = 0;
    bool     Normalized= false;
};

struct VertexLayout {
    std::vector<VertexAttribute> Attributes;
    uint32_t Stride = 0;
};

struct BufferDesc {
    BufferType  Type   = BufferType::Vertex;
    BufferUsage Usage  = BufferUsage::Static;
    uint64_t    Size   = 0;
    const void* Data   = nullptr;
    std::string DebugName;
};

struct TextureDesc {
    uint32_t Width = 0, Height = 0, Depth = 1;
    uint32_t MipLevels = 1;
    uint32_t SampleCount = 1;
    TextureFormat Format = TextureFormat::RGBA8;
    TextureFilter MinFilter = TextureFilter::Linear;
    TextureFilter MagFilter = TextureFilter::Linear;
    TextureWrap   WrapU = TextureWrap::Repeat;
    TextureWrap   WrapV = TextureWrap::Repeat;
    TextureWrap   WrapW = TextureWrap::Repeat;
    bool          GenerateMipmaps = false;
    bool          IsRenderTarget  = false;
    const void*   InitialData     = nullptr;
    std::string   DebugName;
};

struct ShaderDesc {
    ShaderStage Stage    = ShaderStage::Vertex;
    std::string Source;          // GLSL / HLSL source
    std::string EntryPoint = "main";
    std::string DebugName;
};

struct PipelineDesc {
    std::vector<std::shared_ptr<class IShader>> Shaders;
    VertexLayout       Layout;
    PrimitiveTopology  Topology       = PrimitiveTopology::Triangles;
    CullMode           Culling        = CullMode::Back;
    CompareOp          DepthCompare   = CompareOp::Less;
    bool               DepthTest      = true;
    bool               DepthWrite     = true;
    bool               BlendEnable    = false;
    BlendFactor        SrcColor       = BlendFactor::SrcAlpha;
    BlendFactor        DstColor       = BlendFactor::OneMinusSrcAlpha;
    BlendFactor        SrcAlpha       = BlendFactor::One;
    BlendFactor        DstAlpha       = BlendFactor::OneMinusSrcAlpha;
    bool               Wireframe      = false;
    std::string        DebugName;
};

struct FramebufferAttachment {
    TextureFormat Format    = TextureFormat::RGBA8;
    bool          IsDepth   = false;
    bool          IsStencil = false;
};

struct FramebufferDesc {
    uint32_t Width = 0, Height = 0;
    uint32_t SampleCount = 1;
    std::vector<FramebufferAttachment> ColorAttachments;
    FramebufferAttachment DepthAttachment;
    bool HasDepth = false;
    std::string DebugName;
};

// ---------- RHI resource interfaces ----------

class IResource {
public:
    virtual ~IResource() = default;
    virtual const std::string& GetDebugName() const = 0;
};

class IBuffer : public IResource {
public:
    virtual void   Update(const void* data, uint64_t size, uint64_t offset = 0) = 0;
    virtual uint64_t GetSize() const = 0;
    virtual BufferType GetType() const = 0;
};

class ITexture : public IResource {
public:
    virtual uint32_t GetWidth() const  = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual TextureFormat GetFormat() const = 0;
    virtual void* GetNativeHandle() const = 0; // for ImGui / gizmos
};

class IShader : public IResource {
public:
    virtual ShaderStage GetStage() const = 0;
};

class IPipeline : public IResource {};

class IFramebuffer : public IResource {
public:
    virtual uint32_t GetWidth() const  = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual std::shared_ptr<ITexture> GetColorAttachment(uint32_t index = 0) const = 0;
    virtual std::shared_ptr<ITexture> GetDepthAttachment() const = 0;
};

} // namespace Freely::RHI
