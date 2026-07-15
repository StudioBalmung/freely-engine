#pragma once

#include "Scene.h"
#include <string>

namespace Freely {

class Prefab {
public:
    Prefab() = default;
    
    // Create a prefab from an entity in a scene
    static bool CreateFromEntity(Scene& scene, entt::entity entity, const std::string& filepath);
    
    // Instantiate a prefab into a scene
    static entt::entity Instantiate(Scene& scene, const std::string& filepath);
};

} // namespace Freely
