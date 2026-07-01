#include "Editor/Panels/ToolbarPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace FreelyEditor {

ToolbarPanel::ToolbarPanel(EditorContext* context)
    : m_Context(context)
{
}

void ToolbarPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
    const auto& buttonActive = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

    ImGui::Begin("##Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Play / Stop / Simulate buttons
    float size = ImGui::GetWindowHeight() - 4.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

    const char* playText = m_Context->State == EditorState::Edit ? "Play" : "Stop";
    if (ImGui::Button(playText, ImVec2(size * 2.0f, size))) {
        if (m_Context->State == EditorState::Edit) {
            m_Context->State = EditorState::Play;
            if (m_Context->ActiveScene) m_Context->ActiveScene->Start();
        } else {
            m_Context->State = EditorState::Edit;
            if (m_Context->ActiveScene) m_Context->ActiveScene->Stop();
        }
    }

    // Gizmo toggles (left aligned)
    ImGui::SameLine(10.0f);
    if (ImGui::Button("Select", ImVec2(size * 2.0f, size))) m_Context->GizmoType = -1;
    ImGui::SameLine();
    if (ImGui::Button("Translate", ImVec2(size * 3.0f, size))) m_Context->GizmoType = ImGuizmo::OPERATION::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::Button("Rotate", ImVec2(size * 2.5f, size))) m_Context->GizmoType = ImGuizmo::OPERATION::ROTATE;
    ImGui::SameLine();
    if (ImGui::Button("Scale", ImVec2(size * 2.0f, size))) m_Context->GizmoType = ImGuizmo::OPERATION::SCALE;

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(10.0f, 0.0f));
    ImGui::SameLine();
    if (ImGui::Button(m_Context->GizmoWorldSpace ? "World" : "Local", ImVec2(size * 2.5f, size))) {
        m_Context->GizmoWorldSpace = !m_Context->GizmoWorldSpace;
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    ImGui::End();
}

} // namespace FreelyEditor
