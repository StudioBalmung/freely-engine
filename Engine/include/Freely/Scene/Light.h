#pragma once

#include <glm/glm.hpp>

namespace Freely {

enum class LightType {
    Directional,
    Point,
    Spot
};

struct Light {
    LightType Type = LightType::Directional;
    glm::vec3 Position{0.0f, 5.0f, 0.0f};
    glm::vec3 Direction{0.0f, -1.0f, 0.0f};
    glm::vec3 Color{1.0f, 1.0f, 1.0f};
    float Intensity = 1.0f;

    // Attenuation (point/spot)
    float Constant = 1.0f;
    float Linear = 0.09f;
    float Quadratic = 0.032f;

    // Spotlight
    float CutOff = 12.5f;       // degrees
    float OuterCutOff = 17.5f;  // degrees
};

} // namespace Freely
