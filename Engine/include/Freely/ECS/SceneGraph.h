#pragma once

// Freely Engine 0.4.2 — Scene Graph
// Handles hierarchical transform propagation through the entity tree.

#include "Registry.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Freely {

class SceneGraph {
public:
    /// Update all transform matrices in the scene.
    /// Call once per frame before rendering.
    static void UpdateTransforms(Registry& registry) {
        // Process only root entities (no parent), then recurse
        registry.ForEachRootEntity([&](entt::entity root) {
            UpdateEntityTransform(registry, root, glm::mat4(1.0f));
        });
    }

    /// Force-update a single entity and its children.
    static void UpdateEntityTransform(Registry& registry, entt::entity entity, const glm::mat4& parentWorld) {
        auto* transform = registry.TryGetComponent<TransformComponent>(entity);
        if (!transform) return;

        // Compute local matrix: T * R * S
        glm::mat4 local = glm::translate(glm::mat4(1.0f), transform->Position)
                        * glm::toMat4(transform->Rotation)
                        * glm::scale(glm::mat4(1.0f), transform->Scale);
        transform->LocalMatrix = local;
        transform->WorldMatrix = parentWorld * local;
        transform->Dirty = false;

        // Recurse into children
        registry.ForEachChild(entity, [&](entt::entity child) {
            UpdateEntityTransform(registry, child, transform->WorldMatrix);
        });
    }

    /// Get the world position of an entity.
    static glm::vec3 GetWorldPosition(const Registry& registry, entt::entity entity) {
        auto* transform = registry.TryGetComponent<TransformComponent>(entity);
        if (!transform) return glm::vec3(0.0f);
        return glm::vec3(transform->WorldMatrix[3]);
    }

    /// Get the world rotation of an entity.
    static glm::quat GetWorldRotation(const Registry& registry, entt::entity entity) {
        auto* transform = registry.TryGetComponent<TransformComponent>(entity);
        if (!transform) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        return glm::quat_cast(transform->WorldMatrix);
    }

    /// Get the world scale of an entity (approximate — shear is ignored).
    static glm::vec3 GetWorldScale(const Registry& registry, entt::entity entity) {
        auto* transform = registry.TryGetComponent<TransformComponent>(entity);
        if (!transform) return glm::vec3(1.0f);
        const auto& m = transform->WorldMatrix;
        return glm::vec3(
            glm::length(glm::vec3(m[0])),
            glm::length(glm::vec3(m[1])),
            glm::length(glm::vec3(m[2]))
        );
    }

    /// Convert a world-space position to local-space relative to parent.
    static glm::vec3 WorldToLocal(const Registry& registry, entt::entity entity, const glm::vec3& worldPos) {
        auto* rel = registry.TryGetComponent<RelationshipComponent>(entity);
        if (!rel || rel->Parent == entt::null) return worldPos;

        auto* parentTransform = registry.TryGetComponent<TransformComponent>(rel->Parent);
        if (!parentTransform) return worldPos;

        glm::mat4 invParent = glm::inverse(parentTransform->WorldMatrix);
        return glm::vec3(invParent * glm::vec4(worldPos, 1.0f));
    }

    /// Set world position (converts to local space if entity has a parent).
    static void SetWorldPosition(Registry& registry, entt::entity entity, const glm::vec3& worldPos) {
        auto* transform = registry.TryGetComponent<TransformComponent>(entity);
        if (!transform) return;

        transform->Position = WorldToLocal(registry, entity, worldPos);
        transform->Dirty = true;
    }

    /// Collect all entities in depth-first order.
    static void TraverseDFS(const Registry& registry, entt::entity entity,
                            const std::function<void(entt::entity, int depth)>& visitor, int depth = 0) {
        visitor(entity, depth);
        registry.ForEachChild(entity, [&](entt::entity child) {
            TraverseDFS(registry, child, visitor, depth + 1);
        });
    }

    /// Collect all root entities and traverse them.
    static void TraverseAll(const Registry& registry,
                            const std::function<void(entt::entity, int depth)>& visitor) {
        registry.ForEachRootEntity([&](entt::entity root) {
            TraverseDFS(registry, root, visitor, 0);
        });
    }

    /// Check if 'ancestor' is an ancestor of 'entity'.
    static bool IsAncestor(const Registry& registry, entt::entity entity, entt::entity ancestor) {
        auto* rel = registry.TryGetComponent<RelationshipComponent>(entity);
        while (rel && rel->Parent != entt::null) {
            if (rel->Parent == ancestor) return true;
            rel = registry.TryGetComponent<RelationshipComponent>(rel->Parent);
        }
        return false;
    }
};

} // namespace Freely
