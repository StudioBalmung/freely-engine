#include "Freely/ECS/Scene.h"
#include "Freely/Scripting/LuaEngine.h"
#include "Freely/ECS/SceneGraph.h"
#include "Freely/ECS/Systems/RenderSystem.h"

namespace Freely {

void Scene::Init() {
    m_SystemScheduler.AddSystem<RenderSystem>();
    m_SystemScheduler.Initialize(*this);
}

void Scene::Start() {
    m_IsPlaying = true;
    m_SystemScheduler.Initialize(*this);
    LuaEngine::OnSceneStart(*this);
}

void Scene::Stop() {
    LuaEngine::OnSceneStop(*this);
    m_SystemScheduler.Shutdown(*this);
    m_IsPlaying = false;
}

void Scene::Update(float dt) {
    if (m_IsPlaying) {
        m_SystemScheduler.Update(*this, dt);
        LuaEngine::OnSceneUpdate(*this, dt);
    }
    SceneGraph::UpdateTransforms(m_Registry);
}

void Scene::FixedUpdate(float fixedDt) {
    if (m_IsPlaying) {
        m_SystemScheduler.FixedUpdate(*this, fixedDt);
    }
}

void Scene::LateUpdate(float dt) {
    if (m_IsPlaying) {
        m_SystemScheduler.LateUpdate(*this, dt);
    }
}

entt::entity Scene::CreateEntity(const std::string& name) {
    return m_Registry.CreateEntity(name);
}

entt::entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
    return m_Registry.CreateEntityWithUUID(uuid, name);
}

void Scene::DestroyEntity(entt::entity entity) {
    m_Registry.DestroyEntity(entity);
}

entt::entity Scene::FindEntityByName(const std::string& name) {
    return m_Registry.FindByName(name);
}

entt::entity Scene::FindEntityByUUID(UUID uuid) {
    return m_Registry.FindByUUID(uuid);
}

entt::entity Scene::DuplicateEntity(entt::entity entity) {
    if (!m_Registry.IsValid(entity)) return entt::null;
    
    std::string name = m_Registry.GetComponent<TagComponent>(entity).Name;
    entt::entity newEntity = CreateEntity(name + " (Copy)");
    CopyEntity(entity, newEntity);
    return newEntity;
}

void Scene::CopyEntity(entt::entity src, entt::entity dst) {
    // Copy TransformComponent
    if (m_Registry.HasComponent<TransformComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<TransformComponent>(src);
        auto& dstComp = m_Registry.GetComponent<TransformComponent>(dst);
        dstComp.Position = srcComp.Position;
        dstComp.Rotation = srcComp.Rotation;
        dstComp.Scale = srcComp.Scale;
    }

    // Copy MeshRendererComponent
    if (m_Registry.HasComponent<MeshRendererComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<MeshRendererComponent>(src);
        auto& dstComp = m_Registry.AddComponent<MeshRendererComponent>(dst);
        dstComp = srcComp;
    }

    // Copy CameraComponent
    if (m_Registry.HasComponent<CameraComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<CameraComponent>(src);
        auto& dstComp = m_Registry.AddComponent<CameraComponent>(dst);
        dstComp = srcComp;
        dstComp.Primary = false; // duplicated camera shouldn't automatically be primary
    }

    // Copy LightComponent
    if (m_Registry.HasComponent<LightComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<LightComponent>(src);
        auto& dstComp = m_Registry.AddComponent<LightComponent>(dst);
        dstComp = srcComp;
    }

    // Copy RigidBodyComponent
    if (m_Registry.HasComponent<RigidBodyComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<RigidBodyComponent>(src);
        auto& dstComp = m_Registry.AddComponent<RigidBodyComponent>(dst);
        dstComp = srcComp;
        dstComp.RuntimeBodyHandle = 0; // reset runtime handle
    }

    // Copy ColliderComponent
    if (m_Registry.HasComponent<ColliderComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<ColliderComponent>(src);
        auto& dstComp = m_Registry.AddComponent<ColliderComponent>(dst);
        dstComp = srcComp;
        dstComp.RuntimeShapeHandle = 0; // reset runtime handle
    }
    
    // Copy ScriptComponent
    if (m_Registry.HasComponent<ScriptComponent>(src)) {
        auto& srcComp = m_Registry.GetComponent<ScriptComponent>(src);
        auto& dstComp = m_Registry.AddComponent<ScriptComponent>(dst);
        dstComp = srcComp;
        dstComp.RuntimeInstance = nullptr; // reset runtime instance
    }


    // --- 2D Pipeline components -------------------------------------------
    if (m_Registry.HasComponent<SpriteRendererComponent>(src)) {
        auto comp = m_Registry.GetComponent<SpriteRendererComponent>(src);
        comp.TextureHandle = 0;
        m_Registry.AddComponent<SpriteRendererComponent>(dst) = comp;
    }
    if (m_Registry.HasComponent<Text2DComponent>(src)) {
        auto comp = m_Registry.GetComponent<Text2DComponent>(src);
        comp.FontHandle = 0;
        m_Registry.AddComponent<Text2DComponent>(dst) = comp;
    }
    if (m_Registry.HasComponent<Camera2DComponent>(src)) {
        auto comp = m_Registry.GetComponent<Camera2DComponent>(src);
        comp.Primary = false;
        m_Registry.AddComponent<Camera2DComponent>(dst) = comp;
    }
    if (m_Registry.HasComponent<MaterialComponent>(src)) {
        m_Registry.AddComponent<MaterialComponent>(dst) =
            m_Registry.GetComponent<MaterialComponent>(src);
    }
    if (m_Registry.HasComponent<AudioSourceComponent>(src)) {
        auto comp = m_Registry.GetComponent<AudioSourceComponent>(src);
        comp.RuntimeSourceHandle = 0;
        m_Registry.AddComponent<AudioSourceComponent>(dst) = comp;
    }
    if (m_Registry.HasComponent<SkyboxComponent>(src)) {
        m_Registry.AddComponent<SkyboxComponent>(dst) =
            m_Registry.GetComponent<SkyboxComponent>(src);
    }
    if (m_Registry.HasComponent<MeshFilterComponent>(src)) {
        m_Registry.AddComponent<MeshFilterComponent>(dst) =
            m_Registry.GetComponent<MeshFilterComponent>(src);
    }

    // Duplicate children
    auto& rel = m_Registry.GetComponent<RelationshipComponent>(src);
    entt::entity child = rel.FirstChild;
    while (child != entt::null) {
        entt::entity newChild = DuplicateEntity(child);
        m_Registry.SetParent(newChild, dst);
        child = m_Registry.GetComponent<RelationshipComponent>(child).NextSibling;
    }
}

} // namespace Freely
