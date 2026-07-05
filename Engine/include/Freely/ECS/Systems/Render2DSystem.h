#pragma once
// Freely Engine - Render2DSystem
// Collects SpriteRendererComponent, Text2DComponent, and Camera2DComponent
// from the scene, sorts by (SortingLayer, OrderInLayer), and submits to
// Renderer2D each frame.

#include "Freely/ECS/System.h"

namespace Freely {

class Render2DSystem : public ISystem {
public:
    Render2DSystem() = default;
    ~Render2DSystem() override = default;

    const char* GetName()     const override { return "Render2DSystem"; }
    int         GetPriority() const override { return 6000; } // after 3D

    void OnLateUpdate(Scene& scene, float deltaTime) override;

private:
    void TickAnimators(Scene& scene, float deltaTime);
    void FlushSortedSprites(Scene& scene);
    void FlushText(Scene& scene);
};

} // namespace Freely
