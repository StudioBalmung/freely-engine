#include "Freely/Core/Input.h"

#include <GLFW/glfw3.h>
#include <cstring>

namespace Freely {

float InputManager::s_ScrollDelta = 0.0f;

InputManager::InputManager(GLFWwindow* window)
    : m_Window(window)
{
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetWindowUserPointer(window, this);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    m_MousePos = {static_cast<float>(mx), static_cast<float>(my)};
    m_PrevMousePos = m_MousePos;
}

void InputManager::Update() {
    std::memcpy(m_PrevKeyState, m_KeyState, sizeof(m_KeyState));
    std::memcpy(m_PrevMouseState, m_MouseState, sizeof(m_MouseState));

    for (int i = 0; i < 512; i++) {
        m_KeyState[i] = glfwGetKey(m_Window, i) == GLFW_PRESS;
    }

    for (int i = 0; i < 8; i++) {
        m_MouseState[i] = glfwGetMouseButton(m_Window, i) == GLFW_PRESS;
    }

    m_PrevMousePos = m_MousePos;
    double mx, my;
    glfwGetCursorPos(m_Window, &mx, &my);
    m_MousePos = {static_cast<float>(mx), static_cast<float>(my)};

    s_ScrollDelta = 0.0f;
}

bool InputManager::IsKeyDown(int key) const {
    return m_KeyState[key];
}

bool InputManager::IsKeyPressed(int key) const {
    return m_KeyState[key] && !m_PrevKeyState[key];
}

bool InputManager::IsKeyReleased(int key) const {
    return !m_KeyState[key] && m_PrevKeyState[key];
}

bool InputManager::IsMouseButtonDown(int button) const {
    return m_MouseState[button];
}

bool InputManager::IsMouseButtonPressed(int button) const {
    return m_MouseState[button] && !m_PrevMouseState[button];
}

bool InputManager::IsMouseButtonReleased(int button) const {
    return !m_MouseState[button] && m_PrevMouseState[button];
}

glm::vec2 InputManager::GetMousePosition() const {
    return m_MousePos;
}

glm::vec2 InputManager::GetMouseDelta() const {
    return m_MousePos - m_PrevMousePos;
}

float InputManager::GetScrollDelta() const {
    return s_ScrollDelta;
}

void InputManager::SetCursorMode(bool locked) {
    glfwSetInputMode(m_Window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void InputManager::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    s_ScrollDelta = static_cast<float>(yOffset);
}

} // namespace Freely
