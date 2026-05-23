#pragma once

#include <Freely/Core/Engine.h>
#include <Freely/Scene/Camera.h>
#include <Freely/Renderer/Shader.h>
#include <Freely/Renderer/Framebuffer.h>
#include <Freely/Scene/Mesh.h>
#include <AsterCore/PhysicsWorld.h>
#include <AsterCore/RigidBody.h>

#include <memory>
#include <vector>
#include <string>

namespace FreelyEditor {

struct SceneObject {
    std::string Name;
    std::shared_ptr<Freely::Mesh> MeshData;
    std::shared_ptr<AsterCore::RigidBody> Body;
    glm::vec3 Albedo{1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
};

class EditorApp : public Freely::Engine {
public:
    EditorApp();

protected:
    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnRender() override;
    void OnShutdown() override;

private:
    void InitImGui();
    void ShutdownImGui();
    void BeginImGuiFrame();
    void EndImGuiFrame();

    void DrawMenuBar();
    void DrawViewport();
    void DrawSceneHierarchy();
    void DrawProperties();
    void DrawToolbar();

    void HandleViewportCamera(float dt);

    std::shared_ptr<Freely::Framebuffer> m_ViewportFB;
    std::shared_ptr<Freely::Shader> m_PBRShader;
    std::shared_ptr<Freely::Shader> m_GridShader;
    Freely::Camera m_EditorCamera;

    std::vector<SceneObject> m_SceneObjects;
    int m_SelectedObject = -1;

    AsterCore::PhysicsWorld m_PhysicsWorld;
    bool m_PhysicsPlaying = false;

    // Viewport state
    glm::vec2 m_ViewportSize{1280, 720};
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;

    // Camera state
    glm::vec3 m_CamPos{0.0f, 3.0f, 8.0f};
    float m_CamYaw = 180.0f;
    float m_CamPitch = -15.0f;

    // Gizmo
    int m_GizmoOperation = 0; // 0=Translate, 1=Rotate, 2=Scale
};

} // namespace FreelyEditor
