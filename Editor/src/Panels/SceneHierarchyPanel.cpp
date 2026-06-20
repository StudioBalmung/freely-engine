#include "Editor/Panels/SceneHierarchyPanel.h"
#include <Freely/ECS/Components.h>
#include <imgui.h>

namespace FreelyEditor {

SceneHierarchyPanel::SceneHierarchyPanel(EditorContext* context)
    : m_Context(context)
{
}

void SceneHierarchyPanel::OnImGuiRender() {
    ImGui::Begin("Scene Hierarchy");

    if (m_Context->ActiveScene) {
        m_Context->ActiveScene->GetRegistry().ForEachRootEntity([&](entt::entity entity) {
            DrawEntityNode(entity);
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            m_Context->SelectedEntity = entt::null;
        }

        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow("HierarchyPopup", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_Context->ActiveScene->CreateEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(entt::entity entity) {
    auto& tag = m_Context->ActiveScene->GetRegistry().GetComponent<Freely::TagComponent>(entity);
    
    ImGuiTreeNodeFlags flags = ((m_Context->SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    
    // Check if it has children
    auto& rel = m_Context->ActiveScene->GetRegistry().GetComponent<Freely::RelationshipComponent>(entity);
    if (rel.FirstChild == entt::null) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    
    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Name.c_str());
    
    if (ImGui::IsItemClicked()) {
        m_Context->SelectedEntity = entity;
    }

    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            entityDeleted = true;
        }
        if (ImGui::MenuItem("Create Child Entity")) {
            entt::entity child = m_Context->ActiveScene->CreateEntity("Empty Entity");
            m_Context->ActiveScene->GetRegistry().SetParent(child, entity);
        }
        ImGui::EndPopup();
    }
    
    // Drag and drop reparenting could go here
    
    if (opened) {
        if (!entityDeleted) {
            entt::entity child = rel.FirstChild;
            while (child != entt::null) {
                // We need to fetch next sibling before drawing, in case drawing deletes it
                auto next = m_Context->ActiveScene->GetRegistry().GetComponent<Freely::RelationshipComponent>(child).NextSibling;
                DrawEntityNode(child);
                child = next;
            }
        }
        ImGui::TreePop();
    }
    
    if (entityDeleted) {
        if (m_Context->SelectedEntity == entity) {
            m_Context->SelectedEntity = entt::null;
        }
        m_Context->ActiveScene->DestroyEntity(entity);
    }
}

} // namespace FreelyEditor
