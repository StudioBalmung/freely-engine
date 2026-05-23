#include "Editor/EditorApp.h"
#include <Freely/Freely.h>
#include <AsterCore/AsterCore.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace FreelyEditor {

EditorApp::EditorApp()
    : Engine({"Freely Editor", 1600, 900, true, false})
{
}

void EditorApp::OnInit() {
    FL_INFO("Freely Editor initialized!");

    InitImGui();

    // Create viewport framebuffer
    Freely::FramebufferSpec fbSpec;
    fbSpec.Width = 1280;
    fbSpec.Height = 720;
    m_ViewportFB = Freely::Framebuffer::Create(fbSpec);

    // PBR shader (inline)
    const char* vertSrc = R"(
        #version 450 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Normal;
        layout(location = 2) in vec2 a_TexCoords;
        layout(location = 3) in vec3 a_Tangent;

        uniform mat4 u_Model;
        uniform mat4 u_View;
        uniform mat4 u_Projection;

        out vec3 v_WorldPos;
        out vec3 v_Normal;
        out vec2 v_TexCoords;

        void main() {
            vec4 worldPos = u_Model * vec4(a_Position, 1.0);
            v_WorldPos = worldPos.xyz;
            v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
            v_TexCoords = a_TexCoords;
            gl_Position = u_Projection * u_View * worldPos;
        }
    )";

    const char* fragSrc = R"(
        #version 450 core
        in vec3 v_WorldPos;
        in vec3 v_Normal;
        in vec2 v_TexCoords;

        out vec4 FragColor;

        struct MaterialData {
            vec3 albedo;
            float metallic;
            float roughness;
            float ao;
        };

        uniform MaterialData u_Material;
        uniform vec3 u_LightPos;
        uniform vec3 u_LightColor;
        uniform vec3 u_ViewPos;

        const float PI = 3.14159265359;

        float DistributionGGX(vec3 N, vec3 H, float roughness) {
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(N, H), 0.0);
            float NdotH2 = NdotH * NdotH;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            denom = PI * denom * denom;
            return a2 / denom;
        }

        float GeometrySchlickGGX(float NdotV, float roughness) {
            float r = (roughness + 1.0);
            float k = (r * r) / 8.0;
            return NdotV / (NdotV * (1.0 - k) + k);
        }

        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
        }

        vec3 FresnelSchlick(float cosTheta, vec3 F0) {
            return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
        }

        void main() {
            vec3 N = normalize(v_Normal);
            vec3 V = normalize(u_ViewPos - v_WorldPos);
            vec3 L = normalize(u_LightPos - v_WorldPos);
            vec3 H = normalize(V + L);

            vec3 F0 = vec3(0.04);
            F0 = mix(F0, u_Material.albedo, u_Material.metallic);

            float distance = length(u_LightPos - v_WorldPos);
            float attenuation = 1.0 / (distance * distance);
            vec3 radiance = u_LightColor * attenuation * 50.0;

            float NDF = DistributionGGX(N, H, u_Material.roughness);
            float G = GeometrySmith(N, V, L, u_Material.roughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - u_Material.metallic;

            float NdotL = max(dot(N, L), 0.0);
            vec3 Lo = (kD * u_Material.albedo / PI + specular) * radiance * NdotL;

            vec3 ambient = vec3(0.03) * u_Material.albedo * u_Material.ao;
            vec3 color = ambient + Lo;

            color = color / (color + vec3(1.0));
            color = pow(color, vec3(1.0 / 2.2));

            FragColor = vec4(color, 1.0);
        }
    )";

    m_PBRShader = Freely::Shader::Create(vertSrc, fragSrc);

    // Grid shader
    const char* gridVert = R"(
        #version 450 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_View;
        uniform mat4 u_Projection;
        out vec3 v_WorldPos;
        void main() {
            v_WorldPos = a_Position;
            gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
        }
    )";

    const char* gridFrag = R"(
        #version 450 core
        in vec3 v_WorldPos;
        out vec4 FragColor;
        void main() {
            vec2 coord = v_WorldPos.xz;
            vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
            float line = min(grid.x, grid.y);
            float alpha = 1.0 - min(line, 1.0);
            float dist = length(v_WorldPos.xz);
            alpha *= max(0.0, 1.0 - dist / 50.0);

            vec3 color = vec3(0.35);
            if (abs(v_WorldPos.x) < 0.05) color = vec3(0.2, 0.2, 0.9);
            if (abs(v_WorldPos.z) < 0.05) color = vec3(0.9, 0.2, 0.2);

            FragColor = vec4(color, alpha * 0.5);
        }
    )";

    m_GridShader = Freely::Shader::Create(gridVert, gridFrag);

    // Setup editor camera
    m_EditorCamera = Freely::Camera(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    m_EditorCamera.SetPosition(m_CamPos);
    m_EditorCamera.SetRotation({m_CamPitch, m_CamYaw, 0.0f});

    // Add default scene objects
    SceneObject cube;
    cube.Name = "Cube";
    cube.MeshData = Freely::Mesh::CreateCube();
    cube.Albedo = {0.8f, 0.2f, 0.2f};
    cube.Metallic = 0.5f;
    cube.Roughness = 0.3f;
    cube.Body = std::make_shared<AsterCore::RigidBody>(AsterCore::BodyType::Dynamic);
    cube.Body->SetPosition({0.0f, 3.0f, 0.0f});
    cube.Body->SetCollider(std::make_shared<AsterCore::BoxCollider>());
    m_PhysicsWorld.AddBody(cube.Body);
    m_SceneObjects.push_back(cube);

    SceneObject sphere;
    sphere.Name = "Sphere";
    sphere.MeshData = Freely::Mesh::CreateSphere(32, 16);
    sphere.Albedo = {0.9f, 0.8f, 0.2f};
    sphere.Metallic = 0.9f;
    sphere.Roughness = 0.1f;
    sphere.Body = std::make_shared<AsterCore::RigidBody>(AsterCore::BodyType::Dynamic);
    sphere.Body->SetPosition({3.0f, 5.0f, 0.0f});
    sphere.Body->SetCollider(std::make_shared<AsterCore::SphereCollider>(0.5f));
    m_PhysicsWorld.AddBody(sphere.Body);
    m_SceneObjects.push_back(sphere);

    SceneObject ground;
    ground.Name = "Ground";
    ground.MeshData = Freely::Mesh::CreatePlane(50.0f);
    ground.Albedo = {0.3f, 0.3f, 0.35f};
    ground.Metallic = 0.0f;
    ground.Roughness = 0.9f;
    ground.Body = std::make_shared<AsterCore::RigidBody>(AsterCore::BodyType::Static);
    ground.Body->SetPosition({0.0f, 0.0f, 0.0f});
    ground.Body->SetCollider(std::make_shared<AsterCore::PlaneCollider>(glm::vec3(0, 1, 0), 0.0f));
    m_PhysicsWorld.AddBody(ground.Body);
    m_SceneObjects.push_back(ground);

    GetRenderer().SetClearColor({0.15f, 0.15f, 0.18f, 1.0f});
}

void EditorApp::OnUpdate(float dt) {
    HandleViewportCamera(dt);

    if (m_PhysicsPlaying) {
        m_PhysicsWorld.Step(dt);
    }

    if (GetInput().IsKeyPressed(GLFW_KEY_ESCAPE)) Shutdown();
    if (GetInput().IsKeyPressed(GLFW_KEY_W) && !m_ViewportFocused) m_GizmoOperation = 0;
    if (GetInput().IsKeyPressed(GLFW_KEY_E) && !m_ViewportFocused) m_GizmoOperation = 1;
    if (GetInput().IsKeyPressed(GLFW_KEY_R) && !m_ViewportFocused) m_GizmoOperation = 2;
}

void EditorApp::OnRender() {
    // Render scene to framebuffer
    m_ViewportFB->Bind();
    GetRenderer().SetClearColor({0.12f, 0.12f, 0.14f, 1.0f});
    GetRenderer().Clear();
    GetRenderer().SetDepthTest(true);

    m_PBRShader->Bind();
    m_PBRShader->SetMat4("u_View", m_EditorCamera.GetViewMatrix());
    m_PBRShader->SetMat4("u_Projection", m_EditorCamera.GetProjectionMatrix());
    m_PBRShader->SetVec3("u_ViewPos", m_EditorCamera.GetPosition());
    m_PBRShader->SetVec3("u_LightPos", {5.0f, 10.0f, 5.0f});
    m_PBRShader->SetVec3("u_LightColor", {1.0f, 1.0f, 1.0f});

    for (auto& obj : m_SceneObjects) {
        glm::mat4 model = obj.Body->GetTransformMatrix();
        m_PBRShader->SetMat4("u_Model", model);
        m_PBRShader->SetVec3("u_Material.albedo", obj.Albedo);
        m_PBRShader->SetFloat("u_Material.metallic", obj.Metallic);
        m_PBRShader->SetFloat("u_Material.roughness", obj.Roughness);
        m_PBRShader->SetFloat("u_Material.ao", 1.0f);
        GetRenderer().DrawIndexed(*obj.MeshData->GetVAO(), obj.MeshData->GetIndexCount());
    }

    // Draw grid
    GetRenderer().SetBlending(true);
    GetRenderer().SetDepthTest(true);
    auto gridPlane = Freely::Mesh::CreatePlane(100.0f);
    m_GridShader->Bind();
    m_GridShader->SetMat4("u_View", m_EditorCamera.GetViewMatrix());
    m_GridShader->SetMat4("u_Projection", m_EditorCamera.GetProjectionMatrix());
    GetRenderer().DrawIndexed(*gridPlane->GetVAO(), gridPlane->GetIndexCount());

    m_ViewportFB->Unbind();

    // Render ImGui
    GetRenderer().SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    GetRenderer().Clear();
    BeginImGuiFrame();

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
    DrawToolbar();
    DrawViewport();
    DrawSceneHierarchy();
    DrawProperties();

    ImGui::End();
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
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.38f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);

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
                m_SceneObjects.clear();
                m_SelectedObject = -1;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) Shutdown();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Cube")) {
                SceneObject obj;
                obj.Name = "Cube";
                obj.MeshData = Freely::Mesh::CreateCube();
                obj.Albedo = {0.7f, 0.7f, 0.7f};
                obj.Body = std::make_shared<AsterCore::RigidBody>(AsterCore::BodyType::Dynamic);
                obj.Body->SetPosition({0.0f, 2.0f, 0.0f});
                obj.Body->SetCollider(std::make_shared<AsterCore::BoxCollider>());
                m_PhysicsWorld.AddBody(obj.Body);
                m_SceneObjects.push_back(obj);
            }
            if (ImGui::MenuItem("Sphere")) {
                SceneObject obj;
                obj.Name = "Sphere";
                obj.MeshData = Freely::Mesh::CreateSphere(32, 16);
                obj.Albedo = {0.7f, 0.7f, 0.7f};
                obj.Body = std::make_shared<AsterCore::RigidBody>(AsterCore::BodyType::Dynamic);
                obj.Body->SetPosition({0.0f, 2.0f, 0.0f});
                obj.Body->SetCollider(std::make_shared<AsterCore::SphereCollider>(0.5f));
                m_PhysicsWorld.AddBody(obj.Body);
                m_SceneObjects.push_back(obj);
            }
            if (ImGui::MenuItem("Plane")) {
                SceneObject obj;
                obj.Name = "Plane";
                obj.MeshData = Freely::Mesh::CreatePlane(10.0f);
                obj.Albedo = {0.5f, 0.5f, 0.5f};
                obj.Body = std::make_shared<AsterCore::RigidBody>(AsterCore::BodyType::Static);
                obj.Body->SetPosition({0.0f, 0.0f, 0.0f});
                obj.Body->SetCollider(std::make_shared<AsterCore::PlaneCollider>());
                m_PhysicsWorld.AddBody(obj.Body);
                m_SceneObjects.push_back(obj);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void EditorApp::DrawToolbar() {
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec4 activeColor(0.3f, 0.6f, 0.9f, 1.0f);
    ImVec4 normalColor(0.25f, 0.25f, 0.3f, 1.0f);

    if (m_GizmoOperation == 0) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    else ImGui::PushStyleColor(ImGuiCol_Button, normalColor);
    if (ImGui::Button("Translate (W)")) m_GizmoOperation = 0;
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (m_GizmoOperation == 1) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    else ImGui::PushStyleColor(ImGuiCol_Button, normalColor);
    if (ImGui::Button("Rotate (E)")) m_GizmoOperation = 1;
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (m_GizmoOperation == 2) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    else ImGui::PushStyleColor(ImGuiCol_Button, normalColor);
    if (ImGui::Button("Scale (R)")) m_GizmoOperation = 2;
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    if (m_PhysicsPlaying) {
        if (ImGui::Button("Stop Physics")) m_PhysicsPlaying = false;
    } else {
        if (ImGui::Button("Play Physics")) m_PhysicsPlaying = true;
    }

    ImGui::End();
}

void EditorApp::DrawViewport() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x != m_ViewportSize.x || viewportSize.y != m_ViewportSize.y) {
        m_ViewportSize = {viewportSize.x, viewportSize.y};
        m_ViewportFB->Resize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
        m_EditorCamera.SetPerspective(60.0f, m_ViewportSize.x / m_ViewportSize.y, 0.1f, 1000.0f);
    }

    uint32_t texID = m_ViewportFB->GetColorAttachment();
    ImGui::Image((ImTextureID)(uint64_t)texID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

    // ImGuizmo
    if (m_SelectedObject >= 0 && m_SelectedObject < static_cast<int>(m_SceneObjects.size())) {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

        glm::mat4 view = m_EditorCamera.GetViewMatrix();
        glm::mat4 proj = m_EditorCamera.GetProjectionMatrix();
        glm::mat4 transform = m_SceneObjects[m_SelectedObject].Body->GetTransformMatrix();

        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        if (m_GizmoOperation == 1) op = ImGuizmo::ROTATE;
        if (m_GizmoOperation == 2) op = ImGuizmo::SCALE;

        if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                op, ImGuizmo::LOCAL, glm::value_ptr(transform))) {
            glm::vec3 pos, rot, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform),
                glm::value_ptr(pos), glm::value_ptr(rot), glm::value_ptr(scale));
            m_SceneObjects[m_SelectedObject].Body->SetPosition(pos);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorApp::DrawSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy");

    for (int i = 0; i < static_cast<int>(m_SceneObjects.size()); i++) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (i == m_SelectedObject) flags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", m_SceneObjects[i].Name.c_str());
        if (ImGui::IsItemClicked()) {
            m_SelectedObject = i;
        }
        if (opened) ImGui::TreePop();
    }

    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
        m_SelectedObject = -1;
    }

    ImGui::End();
}

void EditorApp::DrawProperties() {
    ImGui::Begin("Properties");

    if (m_SelectedObject >= 0 && m_SelectedObject < static_cast<int>(m_SceneObjects.size())) {
        auto& obj = m_SceneObjects[m_SelectedObject];

        char nameBuf[256];
        strncpy(nameBuf, obj.Name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
            obj.Name = nameBuf;
        }

        ImGui::Separator();
        ImGui::Text("Transform");

        glm::vec3 pos = obj.Body->GetPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f)) {
            obj.Body->SetPosition(pos);
        }

        ImGui::Separator();
        ImGui::Text("Material");
        ImGui::ColorEdit3("Albedo", glm::value_ptr(obj.Albedo));
        ImGui::SliderFloat("Metallic", &obj.Metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &obj.Roughness, 0.0f, 1.0f);

        ImGui::Separator();
        ImGui::Text("Physics");

        const char* bodyTypes[] = {"Static", "Dynamic", "Kinematic"};
        int currentType = static_cast<int>(obj.Body->GetType());
        if (ImGui::Combo("Body Type", &currentType, bodyTypes, 3)) {
            // Recreate body with new type
            auto newBody = std::make_shared<AsterCore::RigidBody>(static_cast<AsterCore::BodyType>(currentType));
            newBody->SetPosition(obj.Body->GetPosition());
            newBody->SetRotation(obj.Body->GetRotation());
            newBody->SetCollider(obj.Body->GetCollider());
            m_PhysicsWorld.RemoveBody(obj.Body);
            obj.Body = newBody;
            m_PhysicsWorld.AddBody(obj.Body);
        }

        if (!obj.Body->IsStatic()) {
            float mass = obj.Body->GetMass();
            if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.01f, 1000.0f)) {
                obj.Body->SetMass(mass);
            }

            float restitution = obj.Body->GetRestitution();
            if (ImGui::SliderFloat("Restitution", &restitution, 0.0f, 1.0f)) {
                obj.Body->SetRestitution(restitution);
            }

            float friction = obj.Body->GetFriction();
            if (ImGui::SliderFloat("Friction", &friction, 0.0f, 1.0f)) {
                obj.Body->SetFriction(friction);
            }

            glm::vec3 vel = obj.Body->GetLinearVelocity();
            ImGui::Text("Velocity: %.2f, %.2f, %.2f", vel.x, vel.y, vel.z);
        }
    } else {
        ImGui::Text("No object selected.");
    }

    ImGui::End();
}

void EditorApp::HandleViewportCamera(float dt) {
    if (!m_ViewportHovered) return;

    auto& input = GetInput();

    // Right mouse button = orbit/pan camera
    if (input.IsMouseButtonDown(1)) {
        glm::vec2 delta = input.GetMouseDelta();
        m_CamYaw += delta.x * 0.15f;
        m_CamPitch -= delta.y * 0.15f;
        m_CamPitch = glm::clamp(m_CamPitch, -89.0f, 89.0f);
    }

    // Scroll to zoom
    float scroll = input.GetScrollDelta();
    if (scroll != 0.0f) {
        m_CamPos += m_EditorCamera.GetForward() * scroll * 1.5f;
    }

    // Middle mouse = pan
    if (input.IsMouseButtonDown(2)) {
        glm::vec2 delta = input.GetMouseDelta();
        m_CamPos -= m_EditorCamera.GetRight() * delta.x * 0.01f;
        m_CamPos += m_EditorCamera.GetUp() * delta.y * 0.01f;
    }

    m_EditorCamera.SetPosition(m_CamPos);
    m_EditorCamera.SetRotation({m_CamPitch, m_CamYaw, 0.0f});
}

} // namespace FreelyEditor
