#include "Editor/Panels/SceneHierarchyPanel.h"

#include <imgui.h>
#include <Freely/ECS/Components.h>
#include <Freely/ECS/SceneGraph.h>
#include "Editor/Commands/EntityCommands.h"

namespace FreelyEditor {

namespace {
    constexpr const char* kHierarchyDragDropPayload = "FREELY_HIERARCHY_ENTITY";

    // ── "Add Component" popup shown in Inspector context ─────────────────────
    void DrawAddComponentPopup(entt::entity entity, Freely::Registry& registry) {
        if (!ImGui::BeginPopup("AddComponentPopup")) return;

        ImGui::SeparatorText("3D");
        if (!registry.HasComponent<Freely::MeshRendererComponent>(entity) && ImGui::MenuItem("Mesh Renderer"))
            { registry.AddComponent<Freely::MeshRendererComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::MeshFilterComponent>(entity) && ImGui::MenuItem("Mesh Filter"))
            { registry.AddComponent<Freely::MeshFilterComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::MaterialComponent>(entity) && ImGui::MenuItem("Material"))
            { registry.AddComponent<Freely::MaterialComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::LightComponent>(entity) && ImGui::MenuItem("Light"))
            { registry.AddComponent<Freely::LightComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::CameraComponent>(entity) && ImGui::MenuItem("Camera"))
            { registry.AddComponent<Freely::CameraComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::SkyboxComponent>(entity) && ImGui::MenuItem("Skybox"))
            { registry.AddComponent<Freely::SkyboxComponent>(entity); ImGui::CloseCurrentPopup(); }

        ImGui::SeparatorText("2D");
        if (!registry.HasComponent<Freely::SpriteRendererComponent>(entity) && ImGui::MenuItem("Sprite Renderer"))
            { registry.AddComponent<Freely::SpriteRendererComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::Text2DComponent>(entity) && ImGui::MenuItem("Text 2D"))
            { registry.AddComponent<Freely::Text2DComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::Camera2DComponent>(entity) && ImGui::MenuItem("Camera 2D"))
            { registry.AddComponent<Freely::Camera2DComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::SpriteAnimatorComponent>(entity) && ImGui::MenuItem("Sprite Animator"))
            { registry.AddComponent<Freely::SpriteAnimatorComponent>(entity); ImGui::CloseCurrentPopup(); }

        ImGui::SeparatorText("Physics");
        if (!registry.HasComponent<Freely::RigidBodyComponent>(entity) && ImGui::MenuItem("Rigidbody"))
            { registry.AddComponent<Freely::RigidBodyComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::ColliderComponent>(entity) && ImGui::MenuItem("Box Collider"))
            { auto& c = registry.AddComponent<Freely::ColliderComponent>(entity); c.Shape = Freely::ColliderShape::Box; ImGui::CloseCurrentPopup(); }
        if (ImGui::MenuItem("Sphere Collider") && !registry.HasComponent<Freely::ColliderComponent>(entity))
            { auto& c = registry.AddComponent<Freely::ColliderComponent>(entity); c.Shape = Freely::ColliderShape::Sphere; ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::Rigidbody2DComponent>(entity) && ImGui::MenuItem("Rigidbody 2D"))
            { registry.AddComponent<Freely::Rigidbody2DComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::BoxCollider2DComponent>(entity) && ImGui::MenuItem("Box Collider 2D"))
            { registry.AddComponent<Freely::BoxCollider2DComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::CircleCollider2DComponent>(entity) && ImGui::MenuItem("Circle Collider 2D"))
            { registry.AddComponent<Freely::CircleCollider2DComponent>(entity); ImGui::CloseCurrentPopup(); }

        ImGui::SeparatorText("Audio / Scripting");
        if (!registry.HasComponent<Freely::AudioSourceComponent>(entity) && ImGui::MenuItem("Audio Source"))
            { registry.AddComponent<Freely::AudioSourceComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::ScriptComponent>(entity) && ImGui::MenuItem("Script"))
            { registry.AddComponent<Freely::ScriptComponent>(entity); ImGui::CloseCurrentPopup(); }
        if (!registry.HasComponent<Freely::ParticleEmitterComponent>(entity) && ImGui::MenuItem("Particle Emitter"))
            { registry.AddComponent<Freely::ParticleEmitterComponent>(entity); ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }
}

// ─── Constructor ──────────────────────────────────────────────────────────────
SceneHierarchyPanel::SceneHierarchyPanel(EditorContext* context)
    : m_Context(context) {}

// ─── OnImGuiRender ────────────────────────────────────────────────────────────
void SceneHierarchyPanel::OnImGuiRender() {
    ImGui::Begin("Scene Hierarchy");

    if (m_Context->ActiveScene) {
        // Draw all root entities
        m_Context->ActiveScene->GetRegistry().ForEachRootEntity([&](entt::entity entity) {
            DrawEntityNode(entity);
        });

        // Click on blank area → deselect
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            m_Context->SelectedEntity = entt::null;

        // Right-click on blank space → scene-level popup
        if (ImGui::BeginPopupContextWindow("HierarchyPopup",
                ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {

            ImGui::SeparatorText("Scene");
            if (ImGui::MenuItem("Create Empty Entity")) {
                auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, "Empty Entity");
                auto* ptr = cmd.get();
                m_Context->ExecuteCommand(std::move(cmd));
                m_Context->SelectedEntity = ptr->GetCreatedEntity();
            }
            if (ImGui::BeginMenu("3D Object")) {
                auto spawnPrimitive = [&](const char* name, Freely::PrimitiveMeshType prim) {
                    if (ImGui::MenuItem(name)) {
                        auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, name);
                        auto* ptr = cmd.get();
                        m_Context->ExecuteCommand(std::move(cmd));
                        auto e = ptr->GetCreatedEntity();
                        auto& reg = m_Context->ActiveScene->GetRegistry();
                        auto& mf = reg.AddComponent<Freely::MeshFilterComponent>(e);
                        mf.PrimitiveType = prim;
                        reg.AddComponent<Freely::MeshRendererComponent>(e);
                        reg.AddComponent<Freely::MaterialComponent>(e);
                        m_Context->SelectedEntity = e;
                    }
                };
                spawnPrimitive("Cube",   Freely::PrimitiveMeshType::Cube);
                spawnPrimitive("Sphere", Freely::PrimitiveMeshType::Sphere);
                spawnPrimitive("Plane",  Freely::PrimitiveMeshType::Plane);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("2D Object")) {
                if (ImGui::MenuItem("Sprite")) {
                    auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, "Sprite");
                    auto* ptr = cmd.get();
                    m_Context->ExecuteCommand(std::move(cmd));
                    m_Context->ActiveScene->GetRegistry().AddComponent<Freely::SpriteRendererComponent>(ptr->GetCreatedEntity());
                    m_Context->SelectedEntity = ptr->GetCreatedEntity();
                }
                if (ImGui::MenuItem("Text")) {
                    auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, "Text");
                    auto* ptr = cmd.get();
                    m_Context->ExecuteCommand(std::move(cmd));
                    auto& t = m_Context->ActiveScene->GetRegistry().AddComponent<Freely::Text2DComponent>(ptr->GetCreatedEntity());
                    t.Text = "Hello, World!";
                    m_Context->SelectedEntity = ptr->GetCreatedEntity();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Directional Light")) {
                auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, "Directional Light");
                auto* ptr = cmd.get();
                m_Context->ExecuteCommand(std::move(cmd));
                auto& lc = m_Context->ActiveScene->GetRegistry().AddComponent<Freely::LightComponent>(ptr->GetCreatedEntity());
                lc.Type = Freely::LightType::Directional;
                m_Context->SelectedEntity = ptr->GetCreatedEntity();
            }
            if (ImGui::MenuItem("Camera")) {
                auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, "Camera");
                auto* ptr = cmd.get();
                m_Context->ExecuteCommand(std::move(cmd));
                m_Context->ActiveScene->GetRegistry().AddComponent<Freely::CameraComponent>(ptr->GetCreatedEntity());
                m_Context->SelectedEntity = ptr->GetCreatedEntity();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

// ─── DrawEntityNode ───────────────────────────────────────────────────────────
void SceneHierarchyPanel::DrawEntityNode(entt::entity entity) {
    auto& registry = m_Context->ActiveScene->GetRegistry();
    auto& tag      = registry.GetComponent<Freely::TagComponent>(entity);
    auto& rel      = registry.GetComponent<Freely::RelationshipComponent>(entity);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ((m_Context->SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0);
    if (rel.FirstChild == entt::null)
        flags |= ImGuiTreeNodeFlags_Leaf;

    // Inactive entities shown dimmed
    if (!tag.Active) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Name.c_str());
    if (!tag.Active) ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        m_Context->SelectedEntity = entity;

    // Drag source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload(kHierarchyDragDropPayload, &entity, sizeof(entt::entity));
        ImGui::Text("%s", tag.Name.c_str());
        ImGui::EndDragDropSource();
    }
    HandleDragDropReparent(entity);

    // Right-click context menu on a specific entity
    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem(tag.Active ? "Deactivate" : "Activate"))
            tag.Active = !tag.Active;
        ImGui::Separator();
        if (ImGui::MenuItem("Duplicate Entity")) {
            auto dup = m_Context->ActiveScene->DuplicateEntity(entity);
            m_Context->SelectedEntity = dup;
        }
        if (ImGui::MenuItem("Create Child Entity")) {
            auto cmd = std::make_unique<CreateEntityCommand>(m_Context->ActiveScene, "Empty Entity", entity);
            m_Context->ExecuteCommand(std::move(cmd));
        }
        ImGui::Separator();
        // ── Add Component inline ─────────────────────────────────────────
        if (ImGui::BeginMenu("Add Component")) {
            DrawAddComponentPopup(entity, registry); // reuse helper but inline
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete Entity")) entityDeleted = true;
        ImGui::EndPopup();
    }

    // Recurse into children
    if (opened) {
        entt::entity child = rel.FirstChild;
        while (child != entt::null) {
            auto next = registry.GetComponent<Freely::RelationshipComponent>(child).NextSibling;
            DrawEntityNode(child);
            child = next;
        }
        ImGui::TreePop();
    }

    if (entityDeleted) {
        if (m_Context->SelectedEntity == entity)
            m_Context->SelectedEntity = entt::null;
        m_Context->ExecuteCommand(std::make_unique<DeleteEntityCommand>(m_Context->ActiveScene, entity));
    }
}

// ─── Drag-drop reparent ───────────────────────────────────────────────────────
void SceneHierarchyPanel::HandleDragDropReparent(entt::entity targetEntity) {
    if (!ImGui::BeginDragDropTarget()) return;
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kHierarchyDragDropPayload)) {
        entt::entity dragged = *static_cast<const entt::entity*>(payload->Data);
        auto& reg = m_Context->ActiveScene->GetRegistry();
        bool same  = dragged == targetEntity;
        bool cycle = Freely::SceneGraph::IsAncestor(reg, targetEntity, dragged);
        if (!same && !cycle)
            reg.SetParent(dragged, targetEntity);
    }
    ImGui::EndDragDropTarget();
}

} // namespace FreelyEditor
