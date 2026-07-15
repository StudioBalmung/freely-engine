#pragma once

#include "Registry.h"
#include "System.h"
#include <string>

namespace Freely {

class Scene {
public:
    Scene() = default;
    Scene(const std::string& name) : m_Name(name) {}
    ~Scene() { m_SystemScheduler.Shutdown(*this); }

    void Init();
    void Start();
    void Stop();

    void Update(float dt);
    void FixedUpdate(float fixedDt);
    void LateUpdate(float dt);

    entt::entity CreateEntity(const std::string& name = "Entity");
    entt::entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
    void DestroyEntity(entt::entity entity);

    entt::entity DuplicateEntity(entt::entity entity);

    entt::entity FindEntityByName(const std::string& name);
    entt::entity FindEntityByUUID(UUID uuid);

    Registry& GetRegistry() { return m_Registry; }
    const Registry& GetRegistry() const { return m_Registry; }

    SystemScheduler& GetSystemScheduler() { return m_SystemScheduler; }

    bool IsPlaying() const { return m_IsPlaying; }

private:
    void CopyEntity(entt::entity src, entt::entity dst);

    Registry m_Registry;
    SystemScheduler m_SystemScheduler;
    bool m_IsPlaying = false;
    std::string m_Name;



} // namespace Freely
