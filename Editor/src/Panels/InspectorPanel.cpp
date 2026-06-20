#include "Editor/Panels/InspectorPanel.h"
#include <Freely/ECS/Components.h>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace FreelyEditor {

InspectorPanel::InspectorPanel(EditorContext* context)
    : m_Context(context)
{
}

void InspectorPanel::OnImGuiRender() {
    ImGui::Begin("Inspector");

    if (m_Context->SelectedEntity != entt::null && m_Context->ActiveScene) {
        DrawComponents(m_Context->SelectedEntity);
    } else {
        ImGui::Text("No entity selected.");
    }

    ImGui::End();
}

static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    if (ImGui::Button("X", buttonSize)) values.x = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

void InspectorPanel::DrawComponents(entt::entity entity) {
    auto& registry = m_Context->ActiveScene->GetRegistry();

    if (registry.HasComponent<Freely::TagComponent>(entity)) {
        auto& tag = registry.GetComponent<Freely::TagComponent>(entity);
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, tag.Name.c_str(), sizeof(buffer) - 1);
        if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
            tag.Name = std::string(buffer);
        }
    }
    
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponent");
    }
    if (ImGui::BeginPopup("AddComponent")) {
        if (!registry.HasComponent<Freely::CameraComponent>(entity) && ImGui::MenuItem("Camera")) {
            registry.AddComponent<Freely::CameraComponent>(entity);
            ImGui::CloseCurrentPopup();
        }
        if (!registry.HasComponent<Freely::MeshRendererComponent>(entity) && ImGui::MenuItem("Mesh Renderer")) {
            registry.AddComponent<Freely::MeshRendererComponent>(entity);
            ImGui::CloseCurrentPopup();
        }
        if (!registry.HasComponent<Freely::LightComponent>(entity) && ImGui::MenuItem("Light")) {
            registry.AddComponent<Freely::LightComponent>(entity);
            ImGui::CloseCurrentPopup();
        }
        if (!registry.HasComponent<Freely::RigidBodyComponent>(entity) && ImGui::MenuItem("RigidBody")) {
            registry.AddComponent<Freely::RigidBodyComponent>(entity);
            ImGui::CloseCurrentPopup();
        }
        if (!registry.HasComponent<Freely::ColliderComponent>(entity) && ImGui::MenuItem("Collider")) {
            registry.AddComponent<Freely::ColliderComponent>(entity);
            ImGui::CloseCurrentPopup();
        }
        if (!registry.HasComponent<Freely::ScriptComponent>(entity) && ImGui::MenuItem("Script")) {
            registry.AddComponent<Freely::ScriptComponent>(entity);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopItemWidth();

    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    if (registry.HasComponent<Freely::TransformComponent>(entity)) {
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::TransformComponent).hash_code(), treeNodeFlags, "Transform");
        if (open) {
            auto& tc = registry.GetComponent<Freely::TransformComponent>(entity);
            DrawVec3Control("Position", tc.Position);
            
            glm::vec3 rotation = tc.GetEulerAngles();
            DrawVec3Control("Rotation", rotation);
            if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemEdited()) {
                tc.SetEulerAngles(rotation);
            }
            
            DrawVec3Control("Scale", tc.Scale, 1.0f);
            
            tc.Dirty = true; // Simple approach: always dirty if inspector is open. Optimized later.
            ImGui::TreePop();
        }
    }

    if (registry.HasComponent<Freely::CameraComponent>(entity)) {
        ImGui::PushID("CameraComponent");
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::CameraComponent).hash_code(), treeNodeFlags, "Camera");
        bool removeComponent = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Remove Component")) removeComponent = true;
            ImGui::EndPopup();
        }
        if (open) {
            auto& cc = registry.GetComponent<Freely::CameraComponent>(entity);
            
            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            int projType = static_cast<int>(cc.Projection);
            if (ImGui::Combo("Projection", &projType, projectionTypeStrings, 2)) {
                cc.Projection = static_cast<Freely::ProjectionType>(projType);
            }

            if (cc.Projection == Freely::ProjectionType::Perspective) {
                ImGui::DragFloat("FOV", &cc.FOV, 0.1f, 1.0f, 179.0f);
            } else {
                ImGui::DragFloat("Size", &cc.OrthoSize, 0.1f, 0.1f, 1000.0f);
            }
            
            ImGui::DragFloat("Near", &cc.NearPlane, 0.1f);
            ImGui::DragFloat("Far", &cc.FarPlane, 0.1f);
            ImGui::Checkbox("Primary", &cc.Primary);
            ImGui::Checkbox("Fixed Aspect", &cc.FixedAspect);
            if (cc.FixedAspect) {
                ImGui::DragFloat("Aspect Ratio", &cc.AspectRatio, 0.05f);
            }

            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::CameraComponent>(entity);
        ImGui::PopID();
    }
    
    // Stubs for the rest of the components
    if (registry.HasComponent<Freely::MeshRendererComponent>(entity)) {
        ImGui::PushID("MeshRendererComponent");
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::MeshRendererComponent).hash_code(), treeNodeFlags, "Mesh Renderer");
        bool removeComponent = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Remove Component")) removeComponent = true;
            ImGui::EndPopup();
        }
        if (open) {
            auto& mrc = registry.GetComponent<Freely::MeshRendererComponent>(entity);
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, mrc.MeshAsset.c_str(), sizeof(buffer) - 1);
            if (ImGui::InputText("Mesh", buffer, sizeof(buffer))) mrc.MeshAsset = std::string(buffer);
            
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, mrc.MaterialAsset.c_str(), sizeof(buffer) - 1);
            if (ImGui::InputText("Material", buffer, sizeof(buffer))) mrc.MaterialAsset = std::string(buffer);
            
            ImGui::Checkbox("Cast Shadows", &mrc.CastShadows);
            ImGui::Checkbox("Receive Shadows", &mrc.ReceiveShadows);
            ImGui::Checkbox("Visible", &mrc.Visible);
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::MeshRendererComponent>(entity);
        ImGui::PopID();
    }
    
    // Add other components similarly...
}

} // namespace FreelyEditor
