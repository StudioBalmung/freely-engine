#include "Freely/ECS/Systems/RenderSystem.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Core/Engine.h"
#include "Freely/Renderer/Renderer.h"
#include "Freely/Scene/Mesh.h"
#include "Freely/Scene/Material.h"
#include "Freely/Renderer/Shader.h" // Assuming shader is needed for material setup

#include <glm/gtc/type_ptr.hpp>

namespace Freely {

void RenderSystem::OnLateUpdate(Scene& scene, float deltaTime) {
    auto& registry = scene.GetRegistry();
    auto& renderer = Engine::Get().GetRenderer();

    // 1. Find the active camera
    const Camera* activeCamera = m_EditorCamera;
    glm::mat4 cameraView(1.0f);
    glm::mat4 cameraProjection(1.0f);

    if (activeCamera) {
        cameraView = activeCamera->GetViewMatrix();
        cameraProjection = activeCamera->GetProjectionMatrix();
    } else {
        // Find primary camera in the scene if no editor camera is provided
        auto view = registry.GetEnttRegistry().view<TransformComponent, CameraComponent>();
        for (auto entity : view) {
            auto [transform, cameraComp] = view.get<TransformComponent, CameraComponent>(entity);
            if (cameraComp.Primary) {
                // Calculate view matrix from transform
                glm::vec3 pos = transform.Position;
                glm::vec3 front = transform.GetForward();
                glm::vec3 up = transform.GetUp();
                cameraView = glm::lookAt(pos, pos + front, up);
                
                // Assuming a default aspect ratio for now, can be extracted from viewport later
                cameraProjection = cameraComp.GetProjectionMatrix(16.0f / 9.0f); 
                break; // Found primary camera
            }
        }
    }

    // If we have no camera, we can't render
    if (!activeCamera && cameraProjection == glm::mat4(1.0f)) {
        return;
    }

    // For now, since Renderer::BeginFrame takes a Camera object, we construct a temporary one 
    // or we might need to modify Renderer to take matrices directly. Let's use the matrices.
    // Wait, Renderer::BeginFrame requires a Camera&. If activeCamera is null, we can't easily pass it.
    // We will assume Renderer has a way or we just set it manually. 
    // Actually, looking at Renderer::BeginFrame, it just extracts GetViewMatrix and GetProjectionMatrix.
    
    // Instead of calling BeginFrame (which the EditorApp might have already called or will call),
    // we'll just bind the shader uniforms if we had them. 
    // Wait, EditorApp calls:
    // GetRenderer().SetClearColor(...); GetRenderer().Clear();
    // It doesn't call BeginFrame! So we must setup the view/projection in the shaders ourselves.

    // 2. Render all entities with a MeshRendererComponent
    auto renderView = registry.GetEnttRegistry().view<TransformComponent, MeshRendererComponent>();
    for (auto entity : renderView) {
        auto [transform, meshRenderer] = renderView.get<TransformComponent, MeshRendererComponent>(entity);

        if (!meshRenderer.Visible) continue;

        // NOTE: In a complete engine, we would use an AssetManager to get the Mesh and Material 
        // from meshRenderer.MeshHandle and MaterialHandle.
        // For this placeholder, we will check if the entity has a MeshFilterComponent to generate a primitive,
        // otherwise we skip.

        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;

        if (registry.GetEnttRegistry().any_of<MeshFilterComponent>(entity)) {
            auto& filter = registry.GetEnttRegistry().get<MeshFilterComponent>(entity);
            if (filter.PrimitiveType == PrimitiveMeshType::Cube) mesh = Mesh::CreateCube();
            else if (filter.PrimitiveType == PrimitiveMeshType::Plane) mesh = Mesh::CreatePlane();
            else if (filter.PrimitiveType == PrimitiveMeshType::Sphere) mesh = Mesh::CreateSphere();
        }

        // If no mesh was found/generated, skip rendering this entity
        if (!mesh) continue;

        // If we had a material, we would bind it. For now, we assume a default material/shader is active.
        // if (material) material->Bind();

        // We would upload the transform matrix here.
        // if (material && material->ShaderProgram) {
        //     material->ShaderProgram->SetMat4("u_Model", transform.WorldMatrix);
        //     material->ShaderProgram->SetMat4("u_View", cameraView);
        //     material->ShaderProgram->SetMat4("u_Projection", cameraProjection);
        // }

        mesh->Draw();

        // if (material) material->Unbind();
    }
}

} // namespace Freely
