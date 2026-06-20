#pragma once

#include <Freely/Core/Engine.h>
#include <Freely/Scene/Camera.h>
#include <Freely/Renderer/Shader.h>
#include <Freely/Renderer/Framebuffer.h>
#include <Freely/Scene/Mesh.h>

#include "Editor/EditorContext.h"
#include "Editor/ProjectManager.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/InspectorPanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/ViewportPanel.h"
#include "Editor/Panels/ConsolePanel.h"
#include "Editor/Panels/ToolbarPanel.h"

#include <memory>

namespace FreelyEditor {

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
    void HandleViewportCamera(float dt);

    std::shared_ptr<Freely::Framebuffer> m_ViewportFB;
    std::shared_ptr<Freely::Shader> m_PBRShader;
    std::shared_ptr<Freely::Shader> m_GridShader;
    Freely::Camera m_EditorCamera;

    // Viewport camera state
    glm::vec3 m_CamPos{0.0f, 3.0f, 8.0f};
    float m_CamYaw = 180.0f;
    float m_CamPitch = -15.0f;

    // Editor Context & Panels
    EditorContext m_Context;
    ProjectManager m_ProjectManager;

    std::unique_ptr<SceneHierarchyPanel> m_HierarchyPanel;
    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
    std::unique_ptr<ViewportPanel> m_ViewportPanel;
    std::unique_ptr<ConsolePanel> m_ConsolePanel;
    std::unique_ptr<ToolbarPanel> m_ToolbarPanel;
};

} // namespace FreelyEditor
