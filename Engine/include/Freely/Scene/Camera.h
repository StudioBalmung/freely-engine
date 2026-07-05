#pragma once

#include <glm/glm.hpp>

namespace Freely {

class Camera {
public:
    Camera(float fov = 60.0f, float aspectRatio = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 1000.0f);
    ~Camera() = default;

    void SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
    void SetOrthographic(float size, float nearPlane, float farPlane);

    void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateView(); }
    void SetRotation(const glm::vec3& eulerAngles) { m_Rotation = eulerAngles; RecalculateView(); }

    void LookAt(const glm::vec3& target);

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetRotation() const { return m_Rotation; }
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    glm::mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }

    float GetFOV() const { return m_FOV; }
    float GetAspectRatio() const { return m_AspectRatio; }

private:
    void RecalculateView();
    void RecalculateProjection();

    glm::vec3 m_Position{0.0f, 0.0f, 3.0f};
    glm::vec3 m_Rotation{0.0f, 0.0f, 0.0f}; // pitch, yaw, roll
    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_ProjectionMatrix{1.0f};

    float m_FOV = 60.0f;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 1000.0f;
    bool m_IsPerspective = true;
    float m_OrthoSize = 10.0f;
};

} // namespace Freely
