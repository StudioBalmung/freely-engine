#pragma once

#include "Freely/ECS/System.h"
#include "Freely/Scene/Camera.h"

namespace Freely {

class RenderSystem : public ISystem {
public:
    RenderSystem() = default;
    virtual ~RenderSystem() = default;

    virtual const char* GetName() const override { return "RenderSystem"; }
    virtual int GetPriority() const override { return 5000; } // Render late

    // We override OnLateUpdate to do the actual rendering
    virtual void OnLateUpdate(Scene& scene, float deltaTime) override;

    // Set the editor camera when rendering from the editor viewport
    void SetEditorCamera(const Camera* camera) { m_EditorCamera = camera; }

private:
    const Camera* m_EditorCamera = nullptr;
};

} // namespace Freely
