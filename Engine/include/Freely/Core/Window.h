#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace Freely {

class Window {
public:
    Window(const std::string& title, int width, int height, bool vsync, bool fullscreen);
    ~Window();

    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;
    void SetVSync(bool enabled);

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    float GetAspectRatio() const { return static_cast<float>(m_Width) / static_cast<float>(m_Height); }
    GLFWwindow* GetNativeHandle() const { return m_Handle; }

    void SetResizeCallback(std::function<void(int, int)> callback) { m_ResizeCallback = callback; }

private:
    GLFWwindow* m_Handle = nullptr;
    int m_Width;
    int m_Height;
    bool m_VSync;
    std::function<void(int, int)> m_ResizeCallback;
};

} // namespace Freely
