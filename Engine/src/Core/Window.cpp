#include "Freely/Core/Window.h"
#include "Freely/Core/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Freely {

static bool s_GLFWInitialized = false;

Window::Window(const std::string& title, int width, int height, bool vsync, bool fullscreen)
    : m_Width(width), m_Height(height), m_VSync(vsync)
{
    if (!s_GLFWInitialized) {
        int success = glfwInit();
        if (!success) {
            FL_ENGINE_CRITICAL("Failed to initialize GLFW!");
            return;
        }
        s_GLFWInitialized = true;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_Handle = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);

    if (!m_Handle) {
        FL_ENGINE_CRITICAL("Failed to create GLFW window!");
        return;
    }

    glfwMakeContextCurrent(m_Handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        FL_ENGINE_CRITICAL("Failed to initialize GLAD!");
        return;
    }

    FL_ENGINE_INFO("OpenGL Info:");
    FL_ENGINE_INFO("  Vendor:   {}", (const char*)glGetString(GL_VENDOR));
    FL_ENGINE_INFO("  Renderer: {}", (const char*)glGetString(GL_RENDERER));
    FL_ENGINE_INFO("  Version:  {}", (const char*)glGetString(GL_VERSION));

    SetVSync(vsync);

    glfwSetWindowUserPointer(m_Handle, this);
    glfwSetFramebufferSizeCallback(m_Handle, [](GLFWwindow* window, int w, int h) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->m_Width = w;
        self->m_Height = h;
        if (self->m_ResizeCallback)
            self->m_ResizeCallback(w, h);
    });
}

Window::~Window() {
    if (m_Handle) {
        glfwDestroyWindow(m_Handle);
    }
    glfwTerminate();
    s_GLFWInitialized = false;
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_Handle);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_Handle);
}

void Window::SetVSync(bool enabled) {
    m_VSync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

} // namespace Freely
