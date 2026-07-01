#include "Freely/RHI/RHITypes.h"

namespace Freely::RHI {

const char* GraphicsAPIToString(GraphicsAPI api) {
    switch (api) {
        case GraphicsAPI::OpenGL:    return "OpenGL";
        case GraphicsAPI::Vulkan:    return "Vulkan";
        case GraphicsAPI::DirectX12: return "DirectX12";
        case GraphicsAPI::None:      return "None";
    }
    return "Unknown";
}

uint32_t SizeOfVertexAttribute(VertexAttributeType t) {
    switch (t) {
        case VertexAttributeType::Float:  return 4;
        case VertexAttributeType::Float2: return 8;
        case VertexAttributeType::Float3: return 12;
        case VertexAttributeType::Float4: return 16;
        case VertexAttributeType::Int:    return 4;
        case VertexAttributeType::Int2:   return 8;
        case VertexAttributeType::Int3:   return 12;
        case VertexAttributeType::Int4:   return 16;
        case VertexAttributeType::UInt:   return 4;
        case VertexAttributeType::Bool:   return 1;
        case VertexAttributeType::Mat3:   return 36;
        case VertexAttributeType::Mat4:   return 64;
    }
    return 0;
}

uint32_t ComponentCountOfVertexAttribute(VertexAttributeType t) {
    switch (t) {
        case VertexAttributeType::Float:
        case VertexAttributeType::Int:
        case VertexAttributeType::UInt:
        case VertexAttributeType::Bool:   return 1;
        case VertexAttributeType::Float2:
        case VertexAttributeType::Int2:   return 2;
        case VertexAttributeType::Float3:
        case VertexAttributeType::Int3:
        case VertexAttributeType::Mat3:   return 3;
        case VertexAttributeType::Float4:
        case VertexAttributeType::Int4:
        case VertexAttributeType::Mat4:   return 4;
    }
    return 0;
}

} // namespace Freely::RHI
