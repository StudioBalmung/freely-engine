#pragma once

#include "Freely/RHI/RHIResources.h"

#include <glad/glad.h>
#include <vector>
#include <unordered_map>

namespace Freely::RHI {

GLenum   GLBufferTarget(BufferType type);
GLenum   GLBufferUsage(BufferUsage usage);
GLenum   GLPrimitive(PrimitiveTopology topology);
GLenum   GLCompareOp(CompareOp op);
GLenum   GLBlendFactor(BlendFactor f);
GLenum   GLCullMode(CullMode c);
GLenum   GLShaderStage(ShaderStage s);
GLenum   GLTextureFormat(TextureFormat f, bool isInternal);
GLenum   GLTextureType(TextureFormat f);
GLenum   GLTextureFilter(TextureFilter f);
GLenum   GLTextureWrap(TextureWrap w);
GLenum   GLIndexType(IndexType t);
uint32_t GLAttributeComponentType(VertexAttributeType t);

class OpenGLBuffer : public IBuffer {
public:
    OpenGLBuffer(const BufferDesc& desc);
    ~OpenGLBuffer() override;

    void   Update(const void* data, uint64_t size, uint64_t offset) override;
    uint64_t   GetSize() const override { return m_Size; }
    BufferType GetType() const override { return m_Type; }
    const std::string& GetDebugName() const override { return m_DebugName; }
    GLuint Handle() const { return m_Handle; }
    GLenum Target() const { return m_Target; }

private:
    GLuint     m_Handle = 0;
    GLenum     m_Target = 0;
    GLenum     m_Usage  = 0;
    uint64_t   m_Size   = 0;
    BufferType m_Type;
    std::string m_DebugName;
};

class OpenGLTexture : public ITexture {
public:
    OpenGLTexture(const TextureDesc& desc);
    ~OpenGLTexture() override;

    uint32_t GetWidth() const override  { return m_Desc.Width; }
    uint32_t GetHeight() const override { return m_Desc.Height; }
    TextureFormat GetFormat() const override { return m_Desc.Format; }
    void* GetNativeHandle() const override { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_Handle)); }
    const std::string& GetDebugName() const override { return m_Desc.DebugName; }
    GLuint Handle() const { return m_Handle; }

private:
    GLuint      m_Handle = 0;
    TextureDesc m_Desc;
};

class OpenGLShader : public IShader {
public:
    OpenGLShader(const ShaderDesc& desc);
    ~OpenGLShader() override;
    ShaderStage GetStage() const override { return m_Stage; }
    const std::string& GetDebugName() const override { return m_DebugName; }
    GLuint Handle() const { return m_Handle; }

private:
    GLuint      m_Handle = 0;
    ShaderStage m_Stage;
    std::string m_DebugName;
};

class OpenGLPipeline : public IPipeline {
public:
    OpenGLPipeline(const PipelineDesc& desc);
    ~OpenGLPipeline() override;

    const std::string& GetDebugName() const override { return m_DebugName; }
    GLuint Program() const { return m_Program; }
    const PipelineDesc& Desc() const { return m_Desc; }

private:
    GLuint       m_Program = 0;
    PipelineDesc m_Desc;
    std::string  m_DebugName;
};

class OpenGLFramebuffer : public IFramebuffer {
public:
    OpenGLFramebuffer(const FramebufferDesc& desc);
    ~OpenGLFramebuffer() override;

    uint32_t GetWidth() const override  { return m_Desc.Width; }
    uint32_t GetHeight() const override { return m_Desc.Height; }
    void Resize(uint32_t width, uint32_t height) override;

    std::shared_ptr<ITexture> GetColorAttachment(uint32_t index) const override;
    std::shared_ptr<ITexture> GetDepthAttachment() const override { return m_Depth; }
    const std::string& GetDebugName() const override { return m_Desc.DebugName; }

    GLuint Handle() const { return m_Handle; }

private:
    void Build();
    void Destroy();

    GLuint m_Handle = 0;
    FramebufferDesc m_Desc;
    std::vector<std::shared_ptr<OpenGLTexture>> m_Color;
    std::shared_ptr<OpenGLTexture> m_Depth;
};

} // namespace Freely::RHI
