#pragma once

#include "Editor/EditorContext.h"
#include <glm/glm.hpp>

namespace FreelyEditor {

class ViewportPanel {
public:
    ViewportPanel(EditorContext* context);
    
    void OnImGuiRender();

    void SetFramebufferTexture(uint32_t textureId) { m_FramebufferTexture = textureId; }
    
    bool IsHovered() const { return m_IsHovered; }
    bool IsFocused() const { return m_IsFocused; }
    glm::vec2 GetSize() const { return m_ViewportSize; }

private:
    EditorContext* m_Context;
    uint32_t m_FramebufferTexture = 0;
    
    bool m_IsHovered = false;
    bool m_IsFocused = false;
    glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
};

} // namespace FreelyEditor
