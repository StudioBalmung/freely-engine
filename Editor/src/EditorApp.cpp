#include "Editor/EditorApp.h"
#include <Freely/Freely.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Freely/ECS/Systems/RenderSystem.h"

namespace FreelyEditor {

EditorApp::EditorApp()
    : Engine({"Freely Editor 2.0.0", 1600, 900, true, false})
{
}

void EditorApp::OnInit() {
    FL_INFO("Freely Editor 2.0.0 initialized!");

    InitImGui();

    // Init panels
    m_HierarchyPanel = std::make_unique<SceneHierarchyPanel>(&m_Context);
    m_InspectorPanel = std::make_unique<InspectorPanel>(&m_Context);
    m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>(&m_Context);
    m_ViewportPanel = std::make_unique<ViewportPanel>(&m_Context);
    m_ConsolePanel = std::make_unique<ConsolePanel>(&m_Context);
    m_ToolbarPanel = std::make_unique<ToolbarPanel>(&m_Context);

    m_ProjectManager.Init();

    // Create viewport framebuffer
    Freely::FramebufferSpec fbSpec;
    fbSpec.Width = 1280;
    fbSpec.Height = 720;
    m_ViewportFB = Freely::Framebuffer::Create(fbSpec);

    // Setup editor camera
    m_EditorCamera = Freely::Camera(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    m_EditorCamera.SetPosition(m_CamPos);
    m_EditorCamera.SetRotation({m_CamPitch, m_CamYaw, 0.0f});

    GetRenderer().SetClearColor({0.15f, 0.15f, 0.18f, 1.0f});
}

void EditorApp::OnUpdate(float dt) {
    if (m_ProjectManager.IsProjectLoaded()) {
        HandleViewportCamera(dt);

        if (m_Context.State == EditorState::Play) {
            if (m_Context.ActiveScene) {
                m_Context.ActiveScene->Update(dt);
            }
        } else {
            // Editor mode
            if (m_Context.ActiveScene) {
                Freely::SceneGraph::UpdateTransforms(m_Context.ActiveScene->GetRegistry());
            }
        }
    }

    if (GetInput().IsKeyPressed(GLFW_KEY_ESCAPE)) Shutdown();
    
    // Gizmo shortcuts
    if (!m_ViewportPanel->IsFocused()) {
        if (GetInput().IsKeyPressed(GLFW_KEY_Q)) m_Context.GizmoType = -1;
        if (GetInput().IsKeyPressed(GLFW_KEY_W)) m_Context.GizmoType = ImGuizmo::OPERATION::TRANSLATE;
        if (GetInput().IsKeyPressed(GLFW_KEY_E)) m_Context.GizmoType = ImGuizmo::OPERATION::ROTATE;
        if (GetInput().IsKeyPressed(GLFW_KEY_R)) m_Context.GizmoType = ImGuizmo::OPERATION::SCALE;
    }
}

void EditorApp::OnRender() {
    if (m_ProjectManager.IsProjectLoaded()) {
        m_ViewportFB->Bind();
        GetRenderer().SetClearColor({0.12f, 0.12f, 0.14f, 1.0f});
        GetRenderer().Clear();
        GetRenderer().SetDepthTest(true);

        if (m_Context.ActiveScene) {
            auto* renderSystem = m_Context.ActiveScene->GetSystemScheduler().GetSystem<Freely::RenderSystem>();
            if (renderSystem) {
                if (m_Context.State == EditorState::Play) {
                    renderSystem->SetEditorCamera(nullptr);
                } else {
                    renderSystem->SetEditorCamera(&m_EditorCamera);
                }
            }
            m_Context.ActiveScene->LateUpdate(GetDeltaTime());
        }

        m_ViewportFB->Unbind();
        m_ViewportPanel->SetFramebufferTexture(m_ViewportFB->GetColorAttachment());
    }

    GetRenderer().SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    GetRenderer().Clear();
    BeginImGuiFrame();

    if (!m_ProjectManager.IsProjectLoaded()) {
        bool isOpen = true;
        m_ProjectManager.OnImGuiRender(&isOpen);
        if (!isOpen) Shutdown();
    } else {
        // Dockspace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags dockFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, dockFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("FreelyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        DrawMenuBar();
        
        m_ToolbarPanel->OnImGuiRender();
        m_HierarchyPanel->OnImGuiRender();
        m_InspectorPanel->OnImGuiRender();
        m_ContentBrowserPanel->OnImGuiRender();
        m_ConsolePanel->OnImGuiRender();
        m_ViewportPanel->OnImGuiRender();

        ImGui::End();
    }

    EndImGuiFrame();
}

void EditorApp::OnShutdown() {
    ShutdownImGui();
    FL_INFO("Freely Editor shutting down.");
}

void EditorApp::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    
    // Monochrome Theme
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.145f, 0.145f, 0.145f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(GetWindow().GetNativeHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

void EditorApp::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorApp::BeginImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void EditorApp::EndImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorApp::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                m_Context.ActiveScene = std::make_shared<Freely::Scene>();
                m_Context.SelectedEntity = entt::null;
            }
            if (ImGui::MenuItem("Save Scene")) {
                Freely::SceneSerializer serializer(*m_Context.ActiveScene);
                // Hardcoded save path for testing
                serializer.Serialize(m_ProjectManager.IsProjectLoaded() ? (Freely::Project::GetActive()->GetProjectDirectory() / "Scenes/Main.fscene").string() : "Main.fscene");
            }
            if (ImGui::MenuItem("Load Scene")) {
                m_Context.ActiveScene = std::make_shared<Freely::Scene>();
                Freely::SceneSerializer serializer(*m_Context.ActiveScene);
                serializer.Deserialize(m_ProjectManager.IsProjectLoaded() ? (Freely::Project::GetActive()->GetProjectDirectory() / "Scenes/Main.fscene").string() : "Main.fscene");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) Shutdown();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void EditorApp::HandleViewportCamera(float dt) {
    if (!m_ViewportPanel->IsHovered()) return;

    auto& input = GetInput();

    if (input.IsMouseButtonDown(1)) {
        glm::vec2 delta = input.GetMouseDelta();
        m_CamYaw += delta.x * 0.15f;
        m_CamPitch -= delta.y * 0.15f;
        m_CamPitch = glm::clamp(m_CamPitch, -89.0f, 89.0f);
    }

    float scroll = input.GetScrollDelta();
    if (scroll != 0.0f) {
        m_CamPos += m_EditorCamera.GetForward() * scroll * 1.5f;
    }

    if (input.IsMouseButtonDown(2)) {
        glm::vec2 delta = input.GetMouseDelta();
        m_CamPos -= m_EditorCamera.GetRight() * delta.x * 0.01f;
        m_CamPos += m_EditorCamera.GetUp() * delta.y * 0.01f;
    }

    m_EditorCamera.SetPosition(m_CamPos);
    m_EditorCamera.SetRotation({m_CamPitch, m_CamYaw, 0.0f});

    glm::vec2 vpSize = m_ViewportPanel->GetSize();
    if (vpSize.x > 0 && vpSize.y > 0) {
        if (vpSize.x != m_ViewportFB->GetSpecification().Width || vpSize.y != m_ViewportFB->GetSpecification().Height) {
            m_ViewportFB->Resize((uint32_t)vpSize.x, (uint32_t)vpSize.y);
            m_EditorCamera.SetPerspective(60.0f, vpSize.x / vpSize.y, 0.1f, 1000.0f);
        }
    }
}

} // namespace FreelyEditor
