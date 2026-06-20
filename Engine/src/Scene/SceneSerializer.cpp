#include "Freely/ECS/SceneSerializer.h"
#include "Freely/ECS/Components.h"
#include "Freely/Core/Logger.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

using json = nlohmann::json;

namespace glm {
    void to_json(json& j, const glm::vec2& v) { j = json{v.x, v.y}; }
    void from_json(const json& j, glm::vec2& v) { j.at(0).get_to(v.x); j.at(1).get_to(v.y); }

    void to_json(json& j, const glm::vec3& v) { j = json{v.x, v.y, v.z}; }
    void from_json(const json& j, glm::vec3& v) { j.at(0).get_to(v.x); j.at(1).get_to(v.y); j.at(2).get_to(v.z); }

    void to_json(json& j, const glm::vec4& v) { j = json{v.x, v.y, v.z, v.w}; }
    void from_json(const json& j, glm::vec4& v) { j.at(0).get_to(v.x); j.at(1).get_to(v.y); j.at(2).get_to(v.z); j.at(3).get_to(v.w); }
    
    void to_json(json& j, const glm::quat& v) { j = json{v.w, v.x, v.y, v.z}; }
    void from_json(const json& j, glm::quat& v) { j.at(0).get_to(v.w); j.at(1).get_to(v.x); j.at(2).get_to(v.y); j.at(3).get_to(v.z); }
}

namespace Freely {

SceneSerializer::SceneSerializer(Scene& scene)
    : m_Scene(scene)
{
}

static void SerializeEntity(json& j, entt::entity entity, const Registry& registry) {
    if (!registry.HasComponent<IDComponent>(entity))
        return;

    j["Entity"] = registry.GetComponent<IDComponent>(entity).ID.Value;

    if (registry.HasComponent<TagComponent>(entity)) {
        auto& tc = registry.GetComponent<TagComponent>(entity);
        j["TagComponent"] = {
            {"Name", tc.Name},
            {"Tag", tc.Tag},
            {"Layer", tc.Layer},
            {"Active", tc.Active}
        };
    }

    if (registry.HasComponent<TransformComponent>(entity)) {
        auto& tc = registry.GetComponent<TransformComponent>(entity);
        j["TransformComponent"] = {
            {"Position", tc.Position},
            {"Rotation", tc.Rotation},
            {"Scale", tc.Scale}
        };
    }

    if (registry.HasComponent<RelationshipComponent>(entity)) {
        auto& rc = registry.GetComponent<RelationshipComponent>(entity);
        j["RelationshipComponent"] = {
            {"Parent", rc.Parent == entt::null ? 0 : registry.GetComponent<IDComponent>(rc.Parent).ID.Value}
        };
    }

    if (registry.HasComponent<CameraComponent>(entity)) {
        auto& cc = registry.GetComponent<CameraComponent>(entity);
        j["CameraComponent"] = {
            {"Projection", static_cast<int>(cc.Projection)},
            {"FOV", cc.FOV},
            {"NearPlane", cc.NearPlane},
            {"FarPlane", cc.FarPlane},
            {"OrthoSize", cc.OrthoSize},
            {"Primary", cc.Primary},
            {"FixedAspect", cc.FixedAspect},
            {"AspectRatio", cc.AspectRatio}
        };
    }
    
    if (registry.HasComponent<MeshRendererComponent>(entity)) {
        auto& mrc = registry.GetComponent<MeshRendererComponent>(entity);
        j["MeshRendererComponent"] = {
            {"MeshAsset", mrc.MeshAsset},
            {"MaterialAsset", mrc.MaterialAsset},
            {"CastShadows", mrc.CastShadows},
            {"ReceiveShadows", mrc.ReceiveShadows},
            {"Visible", mrc.Visible}
        };
    }

    if (registry.HasComponent<LightComponent>(entity)) {
        auto& lc = registry.GetComponent<LightComponent>(entity);
        j["LightComponent"] = {
            {"Type", static_cast<int>(lc.Type)},
            {"Color", lc.Color},
            {"Intensity", lc.Intensity},
            {"Range", lc.Range},
            {"Constant", lc.Constant},
            {"Linear", lc.Linear},
            {"Quadratic", lc.Quadratic},
            {"InnerCutoff", lc.InnerCutoff},
            {"OuterCutoff", lc.OuterCutoff},
            {"CastShadows", lc.CastShadows},
            {"ShadowMapSize", lc.ShadowMapSize},
            {"ShadowBias", lc.ShadowBias},
            {"ShadowNearPlane", lc.ShadowNearPlane}
        };
    }
    
    if (registry.HasComponent<RigidBodyComponent>(entity)) {
        auto& rbc = registry.GetComponent<RigidBodyComponent>(entity);
        j["RigidBodyComponent"] = {
            {"Type", static_cast<int>(rbc.Type)},
            {"Mass", rbc.Mass},
            {"LinearDamping", rbc.LinearDamping},
            {"AngularDamping", rbc.AngularDamping},
            {"Friction", rbc.Friction},
            {"Restitution", rbc.Restitution},
            {"UseGravity", rbc.UseGravity},
            {"ContinuousCD", rbc.ContinuousCD},
            {"FreezeRotationX", rbc.FreezeRotationX},
            {"FreezeRotationY", rbc.FreezeRotationY},
            {"FreezeRotationZ", rbc.FreezeRotationZ}
        };
    }

    if (registry.HasComponent<ColliderComponent>(entity)) {
        auto& cc = registry.GetComponent<ColliderComponent>(entity);
        j["ColliderComponent"] = {
            {"Shape", static_cast<int>(cc.Shape)},
            {"BoxHalfExtents", cc.BoxHalfExtents},
            {"SphereRadius", cc.SphereRadius},
            {"CapsuleRadius", cc.CapsuleRadius},
            {"CapsuleHeight", cc.CapsuleHeight},
            {"Center", cc.Center},
            {"IsTrigger", cc.IsTrigger}
        };
    }
    
    if (registry.HasComponent<ScriptComponent>(entity)) {
        auto& sc = registry.GetComponent<ScriptComponent>(entity);
        j["ScriptComponent"] = {
            {"ClassName", sc.ClassName},
            {"ScriptPath", sc.ScriptPath},
            {"Language", sc.Language}
        };
    }
}

void SceneSerializer::Serialize(const std::string& filepath) {
    json sceneJson;
    sceneJson["Scene"] = "Untitled";

    json entitiesJson = json::array();

    m_Scene.GetRegistry().ForEachEntity([&](entt::entity entity) {
        if (!m_Scene.GetRegistry().HasComponent<IDComponent>(entity)) return;
        json entityJson;
        SerializeEntity(entityJson, entity, m_Scene.GetRegistry());
        entitiesJson.push_back(entityJson);
    });

    sceneJson["Entities"] = entitiesJson;

    std::ofstream fout(filepath);
    fout << sceneJson.dump(4);
}

bool SceneSerializer::Deserialize(const std::string& filepath) {
    std::ifstream stream(filepath);
    if (!stream.is_open()) {
        FL_ENGINE_ERROR("Failed to open scene file '{0}'", filepath);
        return false;
    }

    json sceneJson;
    try {
        stream >> sceneJson;
    } catch (json::parse_error& e) {
        FL_ENGINE_ERROR("Failed to parse scene file '{0}': {1}", filepath, e.what());
        return false;
    }

    if (!sceneJson.contains("Scene"))
        return false;

    std::string sceneName = sceneJson["Scene"];
    FL_ENGINE_INFO("Deserializing scene '{0}'", sceneName);

    if (sceneJson.contains("Entities")) {
        auto entitiesJson = sceneJson["Entities"];
        for (auto& entityJson : entitiesJson) {
            uint64_t uuid = entityJson["Entity"].get<uint64_t>();

            std::string name;
            auto tagComponentJson = entityJson["TagComponent"];
            if (!tagComponentJson.is_null()) {
                name = tagComponentJson["Name"].get<std::string>();
            }

            FL_ENGINE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);
            entt::entity deserializedEntity = m_Scene.CreateEntityWithUUID(UUID(uuid), name);

            if (!tagComponentJson.is_null()) {
                auto& tc = m_Scene.GetRegistry().GetComponent<TagComponent>(deserializedEntity);
                tc.Tag = tagComponentJson["Tag"].get<std::string>();
                tc.Layer = tagComponentJson["Layer"].get<int>();
                tc.Active = tagComponentJson["Active"].get<bool>();
            }

            auto transformComponentJson = entityJson["TransformComponent"];
            if (!transformComponentJson.is_null()) {
                auto& tc = m_Scene.GetRegistry().GetComponent<TransformComponent>(deserializedEntity);
                tc.Position = transformComponentJson["Position"].get<glm::vec3>();
                tc.Rotation = transformComponentJson["Rotation"].get<glm::quat>();
                tc.Scale = transformComponentJson["Scale"].get<glm::vec3>();
            }

            // Relationship Component logic must be deferred or processed in a second pass if parents aren't loaded yet
            auto relationshipComponentJson = entityJson["RelationshipComponent"];
            if (!relationshipComponentJson.is_null()) {
                uint64_t parentUUID = relationshipComponentJson["Parent"].get<uint64_t>();
                if (parentUUID != 0) {
                    entt::entity parentEntity = m_Scene.FindEntityByUUID(UUID(parentUUID));
                    if (parentEntity != entt::null) {
                        m_Scene.GetRegistry().SetParent(deserializedEntity, parentEntity);
                    } else {
                        // TODO: Handle forward references if parents are deserialized after children
                    }
                }
            }

            auto cameraComponentJson = entityJson["CameraComponent"];
            if (!cameraComponentJson.is_null()) {
                auto& cc = m_Scene.GetRegistry().AddComponent<CameraComponent>(deserializedEntity);
                cc.Projection = static_cast<ProjectionType>(cameraComponentJson["Projection"].get<int>());
                cc.FOV = cameraComponentJson["FOV"].get<float>();
                cc.NearPlane = cameraComponentJson["NearPlane"].get<float>();
                cc.FarPlane = cameraComponentJson["FarPlane"].get<float>();
                cc.OrthoSize = cameraComponentJson["OrthoSize"].get<float>();
                cc.Primary = cameraComponentJson["Primary"].get<bool>();
                cc.FixedAspect = cameraComponentJson["FixedAspect"].get<bool>();
                cc.AspectRatio = cameraComponentJson["AspectRatio"].get<float>();
            }
            
            auto meshRendererComponentJson = entityJson["MeshRendererComponent"];
            if (!meshRendererComponentJson.is_null()) {
                auto& mrc = m_Scene.GetRegistry().AddComponent<MeshRendererComponent>(deserializedEntity);
                mrc.MeshAsset = meshRendererComponentJson["MeshAsset"].get<std::string>();
                mrc.MaterialAsset = meshRendererComponentJson["MaterialAsset"].get<std::string>();
                mrc.CastShadows = meshRendererComponentJson["CastShadows"].get<bool>();
                mrc.ReceiveShadows = meshRendererComponentJson["ReceiveShadows"].get<bool>();
                mrc.Visible = meshRendererComponentJson["Visible"].get<bool>();
            }

            auto lightComponentJson = entityJson["LightComponent"];
            if (!lightComponentJson.is_null()) {
                auto& lc = m_Scene.GetRegistry().AddComponent<LightComponent>(deserializedEntity);
                lc.Type = static_cast<LightType>(lightComponentJson["Type"].get<int>());
                lc.Color = lightComponentJson["Color"].get<glm::vec3>();
                lc.Intensity = lightComponentJson["Intensity"].get<float>();
                lc.Range = lightComponentJson["Range"].get<float>();
                lc.Constant = lightComponentJson["Constant"].get<float>();
                lc.Linear = lightComponentJson["Linear"].get<float>();
                lc.Quadratic = lightComponentJson["Quadratic"].get<float>();
                lc.InnerCutoff = lightComponentJson["InnerCutoff"].get<float>();
                lc.OuterCutoff = lightComponentJson["OuterCutoff"].get<float>();
                lc.CastShadows = lightComponentJson["CastShadows"].get<bool>();
                lc.ShadowMapSize = lightComponentJson["ShadowMapSize"].get<int>();
                lc.ShadowBias = lightComponentJson["ShadowBias"].get<float>();
                lc.ShadowNearPlane = lightComponentJson["ShadowNearPlane"].get<float>();
            }
            
            auto rigidBodyComponentJson = entityJson["RigidBodyComponent"];
            if (!rigidBodyComponentJson.is_null()) {
                auto& rbc = m_Scene.GetRegistry().AddComponent<RigidBodyComponent>(deserializedEntity);
                rbc.Type = static_cast<BodyType>(rigidBodyComponentJson["Type"].get<int>());
                rbc.Mass = rigidBodyComponentJson["Mass"].get<float>();
                rbc.LinearDamping = rigidBodyComponentJson["LinearDamping"].get<float>();
                rbc.AngularDamping = rigidBodyComponentJson["AngularDamping"].get<float>();
                rbc.Friction = rigidBodyComponentJson["Friction"].get<float>();
                rbc.Restitution = rigidBodyComponentJson["Restitution"].get<float>();
                rbc.UseGravity = rigidBodyComponentJson["UseGravity"].get<bool>();
                rbc.ContinuousCD = rigidBodyComponentJson["ContinuousCD"].get<bool>();
                rbc.FreezeRotationX = rigidBodyComponentJson["FreezeRotationX"].get<bool>();
                rbc.FreezeRotationY = rigidBodyComponentJson["FreezeRotationY"].get<bool>();
                rbc.FreezeRotationZ = rigidBodyComponentJson["FreezeRotationZ"].get<bool>();
            }

            auto colliderComponentJson = entityJson["ColliderComponent"];
            if (!colliderComponentJson.is_null()) {
                auto& cc = m_Scene.GetRegistry().AddComponent<ColliderComponent>(deserializedEntity);
                cc.Shape = static_cast<ColliderShape>(colliderComponentJson["Shape"].get<int>());
                cc.BoxHalfExtents = colliderComponentJson["BoxHalfExtents"].get<glm::vec3>();
                cc.SphereRadius = colliderComponentJson["SphereRadius"].get<float>();
                cc.CapsuleRadius = colliderComponentJson["CapsuleRadius"].get<float>();
                cc.CapsuleHeight = colliderComponentJson["CapsuleHeight"].get<float>();
                cc.Center = colliderComponentJson["Center"].get<glm::vec3>();
                cc.IsTrigger = colliderComponentJson["IsTrigger"].get<bool>();
            }

            auto scriptComponentJson = entityJson["ScriptComponent"];
            if (!scriptComponentJson.is_null()) {
                auto& sc = m_Scene.GetRegistry().AddComponent<ScriptComponent>(deserializedEntity);
                sc.ClassName = scriptComponentJson["ClassName"].get<std::string>();
                sc.ScriptPath = scriptComponentJson["ScriptPath"].get<std::string>();
                sc.Language = scriptComponentJson["Language"].get<std::string>();
            }
        }
    }

    return true;
}

void SceneSerializer::SerializeRuntime(const std::string& filepath) {
    // Optional: Binary serialization
    FL_ENGINE_WARN("SerializeRuntime not implemented");
}

bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
    // Optional: Binary deserialization
    FL_ENGINE_WARN("DeserializeRuntime not implemented");
    return false;
}

} // namespace Freely
