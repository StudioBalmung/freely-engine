#include "Editor/Panels/InspectorPanel.h"
#include <Freely/ECS/Components.h>
#include <imgui.h>
#include <imgui_internal.h>
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

    float lineHeight = ImGui::GetFrameHeight();
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

    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding;

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
    

    // --- MaterialComponent ----------------------------------------------------
    if (registry.HasComponent<Freely::MaterialComponent>(entity)) {
        ImGui::PushID("MaterialComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::MaterialComponent).hash_code(), treeNodeFlags, "Material");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& mc = registry.GetComponent<Freely::MaterialComponent>(entity);
            ImGui::ColorEdit3("Albedo",    glm::value_ptr(mc.Albedo));
            ImGui::SliderFloat("Metallic", &mc.Metallic,  0.0f, 1.0f);
            ImGui::SliderFloat("Roughness",&mc.Roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("AO",       &mc.AO,        0.0f, 1.0f);
            ImGui::ColorEdit3("Emissive",  glm::value_ptr(mc.Emissive));
            ImGui::DragFloat("Emissive Strength", &mc.EmissiveStrength, 0.1f, 0.0f, 100.0f);
            ImGui::Separator();
            char buf[256];
            auto pathField = [&](const char* label, std::string& path) {
                memset(buf, 0, sizeof(buf)); strncpy(buf, path.c_str(), sizeof(buf)-1);
                if (ImGui::InputText(label, buf, sizeof(buf))) path = buf;
            };
            pathField("Albedo Map",    mc.AlbedoMapPath);
            pathField("Normal Map",    mc.NormalMapPath);
            pathField("MetRough Map",  mc.MetallicRoughnessMapPath);
            pathField("AO Map",        mc.AOMapPath);
            pathField("Emissive Map",  mc.EmissiveMapPath);
            ImGui::Checkbox("Two-Sided", &mc.TwoSided);
            ImGui::DragFloat("Alpha Cutoff", &mc.AlphaCutoff, 0.01f, 0.0f, 1.0f);
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::MaterialComponent>(entity);
        ImGui::PopID();
    }

    // --- LightComponent -------------------------------------------------------
    if (registry.HasComponent<Freely::LightComponent>(entity)) {
        ImGui::PushID("LightComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::LightComponent).hash_code(), treeNodeFlags, "Light");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& lc = registry.GetComponent<Freely::LightComponent>(entity);
            const char* types[] = {"Directional","Point","Spot"};
            int lt = (int)lc.Type;
            if (ImGui::Combo("Type", &lt, types, 3)) lc.Type = (Freely::LightType)lt;
            ImGui::ColorEdit3("Color",     glm::value_ptr(lc.Color));
            ImGui::DragFloat("Intensity", &lc.Intensity, 0.05f, 0.0f, 100.0f);
            if (lc.Type != Freely::LightType::Directional) {
                ImGui::DragFloat("Range",     &lc.Range,    0.5f, 0.0f, 500.0f);
                ImGui::DragFloat("Constant",  &lc.Constant, 0.01f, 0.01f, 10.0f);
                ImGui::DragFloat("Linear",    &lc.Linear,   0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Quadratic", &lc.Quadratic,0.0001f, 0.0f, 1.0f);
            }
            if (lc.Type == Freely::LightType::Spot) {
                ImGui::DragFloat("Inner Cutoff", &lc.InnerCutoff, 0.5f, 0.0f, 90.0f);
                ImGui::DragFloat("Outer Cutoff", &lc.OuterCutoff, 0.5f, 0.0f, 90.0f);
            }
            ImGui::Separator();
            ImGui::Checkbox("Cast Shadows", &lc.CastShadows);
            if (lc.CastShadows) {
                int sizes[] = {512,1024,2048,4096};
                const char* sizeLabels[] = {"512","1024","2048","4096"};
                int idx = 2;
                for (int i=0;i<4;i++) if (sizes[i]==lc.ShadowMapSize) idx=i;
                if (ImGui::Combo("Shadow Res", &idx, sizeLabels, 4)) lc.ShadowMapSize = sizes[idx];
                ImGui::DragFloat("Shadow Bias", &lc.ShadowBias, 0.0001f, 0.0f, 0.1f, "%.5f");
            }
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::LightComponent>(entity);
        ImGui::PopID();
    }

    // --- RigidBodyComponent --------------------------------------------------
    if (registry.HasComponent<Freely::RigidBodyComponent>(entity)) {
        ImGui::PushID("RigidBodyComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::RigidBodyComponent).hash_code(), treeNodeFlags, "Rigidbody");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& rb = registry.GetComponent<Freely::RigidBodyComponent>(entity);
            const char* bt[] = {"Static","Dynamic","Kinematic"};
            int bti = (int)rb.Type;
            if (ImGui::Combo("Body Type", &bti, bt, 3)) rb.Type = (Freely::BodyType)bti;
            if (rb.Type != Freely::BodyType::Static) {
                ImGui::DragFloat("Mass",            &rb.Mass,            0.1f, 0.001f, 10000.0f);
                ImGui::DragFloat("Linear Damping",  &rb.LinearDamping,   0.01f, 0.0f, 10.0f);
                ImGui::DragFloat("Angular Damping", &rb.AngularDamping,  0.01f, 0.0f, 10.0f);
                ImGui::DragFloat("Friction",        &rb.Friction,        0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Restitution",     &rb.Restitution,     0.01f, 0.0f, 1.0f);
                ImGui::Checkbox("Use Gravity",    &rb.UseGravity);
                ImGui::Checkbox("Continuous CD",  &rb.ContinuousCD);
                ImGui::Text("Freeze Rotation:");
                ImGui::SameLine(); ImGui::Checkbox("X",&rb.FreezeRotationX);
                ImGui::SameLine(); ImGui::Checkbox("Y",&rb.FreezeRotationY);
                ImGui::SameLine(); ImGui::Checkbox("Z",&rb.FreezeRotationZ);
            }
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::RigidBodyComponent>(entity);
        ImGui::PopID();
    }

    // --- ColliderComponent ---------------------------------------------------
    if (registry.HasComponent<Freely::ColliderComponent>(entity)) {
        ImGui::PushID("ColliderComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::ColliderComponent).hash_code(), treeNodeFlags, "Collider");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& col = registry.GetComponent<Freely::ColliderComponent>(entity);
            const char* shapes[] = {"Box","Sphere","Capsule","Cylinder","Plane","Mesh","Heightfield"};
            int si = (int)col.Shape;
            if (ImGui::Combo("Shape", &si, shapes, 7)) col.Shape = (Freely::ColliderShape)si;
            DrawVec3Control("Center", col.Center, 0.0f, 80.0f);
            if (col.Shape == Freely::ColliderShape::Box)
                DrawVec3Control("Half Extents", col.BoxHalfExtents, 0.5f, 80.0f);
            else if (col.Shape == Freely::ColliderShape::Sphere)
                ImGui::DragFloat("Radius", &col.SphereRadius, 0.05f, 0.001f, 100.0f);
            else if (col.Shape == Freely::ColliderShape::Capsule) {
                ImGui::DragFloat("Radius", &col.CapsuleRadius, 0.05f, 0.001f, 100.0f);
                ImGui::DragFloat("Height", &col.CapsuleHeight, 0.05f, 0.001f, 100.0f);
            }
            ImGui::Checkbox("Is Trigger", &col.IsTrigger);
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::ColliderComponent>(entity);
        ImGui::PopID();
    }

    // --- AudioSourceComponent ------------------------------------------------
    if (registry.HasComponent<Freely::AudioSourceComponent>(entity)) {
        ImGui::PushID("AudioSourceComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::AudioSourceComponent).hash_code(), treeNodeFlags, "Audio Source");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& ac = registry.GetComponent<Freely::AudioSourceComponent>(entity);
            char buf[256]; memset(buf,0,sizeof(buf)); strncpy(buf,ac.ClipPath.c_str(),sizeof(buf)-1);
            if (ImGui::InputText("Clip",buf,sizeof(buf))) ac.ClipPath=buf;
            ImGui::SliderFloat("Volume",       &ac.Volume, 0.0f, 1.0f);
            ImGui::DragFloat("Pitch",          &ac.Pitch,  0.01f, 0.1f, 4.0f);
            ImGui::DragFloat("Min Distance",   &ac.MinDistance, 0.1f, 0.0f, 1000.0f);
            ImGui::DragFloat("Max Distance",   &ac.MaxDistance, 1.0f, 0.0f, 10000.0f);
            ImGui::Checkbox("Loop",           &ac.Loop);
            ImGui::Checkbox("Play On Awake",  &ac.PlayOnAwake);
            ImGui::Checkbox("3D Spatial",     &ac.Spatial);
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::AudioSourceComponent>(entity);
        ImGui::PopID();
    }

    // --- ScriptComponent -----------------------------------------------------
    if (registry.HasComponent<Freely::ScriptComponent>(entity)) {
        ImGui::PushID("ScriptComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::ScriptComponent).hash_code(), treeNodeFlags, "Script");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& sc = registry.GetComponent<Freely::ScriptComponent>(entity);
            char buf[256];
            memset(buf,0,sizeof(buf)); strncpy(buf,sc.ClassName.c_str(),sizeof(buf)-1);
            if (ImGui::InputText("Class Name", buf, sizeof(buf))) sc.ClassName = buf;
            memset(buf,0,sizeof(buf)); strncpy(buf,sc.ScriptPath.c_str(),sizeof(buf)-1);
            if (ImGui::InputText("Script Path",buf, sizeof(buf))) sc.ScriptPath = buf;
            const char* langs[] = {"lua","csharp"};
            int li = (sc.Language == "csharp") ? 1 : 0;
            if (ImGui::Combo("Language",&li,langs,2)) sc.Language = langs[li];
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::ScriptComponent>(entity);
        ImGui::PopID();
    }

    // --- SpriteRendererComponent ---------------------------------------------
    if (registry.HasComponent<Freely::SpriteRendererComponent>(entity)) {
        ImGui::PushID("SpriteRendererComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::SpriteRendererComponent).hash_code(), treeNodeFlags, "Sprite Renderer");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& spr = registry.GetComponent<Freely::SpriteRendererComponent>(entity);
            ImGui::ColorEdit4("Color",    glm::value_ptr(spr.Color));
            char buf[256]; memset(buf,0,sizeof(buf)); strncpy(buf,spr.TexturePath.c_str(),sizeof(buf)-1);
            if (ImGui::InputText("Texture",buf,sizeof(buf))) spr.TexturePath=buf;
            ImGui::DragFloat2("Tiling",  glm::value_ptr(spr.TilingFactor), 0.1f);
            ImGui::DragFloat2("Offset",  glm::value_ptr(spr.Offset), 0.01f);
            ImGui::Checkbox("Flip X",   &spr.FlipX); ImGui::SameLine();
            ImGui::Checkbox("Flip Y",   &spr.FlipY);
            ImGui::DragInt("Sorting Layer", &spr.SortingLayer);
            ImGui::DragInt("Order in Layer",&spr.OrderInLayer);
            ImGui::Checkbox("Visible",  &spr.Visible);
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::SpriteRendererComponent>(entity);
        ImGui::PopID();
    }

    // --- Text2DComponent -----------------------------------------------------
    if (registry.HasComponent<Freely::Text2DComponent>(entity)) {
        ImGui::PushID("Text2DComponent");
        bool removeComponent = false;
        bool open = ImGui::TreeNodeEx((void*)typeid(Freely::Text2DComponent).hash_code(), treeNodeFlags, "Text 2D");
        if (ImGui::BeginPopupContextItem()) { if (ImGui::MenuItem("Remove Component")) removeComponent = true; ImGui::EndPopup(); }
        if (open) {
            auto& txt = registry.GetComponent<Freely::Text2DComponent>(entity);
            char buf[1024]; memset(buf,0,sizeof(buf)); strncpy(buf,txt.Text.c_str(),sizeof(buf)-1);
            if (ImGui::InputTextMultiline("Text",buf,sizeof(buf),{0,60})) txt.Text=buf;
            memset(buf,0,sizeof(buf)); strncpy(buf,txt.FontPath.c_str(),sizeof(buf)-1);
            if (ImGui::InputText("Font",buf,sizeof(buf))) txt.FontPath=buf;
            ImGui::ColorEdit4("Color",    glm::value_ptr(txt.Color));
            ImGui::DragFloat("Font Size", &txt.FontSize,   0.5f, 4.0f, 256.0f);
            ImGui::DragFloat("Kerning",   &txt.Kerning,    0.1f);
            ImGui::DragFloat("Line Spacing",&txt.LineSpacing,0.05f, 0.5f, 5.0f);
            ImGui::SliderFloat("Align H", &txt.AlignH, 0.0f, 1.0f);
            ImGui::Checkbox("Visible", &txt.Visible);
            ImGui::TreePop();
        }
        if (removeComponent) registry.RemoveComponent<Freely::Text2DComponent>(entity);
        ImGui::PopID();
    }

}

} // namespace FreelyEditor
