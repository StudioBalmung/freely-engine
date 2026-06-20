#pragma once

// Freely Engine 2.0.0 — ECS Registry
// Thin wrapper around entt::registry with Freely-specific helpers.

#include "Components.h"
#include "UUID.h"

#include <entt/entt.hpp>
#include <unordered_map>
#include <functional>
#include <string>

namespace Freely {

class Registry {
public:
    Registry() = default;
    ~Registry() = default;

    // Non-copyable, movable
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    Registry(Registry&&) noexcept = default;
    Registry& operator=(Registry&&) noexcept = default;

    // ─── Entity lifecycle ────────────────────────────────────────────────

    /// Create an entity with default IDComponent, TagComponent, TransformComponent, RelationshipComponent.
    entt::entity CreateEntity(const std::string& name = "Entity") {
        entt::entity e = m_Registry.create();

        auto& id = m_Registry.emplace<IDComponent>(e);
        m_UUIDMap[id.ID] = e;

        auto& tag = m_Registry.emplace<TagComponent>(e);
        tag.Name = name;

        m_Registry.emplace<TransformComponent>(e);
        m_Registry.emplace<RelationshipComponent>(e);

        return e;
    }

    /// Create an entity with a specific UUID (used during deserialization).
    entt::entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity") {
        entt::entity e = m_Registry.create();

        auto& id = m_Registry.emplace<IDComponent>(e, uuid);
        m_UUIDMap[id.ID] = e;

        auto& tag = m_Registry.emplace<TagComponent>(e);
        tag.Name = name;

        m_Registry.emplace<TransformComponent>(e);
        m_Registry.emplace<RelationshipComponent>(e);

        return e;
    }

    /// Destroy an entity and remove it from the UUID map.
    void DestroyEntity(entt::entity entity) {
        if (m_Registry.valid(entity)) {
            // Remove children recursively
            if (auto* rel = m_Registry.try_get<RelationshipComponent>(entity)) {
                auto child = rel->FirstChild;
                while (child != entt::null) {
                    auto next = entt::null;
                    if (auto* childRel = m_Registry.try_get<RelationshipComponent>(child))
                        next = childRel->NextSibling;
                    DestroyEntity(child);
                    child = next;
                }
            }

            // Remove from UUID map
            if (auto* id = m_Registry.try_get<IDComponent>(entity))
                m_UUIDMap.erase(id->ID);

            // Detach from parent
            DetachFromParent(entity);

            m_Registry.destroy(entity);
        }
    }

    /// Check if an entity is valid.
    bool IsValid(entt::entity entity) const {
        return m_Registry.valid(entity);
    }

    // ─── Component access ────────────────────────────────────────────────

    template<typename T, typename... Args>
    T& AddComponent(entt::entity entity, Args&&... args) {
        return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T& AddOrReplaceComponent(entt::entity entity, Args&&... args) {
        return m_Registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent(entt::entity entity) {
        return m_Registry.get<T>(entity);
    }

    template<typename T>
    const T& GetComponent(entt::entity entity) const {
        return m_Registry.get<T>(entity);
    }

    template<typename T>
    T* TryGetComponent(entt::entity entity) {
        return m_Registry.try_get<T>(entity);
    }

    template<typename T>
    const T* TryGetComponent(entt::entity entity) const {
        return m_Registry.try_get<T>(entity);
    }

    template<typename T>
    bool HasComponent(entt::entity entity) const {
        return m_Registry.all_of<T>(entity);
    }

    template<typename T>
    void RemoveComponent(entt::entity entity) {
        m_Registry.remove<T>(entity);
    }

    // ─── Views ───────────────────────────────────────────────────────────

    template<typename... Components>
    auto View() {
        return m_Registry.view<Components...>();
    }

    template<typename... Components>
    auto View() const {
        return m_Registry.view<Components...>();
    }

    // ─── Entity queries ──────────────────────────────────────────────────

    /// Find entity by UUID. Returns entt::null if not found.
    entt::entity FindByUUID(UUID uuid) const {
        auto it = m_UUIDMap.find(uuid);
        return (it != m_UUIDMap.end()) ? it->second : entt::null;
    }

    /// Find first entity by name. Returns entt::null if not found.
    entt::entity FindByName(const std::string& name) const {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view) {
            if (view.get<TagComponent>(entity).Name == name)
                return entity;
        }
        return entt::null;
    }

    /// Find all entities by tag.
    std::vector<entt::entity> FindByTag(const std::string& tag) const {
        std::vector<entt::entity> results;
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view) {
            if (view.get<TagComponent>(entity).Tag == tag)
                results.push_back(entity);
        }
        return results;
    }

    /// Iterate over all entities (top-level: those without a parent).
    void ForEachRootEntity(const std::function<void(entt::entity)>& fn) const {
        auto view = m_Registry.view<RelationshipComponent>();
        for (auto entity : view) {
            auto& rel = view.get<RelationshipComponent>(entity);
            if (rel.Parent == entt::null)
                fn(entity);
        }
    }

    /// Iterate over all entities.
    void ForEachEntity(const std::function<void(entt::entity)>& fn) const {
        m_Registry.each([&](entt::entity e) { fn(e); });
    }

    /// Get entity count.
    size_t GetEntityCount() const {
        return m_Registry.storage<entt::entity>().in_use();
    }

    /// Clear all entities and reset UUID map.
    void Clear() {
        m_Registry.clear();
        m_UUIDMap.clear();
    }

    // ─── Hierarchy ───────────────────────────────────────────────────────

    /// Set parent-child relationship.
    void SetParent(entt::entity child, entt::entity parent) {
        if (child == parent || child == entt::null) return;
        if (!m_Registry.valid(child)) return;

        // Detach from current parent first
        DetachFromParent(child);

        if (parent == entt::null) return;
        if (!m_Registry.valid(parent)) return;

        auto& childRel  = m_Registry.get<RelationshipComponent>(child);
        auto& parentRel = m_Registry.get<RelationshipComponent>(parent);

        childRel.Parent = parent;
        childRel.NextSibling = parentRel.FirstChild;
        childRel.PrevSibling = entt::null;

        if (parentRel.FirstChild != entt::null) {
            auto& firstChildRel = m_Registry.get<RelationshipComponent>(parentRel.FirstChild);
            firstChildRel.PrevSibling = child;
        }

        parentRel.FirstChild = child;
        parentRel.ChildCount++;
    }

    /// Detach entity from its parent.
    void DetachFromParent(entt::entity entity) {
        if (!m_Registry.valid(entity)) return;

        auto& rel = m_Registry.get<RelationshipComponent>(entity);
        if (rel.Parent == entt::null) return;

        auto& parentRel = m_Registry.get<RelationshipComponent>(rel.Parent);

        // Fix sibling links
        if (rel.PrevSibling != entt::null) {
            auto& prevRel = m_Registry.get<RelationshipComponent>(rel.PrevSibling);
            prevRel.NextSibling = rel.NextSibling;
        } else {
            // This was the first child
            parentRel.FirstChild = rel.NextSibling;
        }

        if (rel.NextSibling != entt::null) {
            auto& nextRel = m_Registry.get<RelationshipComponent>(rel.NextSibling);
            nextRel.PrevSibling = rel.PrevSibling;
        }

        parentRel.ChildCount--;
        rel.Parent = entt::null;
        rel.PrevSibling = entt::null;
        rel.NextSibling = entt::null;
    }

    /// Iterate children of an entity.
    void ForEachChild(entt::entity parent, const std::function<void(entt::entity)>& fn) const {
        if (!m_Registry.valid(parent)) return;
        auto& rel = m_Registry.get<RelationshipComponent>(parent);
        auto child = rel.FirstChild;
        while (child != entt::null) {
            fn(child);
            auto& childRel = m_Registry.get<RelationshipComponent>(child);
            child = childRel.NextSibling;
        }
    }

    // ─── Raw access ─────────────────────────────────────────────────────

    entt::registry&       Raw()       { return m_Registry; }
    const entt::registry& Raw() const { return m_Registry; }

private:
    entt::registry m_Registry;
    std::unordered_map<UUID, entt::entity> m_UUIDMap;
};

} // namespace Freely
