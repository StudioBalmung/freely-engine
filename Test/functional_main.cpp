// Freely Engine 0.4.2 - Functional Smoke Test
//
// Exercises the real Scene/ECS/RenderSystem/Renderer3D pipeline through the
// actual Engine lifecycle (Engine::Run()), which is how RenderSystem expects
// to be driven (it calls Engine::Get().GetWindow() internally, so the
// Engine singleton must actually be alive and initialized - a bare Window
// object is not enough). Subclasses Engine, builds the village scene in
// OnInit(), captures the framebuffer after the first real frame in
// OnRender(), then calls Shutdown() to cleanly exit the loop after one pass.

#include "Freely/Core/Engine.h"
#include "Freely/Core/Window.h"
#include "Freely/Core/Logger.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>
#include <vector>
#include <cstring>

using namespace Freely;

static const int OUT_W = 1920;
static const int OUT_H = 1080;

class SmokeTestApp : public Engine {
public:
    SmokeTestApp() : Engine([]{
        EngineConfig cfg;
        cfg.windowTitle = "Freely 0.4.2 SmokeTest";
        cfg.windowWidth = 1600;
        cfg.windowHeight = 900;
        cfg.vsync = false;
        cfg.fullscreen = false;
        return cfg;
    }()) {}

protected:
    void OnInit() override {
        // Sanity check: does a raw clear+readback even work on this context?
        glClearColor(0.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        std::vector<unsigned char> testPixels(4);
        glReadPixels(GetWindow().GetWidth()/2, GetWindow().GetHeight()/2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, testPixels.data());
        std::cout << "[FunctionalSmokeTest] Raw clear+readback sanity check: ("
                  << (int)testPixels[0] << "," << (int)testPixels[1] << ","
                  << (int)testPixels[2] << "," << (int)testPixels[3] << ") - expect ~(51,102,204,255)\n";

        std::cout << "[FunctionalSmokeTest] OnInit: building village scene\n";
        m_Scene = std::make_shared<Scene>();
        m_Scene->Init();
        SetActiveScene(m_Scene);

        entt::entity camEntity = m_Scene->CreateEntity("MainCamera");
        {
            auto& tf = m_Scene->GetRegistry().Raw().get<TransformComponent>(camEntity);
            tf.Position = glm::vec3(6.0f, 4.0f, 7.0f);
            auto& cam = m_Scene->GetRegistry().Raw().emplace<CameraComponent>(camEntity);
            cam.Primary = true;
            cam.FOV = 45.0f;
            cam.NearPlane = 0.1f;
            cam.FarPlane = 200.0f;
            // Pitch/yaw computed to match Camera::GetForward()'s exact convention
            // (forward = cos(pitch)*sin(yaw), -sin(pitch), cos(pitch)*cos(yaw)) -
            // glm::quatLookAt uses a different forward-axis convention and does
            // not round-trip correctly through GetEulerAngles()/SetRotation() here.
            tf.SetEulerAngles(glm::vec3(23.45f, -139.4f, 0.0f));
        }

        entt::entity lightEntity = m_Scene->CreateEntity("Sun");
        {
            auto& tf = m_Scene->GetRegistry().Raw().get<TransformComponent>(lightEntity);
            tf.Rotation = glm::quat(glm::vec3(glm::radians(-50.0f), glm::radians(-30.0f), 0.0f));
            auto& lc = m_Scene->GetRegistry().Raw().emplace<LightComponent>(lightEntity);
            lc.Type = LightType::Directional;
            lc.Color = glm::vec3(1.0f, 0.95f, 0.85f);
            lc.Intensity = 2.0f;
        }

        const char* kitDir = "/home/claude/village/Medieval Village MegaKit[Standard]/OBJ/";
        struct Placement { const char* mesh; glm::vec3 pos; float rotY; };
        std::vector<Placement> placements = {
            {"Floor_Brick", {0, -0.02f, 0}, 0},
            {"Wall_UnevenBrick_Straight", {-2, 0, -2}, 0},
            {"Corner_Exterior_Wood", {-2, 0, -2}, 0},
            {"Roof_RoundTiles_4x4", {0, 3.0f, -1}, 0},
            {"Prop_Wagon", {1.5f, 0, 1.5f}, 25},
        };
        int created = 0;
        for (auto& p : placements) {
            entt::entity e = m_Scene->CreateEntity(p.mesh);
            auto& tf = m_Scene->GetRegistry().Raw().get<TransformComponent>(e);
            tf.Position = p.pos;
            tf.Rotation = glm::quat(glm::vec3(0, glm::radians(p.rotY), 0));
            auto& mf = m_Scene->GetRegistry().Raw().emplace<MeshFilterComponent>(e);
            mf.PrimitiveType = PrimitiveMeshType::Custom;
            mf.CustomMeshPath = std::string(kitDir) + p.mesh + ".obj";
            auto& mrc = m_Scene->GetRegistry().Raw().emplace<MeshRendererComponent>(e);
            mrc.Visible = true;
            created++;
        }
        std::cout << "[FunctionalSmokeTest] Created " << created << " mesh entities + camera + light\n";
        std::cout << "[FunctionalSmokeTest] GL: " << glGetString(GL_VERSION) << "\n";

        m_Scene->Start();
    }

    void OnRender() override {
        // Called after Scene::LateUpdate() (i.e. after RenderSystem has run)
        // and before SwapBuffers() - the default framebuffer already has the
        // rendered frame in it at this point.
        if (m_Captured) return;
        m_Captured = true;

        std::vector<unsigned char> pixels(OUT_W * OUT_H * 4);
        glReadPixels(0, 0, GetWindow().GetWidth(), GetWindow().GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        // Note: window is 1600x900 (Engine owns window sizing); write out at
        // actual window resolution rather than assuming OUT_W/OUT_H match.
        int w = GetWindow().GetWidth(), h = GetWindow().GetHeight();
        std::vector<unsigned char> flipped(w * h * 4);
        for (int y = 0; y < h; y++)
            memcpy(&flipped[y * w * 4], &pixels[(h - 1 - y) * w * 4], w * 4);
        stbi_write_png("/home/claude/freely_042_fixed/SmokeTest/functional_screenshot.png", w, h, 4, flipped.data(), w * 4);
        std::cout << "[FunctionalSmokeTest] Captured frame (" << w << "x" << h << "), requesting shutdown\n";

        Shutdown();
    }

private:
    std::shared_ptr<Scene> m_Scene;
    bool m_Captured = false;
};

int main() {
    std::cout << "[FunctionalSmokeTest] Freely 0.4.2 (fixed) - real Engine/Scene/ECS/RenderSystem path\n";
    SmokeTestApp app;
    app.Run();
    std::cout << "[FunctionalSmokeTest] DONE - engine ran end-to-end through Engine/Scene/ECS/RenderSystem/Renderer3D\n";
    return 0;
}

