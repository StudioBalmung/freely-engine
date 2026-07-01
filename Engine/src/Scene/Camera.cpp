#include "Freely/Scene/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Freely {

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearPlane(nearPlane), m_FarPlane(farPlane)
{
    RecalculateProjection();
    RecalculateView();
}

void Camera::SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_IsPerspective = true;
    m_FOV = fov;
    m_AspectRatio = aspectRatio;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    RecalculateProjection();
}

void Camera::SetOrthographic(float size, float nearPlane, float farPlane) {
    m_IsPerspective = false;
    m_OrthoSize = size;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    RecalculateProjection();
}

void Camera::LookAt(const glm::vec3& target) {
    glm::vec3 direction = glm::normalize(target - m_Position);
    m_Rotation.x = glm::degrees(asin(-direction.y)); // pitch
    m_Rotation.y = glm::degrees(atan2(direction.x, direction.z)); // yaw
    RecalculateView();
}

glm::vec3 Camera::GetForward() const {
    float pitch = glm::radians(m_Rotation.x);
    float yaw = glm::radians(m_Rotation.y);
    return glm::normalize(glm::vec3(
        cos(pitch) * sin(yaw),
        -sin(pitch),
        cos(pitch) * cos(yaw)
    ));
}

glm::vec3 Camera::GetRight() const {
    return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 Camera::GetUp() const {
    return glm::normalize(glm::cross(GetRight(), GetForward()));
}

void Camera::RecalculateView() {
    glm::vec3 forward = GetForward();
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::RecalculateProjection() {
    if (m_IsPerspective) {
        m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
    } else {
        float halfSize = m_OrthoSize * 0.5f;
        float halfWidth = halfSize * m_AspectRatio;
        m_ProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfSize, halfSize, m_NearPlane, m_FarPlane);
    }
}

} // namespace Freely
