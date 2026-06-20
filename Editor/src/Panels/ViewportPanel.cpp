#include "Editor/Panels/ViewportPanel.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <Freely/ECS/Components.h>
#include <Freely/ECS/SceneGraph.h>

namespace FreelyEditor {

ViewportPanel::ViewportPanel(EditorContext* context)
    : m_Context(context)
{
}

void ViewportPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();

    m_IsFocused = ImGui::IsWindowFocused();
    m_IsHovered = ImGui::IsWindowHovered();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

    if (m_FramebufferTexture != 0) {
        ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(m_FramebufferTexture)), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
    } else {
        ImGui::Text("No Framebuffer Texture");
    }

    // Gizmos
    if (m_Context->ActiveScene && m_Context->SelectedEntity != entt::null && m_Context->GizmoType != -1) {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

        // Editor camera logic goes here. For now we assume an identity matrix if not implemented.
        // Needs a real EditorCamera to be functional
        glm::mat4 cameraProjection = glm::perspective(glm::radians(45.0f), m_ViewportSize.x / m_ViewportSize.y, 0.1f, 1000.0f);
        glm::mat4 cameraView = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10));

        // Entity transform
        auto& tc = m_Context->ActiveScene->GetRegistry().GetComponent<Freely::TransformComponent>(m_Context->SelectedEntity);
        glm::mat4 transform = tc.WorldMatrix;

        // Snapping
        bool snap = ImGui::GetIO().KeyCtrl;
        float snapValue = 0.5f; // Snap to 0.5m for translation/scale
        // Snap to 45 degrees for rotation
        if (m_Context->GizmoType == ImGuizmo::OPERATION::ROTATE)
            snapValue = 45.0f;

        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
            (ImGuizmo::OPERATION)m_Context->GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
            nullptr, snap ? snapValues : nullptr);

        if (ImGuizmo::IsUsing()) {
            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
            
            // Set the world transform via SceneGraph
            Freely::SceneGraph::SetWorldPosition(m_Context->ActiveScene->GetRegistry(), m_Context->SelectedEntity, translation);
            tc.SetEulerAngles(rotation);
            tc.Scale = scale;
            tc.Dirty = true;
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace FreelyEditor
