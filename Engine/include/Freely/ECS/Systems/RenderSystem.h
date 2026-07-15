#pragma once

#include "Freely/ECS/System.h"
#include "Freely/Scene/Camera.h"

namespace Freely {

class RenderSystem : public ISystem {
public:
    RenderSystem() = default;
    virtual ~RenderSystem() = default;

    virtual const char* GetName() const override { return "RenderSystem"; }
    virtual int GetPriority() const override { return 5000; }

    virtual void OnLateUpdate(Scene& scene, float deltaTime) override;

    void SetEditorCamera(const Camera* camera) { m_EditorCamera = camera; }
    void SetShowGrid(bool show)                { m_ShowGrid = show; }

private:
    const Camera* m_EditorCamera = nullptr;
    bool          m_ShowGrid     = true;
};

} // namespace Freely
