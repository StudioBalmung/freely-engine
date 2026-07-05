#pragma once

#include <memory>
#include <string>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Freely/ECS/Scene.h>

#include "Editor/Commands/ICommand.h"

namespace FreelyEditor {

// Creates a new entity on Execute, destroys it again on Undo. The entity is
// re-created with the same UUID on Redo so any references taken elsewhere
// (selection, hierarchy drag state) keep working across an undo/redo cycle.
class CreateEntityCommand : public ICommand {
public:
    CreateEntityCommand(std::shared_ptr<Freely::Scene> scene, std::string name, entt::entity parent = entt::null);

    void Execute() override;
    void Undo() override;
    const char* GetName() const override { return "Create Entity"; }

    entt::entity GetCreatedEntity() const { return m_CreatedEntity; }

private:
    std::shared_ptr<Freely::Scene> m_Scene;
    std::string m_Name;
    entt::entity m_Parent;
    Freely::UUID m_UUID;
    entt::entity m_CreatedEntity = entt::null;
    bool m_HasRunOnce = false;
};

// Captures an entity's full component set on construction so Undo can fully
// restore it after Execute destroys it. Only restores the components this
// editor knows how to edit directly; anything else attached at runtime by
// scripts is expected to re-attach itself when the entity becomes valid again.
class DeleteEntityCommand : public ICommand {
public:
    DeleteEntityCommand(std::shared_ptr<Freely::Scene> scene, entt::entity entity);

    void Execute() override;
    void Undo() override;
    const char* GetName() const override { return "Delete Entity"; }

private:
    std::shared_ptr<Freely::Scene> m_Scene;
    Freely::UUID m_UUID;
    entt::entity m_RestoredEntity = entt::null;

    std::string m_Name;
    std::string m_Tag;
    int m_Layer = 0;
    bool m_Active = true;

    Freely::TransformComponent m_Transform;

    bool m_HadMeshRenderer = false;
    Freely::MeshRendererComponent m_MeshRenderer;

    bool m_HadMeshFilter = false;
    Freely::MeshFilterComponent m_MeshFilter;

    bool m_HadCamera = false;
    Freely::CameraComponent m_Camera;

    bool m_HadLight = false;
    Freely::LightComponent m_Light;

    bool m_HadMaterial = false;
    Freely::MaterialComponent m_Material;

    bool m_HadRigidBody = false;
    Freely::RigidBodyComponent m_RigidBody;

    bool m_HadCollider = false;
    Freely::ColliderComponent m_Collider;
};

// Records a before/after transform pair for a single entity. Successive
// drags of the same gizmo on the same entity merge into one command so a
// single Ctrl+Z undoes the whole drag, not one tiny step per mouse-move event.
class TransformEntityCommand : public ICommand {
public:
    TransformEntityCommand(std::shared_ptr<Freely::Scene> scene, entt::entity entity,
                            const glm::vec3& beforePosition, const glm::quat& beforeRotation, const glm::vec3& beforeScale,
                            const glm::vec3& afterPosition, const glm::quat& afterRotation, const glm::vec3& afterScale);

    void Execute() override;
    void Undo() override;
    const char* GetName() const override { return "Transform Entity"; }

    bool CanMerge(const ICommand& other) const override;
    void MergeWith(const ICommand& other) override;

private:
    void Apply(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

    std::shared_ptr<Freely::Scene> m_Scene;
    Freely::UUID m_UUID;

    glm::vec3 m_BeforePosition;
    glm::quat m_BeforeRotation;
    glm::vec3 m_BeforeScale;

    glm::vec3 m_AfterPosition;
    glm::quat m_AfterRotation;
    glm::vec3 m_AfterScale;
};

} // namespace FreelyEditor
