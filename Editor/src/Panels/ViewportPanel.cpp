#include "Editor/Panels/ViewportPanel.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include <Freely/ECS/Components.h>
#include <Freely/ECS/SceneGraph.h>
#include <Freely/Scene/Camera.h>

#include "Editor/Commands/EntityCommands.h"

namespace FreelyEditor {

ViewportPanel::ViewportPanel(EditorContext* context)
    : m_Context(context)
{
}

void ViewportPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    m_IsFocused = ImGui::IsWindowFocused();
    m_IsHovered = ImGui::IsWindowHovered();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

    if (m_FramebufferTexture != 0) {
        ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(m_FramebufferTexture)), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
    } else {
        ImGui::Text("No Framebuffer Texture");
    }

    DrawGizmo();

    ImGui::End();
    ImGui::PopStyleVar();
}

void ViewportPanel::DrawGizmo() {
    if (!m_Context->ActiveScene || m_Context->SelectedEntity == entt::null || m_Context->GizmoType == -1 || m_Camera == nullptr) {
        m_IsManipulating = false;
        return;
    }

    if (!m_Context->ActiveScene->GetRegistry().IsValid(m_Context->SelectedEntity)) {
        m_IsManipulating = false;
        return;
    }

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_ViewportSize.x, m_ViewportSize.y);

    // Real editor camera matrices instead of a hardcoded placeholder view/projection.
    const glm::mat4& cameraView = m_Camera->GetViewMatrix();
    const glm::mat4& cameraProjection = m_Camera->GetProjectionMatrix();

    auto& transform = m_Context->ActiveScene->GetRegistry().GetComponent<Freely::TransformComponent>(m_Context->SelectedEntity);
    glm::mat4 matrix = transform.WorldMatrix;

    const bool snap = ImGui::GetIO().KeyCtrl;
    float snapValue = (m_Context->GizmoType == ImGuizmo::OPERATION::ROTATE) ? 45.0f : 0.5f;
    float snapValues[3] = { snapValue, snapValue, snapValue };

    const ImGuizmo::MODE mode = m_Context->GizmoWorldSpace ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

    const bool wasManipulating = m_IsManipulating;
    if (!wasManipulating) {
        m_DragStartPosition = transform.Position;
        m_DragStartRotation = transform.Rotation;
        m_DragStartScale = transform.Scale;
    }

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
        static_cast<ImGuizmo::OPERATION>(m_Context->GizmoType), mode, glm::value_ptr(matrix),
        nullptr, snap ? snapValues : nullptr);

    m_IsManipulating = ImGuizmo::IsUsing();

    if (m_IsManipulating) {
        glm::vec3 translation, rotationDegrees, scale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(matrix), glm::value_ptr(translation), glm::value_ptr(rotationDegrees), glm::value_ptr(scale));

        Freely::SceneGraph::SetWorldPosition(m_Context->ActiveScene->GetRegistry(), m_Context->SelectedEntity, translation);
        transform.SetEulerAngles(rotationDegrees);
        transform.Scale = scale;
        transform.Dirty = true;
    } else if (wasManipulating) {
        // Drag just ended this frame: push one merged undo step covering the whole drag.
        m_Context->ExecuteCommand(std::make_unique<TransformEntityCommand>(
            m_Context->ActiveScene, m_Context->SelectedEntity,
            m_DragStartPosition, m_DragStartRotation, m_DragStartScale,
            transform.Position, transform.Rotation, transform.Scale));
    }
}

} // namespace FreelyEditor
