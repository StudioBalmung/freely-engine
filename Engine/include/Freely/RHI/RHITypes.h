#pragma once

#include <cstdint>
#include <string>

namespace Freely::RHI {

enum class GraphicsAPI : uint8_t {
    None,
    OpenGL,
    Vulkan,
    DirectX12
};

enum class TextureFormat : uint16_t {
    Unknown,
    R8,
    RG8,
    RGB8,
    RGBA8,
    SRGBA8,
    R16F,
    RG16F,
    RGBA16F,
    R32F,
    RG32F,
    RGBA32F,
    Depth16,
    Depth24,
    Depth32F,
    Depth24Stencil8
};

enum class TextureFilter : uint8_t {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

enum class TextureWrap : uint8_t {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

enum class BufferUsage : uint8_t {
    Static,
    Dynamic,
    Stream
};

enum class BufferType : uint8_t {
    Vertex,
    Index,
    Uniform,
    Storage
};

enum class IndexType : uint8_t {
    UInt16,
    UInt32
};

enum class PrimitiveTopology : uint8_t {
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan
};

enum class CullMode : uint8_t {
    None,
    Front,
    Back,
    FrontAndBack
};

enum class CompareOp : uint8_t {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always
};

enum class BlendFactor : uint8_t {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha
};

enum class ShaderStage : uint8_t {
    Vertex,
    Fragment,
    Geometry,
    TessControl,
    TessEvaluation,
    Compute
};

enum class VertexAttributeType : uint8_t {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    Bool,
    Mat3,
    Mat4
};

struct ClearValue {
    float Color[4]   = { 0.0f, 0.0f, 0.0f, 1.0f };
    float Depth      = 1.0f;
    uint32_t Stencil = 0;
};

struct Viewport {
    float X = 0.0f, Y = 0.0f;
    float Width = 0.0f, Height = 0.0f;
    float MinDepth = 0.0f, MaxDepth = 1.0f;
};

struct Rect2D {
    int32_t X = 0, Y = 0;
    uint32_t Width = 0, Height = 0;
};

const char* GraphicsAPIToString(GraphicsAPI api);
uint32_t    SizeOfVertexAttribute(VertexAttributeType t);
uint32_t    ComponentCountOfVertexAttribute(VertexAttributeType t);

} // namespace Freely::RHI
