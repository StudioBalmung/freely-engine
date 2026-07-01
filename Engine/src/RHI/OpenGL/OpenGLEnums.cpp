#include "OpenGLResources.h"

namespace Freely::RHI {

GLenum GLBufferTarget(BufferType type) {
    switch (type) {
        case BufferType::Vertex:  return GL_ARRAY_BUFFER;
        case BufferType::Index:   return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::Uniform: return GL_UNIFORM_BUFFER;
        case BufferType::Storage: return GL_SHADER_STORAGE_BUFFER;
    }
    return GL_ARRAY_BUFFER;
}

GLenum GLBufferUsage(BufferUsage usage) {
    switch (usage) {
        case BufferUsage::Static:  return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
    }
    return GL_STATIC_DRAW;
}

GLenum GLPrimitive(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::Points:        return GL_POINTS;
        case PrimitiveTopology::Lines:         return GL_LINES;
        case PrimitiveTopology::LineStrip:     return GL_LINE_STRIP;
        case PrimitiveTopology::Triangles:     return GL_TRIANGLES;
        case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:   return GL_TRIANGLE_FAN;
    }
    return GL_TRIANGLES;
}

GLenum GLCompareOp(CompareOp op) {
    switch (op) {
        case CompareOp::Never:        return GL_NEVER;
        case CompareOp::Less:         return GL_LESS;
        case CompareOp::Equal:        return GL_EQUAL;
        case CompareOp::LessEqual:    return GL_LEQUAL;
        case CompareOp::Greater:      return GL_GREATER;
        case CompareOp::NotEqual:     return GL_NOTEQUAL;
        case CompareOp::GreaterEqual: return GL_GEQUAL;
        case CompareOp::Always:       return GL_ALWAYS;
    }
    return GL_LESS;
}

GLenum GLBlendFactor(BlendFactor f) {
    switch (f) {
        case BlendFactor::Zero:             return GL_ZERO;
        case BlendFactor::One:              return GL_ONE;
        case BlendFactor::SrcColor:         return GL_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:         return GL_DST_COLOR;
        case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:         return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:         return GL_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
    }
    return GL_ONE;
}

GLenum GLCullMode(CullMode c) {
    switch (c) {
        case CullMode::Front:        return GL_FRONT;
        case CullMode::Back:         return GL_BACK;
        case CullMode::FrontAndBack: return GL_FRONT_AND_BACK;
        default: return GL_BACK;
    }
}

GLenum GLShaderStage(ShaderStage s) {
    switch (s) {
        case ShaderStage::Vertex:         return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:       return GL_FRAGMENT_SHADER;
        case ShaderStage::Geometry:       return GL_GEOMETRY_SHADER;
        case ShaderStage::TessControl:    return GL_TESS_CONTROL_SHADER;
        case ShaderStage::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
        case ShaderStage::Compute:        return GL_COMPUTE_SHADER;
    }
    return GL_VERTEX_SHADER;
}

GLenum GLTextureFormat(TextureFormat f, bool isInternal) {
    switch (f) {
        case TextureFormat::R8:              return isInternal ? GL_R8     : GL_RED;
        case TextureFormat::RG8:             return isInternal ? GL_RG8    : GL_RG;
        case TextureFormat::RGB8:            return isInternal ? GL_RGB8   : GL_RGB;
        case TextureFormat::RGBA8:           return isInternal ? GL_RGBA8  : GL_RGBA;
        case TextureFormat::SRGBA8:          return isInternal ? GL_SRGB8_ALPHA8 : GL_RGBA;
        case TextureFormat::R16F:            return isInternal ? GL_R16F   : GL_RED;
        case TextureFormat::RG16F:           return isInternal ? GL_RG16F  : GL_RG;
        case TextureFormat::RGBA16F:         return isInternal ? GL_RGBA16F: GL_RGBA;
        case TextureFormat::R32F:            return isInternal ? GL_R32F   : GL_RED;
        case TextureFormat::RG32F:           return isInternal ? GL_RG32F  : GL_RG;
        case TextureFormat::RGBA32F:         return isInternal ? GL_RGBA32F: GL_RGBA;
        case TextureFormat::Depth16:         return isInternal ? GL_DEPTH_COMPONENT16  : GL_DEPTH_COMPONENT;
        case TextureFormat::Depth24:         return isInternal ? GL_DEPTH_COMPONENT24  : GL_DEPTH_COMPONENT;
        case TextureFormat::Depth32F:        return isInternal ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT;
        case TextureFormat::Depth24Stencil8: return isInternal ? GL_DEPTH24_STENCIL8   : GL_DEPTH_STENCIL;
        default: return isInternal ? GL_RGBA8 : GL_RGBA;
    }
}

GLenum GLTextureType(TextureFormat f) {
    switch (f) {
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::Depth32F:
            return GL_FLOAT;
        case TextureFormat::Depth16:
            return GL_UNSIGNED_SHORT;
        case TextureFormat::Depth24:
            return GL_UNSIGNED_INT;
        case TextureFormat::Depth24Stencil8:
            return GL_UNSIGNED_INT_24_8;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

GLenum GLTextureFilter(TextureFilter f) {
    switch (f) {
        case TextureFilter::Nearest:              return GL_NEAREST;
        case TextureFilter::Linear:               return GL_LINEAR;
        case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

GLenum GLTextureWrap(TextureWrap w) {
    switch (w) {
        case TextureWrap::Repeat:         return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

GLenum GLIndexType(IndexType t) {
    return t == IndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
}

uint32_t GLAttributeComponentType(VertexAttributeType t) {
    switch (t) {
        case VertexAttributeType::Float:
        case VertexAttributeType::Float2:
        case VertexAttributeType::Float3:
        case VertexAttributeType::Float4:
        case VertexAttributeType::Mat3:
        case VertexAttributeType::Mat4:
            return GL_FLOAT;
        case VertexAttributeType::Int:
        case VertexAttributeType::Int2:
        case VertexAttributeType::Int3:
        case VertexAttributeType::Int4:
            return GL_INT;
        case VertexAttributeType::UInt:
            return GL_UNSIGNED_INT;
        case VertexAttributeType::Bool:
            return GL_BOOL;
    }
    return GL_FLOAT;
}

} // namespace Freely::RHI
