#include "Editor/Commands/EntityCommands.h"

#include <Freely/ECS/Components.h>

namespace FreelyEditor {

// ─── CreateEntityCommand ────────────────────────────────────────────────────

CreateEntityCommand::CreateEntityCommand(std::shared_ptr<Freely::Scene> scene, std::string name, entt::entity parent)
    : m_Scene(std::move(scene))
    , m_Name(std::move(name))
    , m_Parent(parent)
    , m_UUID(Freely::UUID::Generate())
{
}

void CreateEntityCommand::Execute() {
    if (!m_Scene) return;

    if (!m_HasRunOnce) {
        m_CreatedEntity = m_Scene->GetRegistry().CreateEntityWithUUID(m_UUID, m_Name);
        m_HasRunOnce = true;
    } else {
        // Redo path: re-create with the same UUID so anything that captured it earlier still resolves.
        m_CreatedEntity = m_Scene->GetRegistry().CreateEntityWithUUID(m_UUID, m_Name);
    }

    if (m_Parent != entt::null && m_Scene->GetRegistry().IsValid(m_Parent)) {
        m_Scene->GetRegistry().SetParent(m_CreatedEntity, m_Parent);
    }
}

void CreateEntityCommand::Undo() {
    if (!m_Scene) return;
    if (m_CreatedEntity != entt::null && m_Scene->GetRegistry().IsValid(m_CreatedEntity)) {
        m_Scene->GetRegistry().DestroyEntity(m_CreatedEntity);
    }
    m_CreatedEntity = entt::null;
}

// ─── DeleteEntityCommand ────────────────────────────────────────────────────

DeleteEntityCommand::DeleteEntityCommand(std::shared_ptr<Freely::Scene> scene, entt::entity entity)
    : m_Scene(std::move(scene))
{
    auto& registry = m_Scene->GetRegistry();

    m_UUID = registry.GetComponent<Freely::IDComponent>(entity).ID;

    const auto& tag = registry.GetComponent<Freely::TagComponent>(entity);
    m_Name = tag.Name;
    m_Tag = tag.Tag;
    m_Layer = tag.Layer;
    m_Active = tag.Active;

    m_Transform = registry.GetComponent<Freely::TransformComponent>(entity);

    if (auto* c = registry.TryGetComponent<Freely::MeshRendererComponent>(entity)) { m_HadMeshRenderer = true; m_MeshRenderer = *c; }
    if (auto* c = registry.TryGetComponent<Freely::MeshFilterComponent>(entity)) { m_HadMeshFilter = true; m_MeshFilter = *c; }
    if (auto* c = registry.TryGetComponent<Freely::CameraComponent>(entity)) { m_HadCamera = true; m_Camera = *c; }
    if (auto* c = registry.TryGetComponent<Freely::LightComponent>(entity)) { m_HadLight = true; m_Light = *c; }
    if (auto* c = registry.TryGetComponent<Freely::MaterialComponent>(entity)) { m_HadMaterial = true; m_Material = *c; }
    if (auto* c = registry.TryGetComponent<Freely::RigidBodyComponent>(entity)) { m_HadRigidBody = true; m_RigidBody = *c; }
    if (auto* c = registry.TryGetComponent<Freely::ColliderComponent>(entity)) { m_HadCollider = true; m_Collider = *c; }
}

void DeleteEntityCommand::Execute() {
    if (!m_Scene) return;
    entt::entity entity = m_Scene->FindEntityByUUID(m_UUID);
    if (entity != entt::null) {
        m_Scene->GetRegistry().DestroyEntity(entity);
    }
    m_RestoredEntity = entt::null;
}

void DeleteEntityCommand::Undo() {
    if (!m_Scene) return;
    auto& registry = m_Scene->GetRegistry();

    m_RestoredEntity = registry.CreateEntityWithUUID(m_UUID, m_Name);

    auto& tag = registry.GetComponent<Freely::TagComponent>(m_RestoredEntity);
    tag.Tag = m_Tag;
    tag.Layer = m_Layer;
    tag.Active = m_Active;

    registry.GetComponent<Freely::TransformComponent>(m_RestoredEntity) = m_Transform;

    if (m_HadMeshRenderer) registry.AddOrReplaceComponent<Freely::MeshRendererComponent>(m_RestoredEntity, m_MeshRenderer);
    if (m_HadMeshFilter) registry.AddOrReplaceComponent<Freely::MeshFilterComponent>(m_RestoredEntity, m_MeshFilter);
    if (m_HadCamera) registry.AddOrReplaceComponent<Freely::CameraComponent>(m_RestoredEntity, m_Camera);
    if (m_HadLight) registry.AddOrReplaceComponent<Freely::LightComponent>(m_RestoredEntity, m_Light);
    if (m_HadMaterial) registry.AddOrReplaceComponent<Freely::MaterialComponent>(m_RestoredEntity, m_Material);
    if (m_HadRigidBody) registry.AddOrReplaceComponent<Freely::RigidBodyComponent>(m_RestoredEntity, m_RigidBody);
    if (m_HadCollider) registry.AddOrReplaceComponent<Freely::ColliderComponent>(m_RestoredEntity, m_Collider);
}

// ─── TransformEntityCommand ─────────────────────────────────────────────────

TransformEntityCommand::TransformEntityCommand(std::shared_ptr<Freely::Scene> scene, entt::entity entity,
                                                const glm::vec3& beforePosition, const glm::quat& beforeRotation, const glm::vec3& beforeScale,
                                                const glm::vec3& afterPosition, const glm::quat& afterRotation, const glm::vec3& afterScale)
    : m_Scene(std::move(scene))
    , m_BeforePosition(beforePosition)
    , m_BeforeRotation(beforeRotation)
    , m_BeforeScale(beforeScale)
    , m_AfterPosition(afterPosition)
    , m_AfterRotation(afterRotation)
    , m_AfterScale(afterScale)
{
    m_UUID = m_Scene->GetRegistry().GetComponent<Freely::IDComponent>(entity).ID;
}

void TransformEntityCommand::Apply(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
    if (!m_Scene) return;
    entt::entity entity = m_Scene->FindEntityByUUID(m_UUID);
    if (entity == entt::null) return;

    auto& transform = m_Scene->GetRegistry().GetComponent<Freely::TransformComponent>(entity);
    transform.Position = position;
    transform.Rotation = rotation;
    transform.Scale = scale;
    transform.Dirty = true;
}

void TransformEntityCommand::Execute() {
    Apply(m_AfterPosition, m_AfterRotation, m_AfterScale);
}

void TransformEntityCommand::Undo() {
    Apply(m_BeforePosition, m_BeforeRotation, m_BeforeScale);
}

bool TransformEntityCommand::CanMerge(const ICommand& other) const {
    const auto* otherTransform = dynamic_cast<const TransformEntityCommand*>(&other);
    return otherTransform != nullptr && otherTransform->m_UUID == m_UUID;
}

void TransformEntityCommand::MergeWith(const ICommand& other) {
    const auto& otherTransform = static_cast<const TransformEntityCommand&>(other);
    // Keep this command's "before" state (the start of the drag) and adopt the
    // incoming command's "after" state (the latest position in the drag).
    m_AfterPosition = otherTransform.m_AfterPosition;
    m_AfterRotation = otherTransform.m_AfterRotation;
    m_AfterScale = otherTransform.m_AfterScale;
}

} // namespace FreelyEditor
