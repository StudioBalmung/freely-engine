#pragma once

#include "Editor/EditorContext.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Freely { class Camera; }

namespace FreelyEditor {

class ViewportPanel {
public:
    ViewportPanel(EditorContext* context);

    void OnImGuiRender();

    void SetFramebufferTexture(uint32_t textureId) { m_FramebufferTexture = textureId; }

    // The EditorApp owns the actual Freely::Camera; ViewportPanel only reads
    // its view/projection matrices each frame to draw the gizmo in the right
    // place. Pointer is non-owning and may be null before the camera exists.
    void SetCamera(const Freely::Camera* camera) { m_Camera = camera; }

    bool IsHovered() const { return m_IsHovered; }
    bool IsFocused() const { return m_IsFocused; }
    glm::vec2 GetSize() const { return m_ViewportSize; }

private:
    void DrawGizmo();

    EditorContext* m_Context;
    const Freely::Camera* m_Camera = nullptr;
    uint32_t m_FramebufferTexture = 0;

    bool m_IsHovered = false;
    bool m_IsFocused = false;
    glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

    // Tracks the entity transform at the moment a gizmo drag starts, so the
    // whole drag can be pushed onto the undo stack as a single command when
    // the drag ends instead of one command per mouse-move frame.
    bool m_IsManipulating = false;
    glm::vec3 m_DragStartPosition{0.0f};
    glm::quat m_DragStartRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_DragStartScale{1.0f};
};

} // namespace FreelyEditor
