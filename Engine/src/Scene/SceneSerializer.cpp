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

    // --- MaterialComponent ------------------------------------------------
    if (registry.HasComponent<MaterialComponent>(entity)) {
        auto& mc = registry.GetComponent<MaterialComponent>(entity);
        j["MaterialComponent"] = {
            {"Albedo", mc.Albedo}, {"Metallic", mc.Metallic},
            {"Roughness", mc.Roughness}, {"AO", mc.AO},
            {"Emissive", mc.Emissive}, {"EmissiveStrength", mc.EmissiveStrength},
            {"AlbedoMap", mc.AlbedoMapPath}, {"NormalMap", mc.NormalMapPath},
            {"MetallicRoughnessMap", mc.MetallicRoughnessMapPath},
            {"AOMap", mc.AOMapPath}, {"EmissiveMap", mc.EmissiveMapPath},
            {"TwoSided", mc.TwoSided}, {"AlphaCutoff", mc.AlphaCutoff}
        };
    }

    // --- AudioSourceComponent ---------------------------------------------
    if (registry.HasComponent<AudioSourceComponent>(entity)) {
        auto& ac = registry.GetComponent<AudioSourceComponent>(entity);
        j["AudioSourceComponent"] = {
            {"ClipPath", ac.ClipPath}, {"Volume", ac.Volume},
            {"Pitch", ac.Pitch}, {"MinDistance", ac.MinDistance},
            {"MaxDistance", ac.MaxDistance}, {"Loop", ac.Loop},
            {"PlayOnAwake", ac.PlayOnAwake}, {"Spatial", ac.Spatial}
        };
    }

    // --- SkyboxComponent -------------------------------------------------
    if (registry.HasComponent<SkyboxComponent>(entity)) {
        auto& sky = registry.GetComponent<SkyboxComponent>(entity);
        j["SkyboxComponent"] = {
            {"CubemapPath", sky.CubemapPath},
            {"Intensity", sky.Intensity},
            {"Rotation", sky.Rotation}
        };
    }

    // --- MeshFilterComponent ---------------------------------------------
    if (registry.HasComponent<MeshFilterComponent>(entity)) {
        auto& mf = registry.GetComponent<MeshFilterComponent>(entity);
        j["MeshFilterComponent"] = {
            {"PrimitiveType", static_cast<int>(mf.PrimitiveType)},
            {"CustomMeshPath", mf.CustomMeshPath}
        };
    }

    // --- SpriteRendererComponent ------------------------------------------
    if (registry.HasComponent<SpriteRendererComponent>(entity)) {
        auto& spr = registry.GetComponent<SpriteRendererComponent>(entity);
        j["SpriteRendererComponent"] = {
            {"Color", spr.Color}, {"TexturePath", spr.TexturePath},
            {"TilingFactor", spr.TilingFactor}, {"Offset", spr.Offset},
            {"FlipX", spr.FlipX}, {"FlipY", spr.FlipY},
            {"SortingLayer", spr.SortingLayer}, {"OrderInLayer", spr.OrderInLayer},
            {"Visible", spr.Visible}
        };
    }

    // --- Text2DComponent --------------------------------------------------
    if (registry.HasComponent<Text2DComponent>(entity)) {
        auto& txt = registry.GetComponent<Text2DComponent>(entity);
        j["Text2DComponent"] = {
            {"Text", txt.Text}, {"FontPath", txt.FontPath},
            {"Color", txt.Color}, {"FontSize", txt.FontSize},
            {"Kerning", txt.Kerning}, {"LineSpacing", txt.LineSpacing},
            {"AlignH", txt.AlignH}, {"Visible", txt.Visible}
        };
    }

    // --- Camera2DComponent ------------------------------------------------
    if (registry.HasComponent<Camera2DComponent>(entity)) {
        auto& c2d = registry.GetComponent<Camera2DComponent>(entity);
        j["Camera2DComponent"] = {
            {"Size", c2d.Size}, {"Near", c2d.Near}, {"Far", c2d.Far},
            {"Primary", c2d.Primary}, {"FixedAspect", c2d.FixedAspect},
            {"AspectRatio", c2d.AspectRatio}, {"Background", c2d.Background}
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
        struct RelationshipRelation {
            entt::entity Child;
            uint64_t ParentUUID;
        };
        std::vector<RelationshipRelation> relations;

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

            // Relationship Component logic is deferred to a second pass once all entities are loaded
            auto relationshipComponentJson = entityJson["RelationshipComponent"];
            if (!relationshipComponentJson.is_null()) {
                uint64_t parentUUID = relationshipComponentJson["Parent"].get<uint64_t>();
                if (parentUUID != 0) {
                    relations.push_back({deserializedEntity, parentUUID});
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



            // --- MaterialComponent --------------------------------------------
            if (entityJson.contains("MaterialComponent") && !entityJson["MaterialComponent"].is_null()) {
                auto& j = entityJson["MaterialComponent"];
                auto& mc = m_Scene.GetRegistry().AddComponent<MaterialComponent>(deserializedEntity);
                mc.Albedo    = j["Albedo"].get<glm::vec3>();
                mc.Metallic  = j["Metallic"].get<float>();
                mc.Roughness = j["Roughness"].get<float>();
                mc.AO        = j["AO"].get<float>();
                mc.Emissive  = j["Emissive"].get<glm::vec3>();
                mc.EmissiveStrength = j["EmissiveStrength"].get<float>();
                mc.AlbedoMapPath    = j["AlbedoMap"].get<std::string>();
                mc.NormalMapPath    = j["NormalMap"].get<std::string>();
                mc.MetallicRoughnessMapPath = j["MetallicRoughnessMap"].get<std::string>();
                mc.AOMapPath     = j["AOMap"].get<std::string>();
                mc.EmissiveMapPath = j["EmissiveMap"].get<std::string>();
                mc.TwoSided      = j["TwoSided"].get<bool>();
                mc.AlphaCutoff   = j["AlphaCutoff"].get<float>();
            }

            // --- AudioSourceComponent --------------------------------------
            if (entityJson.contains("AudioSourceComponent") && !entityJson["AudioSourceComponent"].is_null()) {
                auto& j = entityJson["AudioSourceComponent"];
                auto& ac = m_Scene.GetRegistry().AddComponent<AudioSourceComponent>(deserializedEntity);
                ac.ClipPath    = j["ClipPath"].get<std::string>();
                ac.Volume      = j["Volume"].get<float>();
                ac.Pitch       = j["Pitch"].get<float>();
                ac.MinDistance = j["MinDistance"].get<float>();
                ac.MaxDistance = j["MaxDistance"].get<float>();
                ac.Loop        = j["Loop"].get<bool>();
                ac.PlayOnAwake = j["PlayOnAwake"].get<bool>();
                ac.Spatial     = j["Spatial"].get<bool>();
            }

            // --- SkyboxComponent -------------------------------------------
            if (entityJson.contains("SkyboxComponent") && !entityJson["SkyboxComponent"].is_null()) {
                auto& j = entityJson["SkyboxComponent"];
                auto& sky = m_Scene.GetRegistry().AddComponent<SkyboxComponent>(deserializedEntity);
                sky.CubemapPath = j["CubemapPath"].get<std::string>();
                sky.Intensity   = j["Intensity"].get<float>();
                sky.Rotation    = j["Rotation"].get<float>();
            }

            // --- MeshFilterComponent ---------------------------------------
            if (entityJson.contains("MeshFilterComponent") && !entityJson["MeshFilterComponent"].is_null()) {
                auto& j = entityJson["MeshFilterComponent"];
                auto& mf = m_Scene.GetRegistry().AddComponent<MeshFilterComponent>(deserializedEntity);
                mf.PrimitiveType  = static_cast<PrimitiveMeshType>(j["PrimitiveType"].get<int>());
                mf.CustomMeshPath = j["CustomMeshPath"].get<std::string>();
            }

            // --- SpriteRendererComponent -----------------------------------
            if (entityJson.contains("SpriteRendererComponent") && !entityJson["SpriteRendererComponent"].is_null()) {
                auto& j = entityJson["SpriteRendererComponent"];
                auto& spr = m_Scene.GetRegistry().AddComponent<SpriteRendererComponent>(deserializedEntity);
                spr.Color        = j["Color"].get<glm::vec4>();
                spr.TexturePath  = j["TexturePath"].get<std::string>();
                spr.TilingFactor = j["TilingFactor"].get<glm::vec2>();
                spr.Offset       = j["Offset"].get<glm::vec2>();
                spr.FlipX        = j["FlipX"].get<bool>();
                spr.FlipY        = j["FlipY"].get<bool>();
                spr.SortingLayer = j["SortingLayer"].get<int>();
                spr.OrderInLayer = j["OrderInLayer"].get<int>();
                spr.Visible      = j["Visible"].get<bool>();
            }

            // --- Text2DComponent -------------------------------------------
            if (entityJson.contains("Text2DComponent") && !entityJson["Text2DComponent"].is_null()) {
                auto& j = entityJson["Text2DComponent"];
                auto& txt = m_Scene.GetRegistry().AddComponent<Text2DComponent>(deserializedEntity);
                txt.Text        = j["Text"].get<std::string>();
                txt.FontPath    = j["FontPath"].get<std::string>();
                txt.Color       = j["Color"].get<glm::vec4>();
                txt.FontSize    = j["FontSize"].get<float>();
                txt.Kerning     = j["Kerning"].get<float>();
                txt.LineSpacing = j["LineSpacing"].get<float>();
                txt.AlignH      = j["AlignH"].get<float>();
                txt.Visible     = j["Visible"].get<bool>();
            }

            // --- Camera2DComponent -----------------------------------------
            if (entityJson.contains("Camera2DComponent") && !entityJson["Camera2DComponent"].is_null()) {
                auto& j = entityJson["Camera2DComponent"];
                auto& c2d = m_Scene.GetRegistry().AddComponent<Camera2DComponent>(deserializedEntity);
                c2d.Size        = j["Size"].get<float>();
                c2d.Near        = j["Near"].get<float>();
                c2d.Far         = j["Far"].get<float>();
                c2d.Primary     = j["Primary"].get<bool>();
                c2d.FixedAspect = j["FixedAspect"].get<bool>();
                c2d.AspectRatio = j["AspectRatio"].get<float>();
            }
        }

        // Resolve parent-child relations in a second pass once all entities are loaded
        for (const auto& relation : relations) {
            entt::entity parentEntity = m_Scene.FindEntityByUUID(UUID(relation.ParentUUID));
            if (parentEntity != entt::null) {
                m_Scene.GetRegistry().SetParent(relation.Child, parentEntity);
            } else {
                FL_ENGINE_WARN("Failed to resolve parent relationship for entity (UUID {}). Parent UUID {} not found in scene.", 
                               m_Scene.GetRegistry().GetUUID(relation.Child).ToString(), relation.ParentUUID);
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

