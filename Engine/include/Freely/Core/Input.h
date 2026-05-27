#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Freely {

class InputManager {
public:
    InputManager(GLFWwindow* window);
    ~InputManager() = default;

    void Update();

    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;

    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;

    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    float GetScrollDelta() const;

    void SetCursorMode(bool locked);

    static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

private:
    GLFWwindow* m_Window;
    bool m_KeyState[512] = {};
    bool m_PrevKeyState[512] = {};
    bool m_MouseState[8] = {};
    bool m_PrevMouseState[8] = {};
    glm::vec2 m_MousePos = {0.0f, 0.0f};
    glm::vec2 m_PrevMousePos = {0.0f, 0.0f};
    static float s_ScrollDelta;
};

} // namespace Freely
