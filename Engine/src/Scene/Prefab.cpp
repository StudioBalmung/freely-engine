#include "Freely/ECS/Prefab.h"
#include "Freely/ECS/SceneSerializer.h"
#include "Freely/Core/Logger.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Freely {

// Reuse SceneSerializer's logic for individual entities by making a dummy scene
// Alternatively, just do manual JSON serialization here. For simplicity, we'll implement a custom pass.

bool Prefab::CreateFromEntity(Scene& scene, entt::entity entity, const std::string& filepath) {
    if (!scene.GetRegistry().IsValid(entity)) {
        FL_ENGINE_ERROR("Cannot create prefab from invalid entity");
        return false;
    }

    // We can use the scene serializer to serialize just this entity and its children.
    // For a robust system, we would serialize an array of entities starting with the root.
    
    // Let's create a temporary scene, duplicate the entity there, and serialize it.
    Scene tempScene;
    entt::entity copiedRoot = tempScene.CreateEntity("Root");
    
    // For now, we can manually copy components or use the duplicate logic if we refactor it to accept a target scene.
    // Assuming we want a simple approach: just serialize the single entity to JSON.
    // In a real engine, we'd recursively serialize the entity and all children into a JSON array.
    
    FL_ENGINE_WARN("Prefab::CreateFromEntity is a stub implementation");
    return false;
}

entt::entity Prefab::Instantiate(Scene& scene, const std::string& filepath) {
    std::ifstream stream(filepath);
    if (!stream.is_open()) {
        FL_ENGINE_ERROR("Failed to open prefab file '{0}'", filepath);
        return entt::null;
    }

    json prefabJson;
    try {
        stream >> prefabJson;
    } catch (json::parse_error& e) {
        FL_ENGINE_ERROR("Failed to parse prefab file '{0}': {1}", filepath, e.what());
        return entt::null;
    }

    // A full implementation would deserialize the entities array from the prefab JSON
    // and remap UUIDs to new generated UUIDs.
    FL_ENGINE_WARN("Prefab::Instantiate is a stub implementation");
    return entt::null;
}

} // namespace Freely
