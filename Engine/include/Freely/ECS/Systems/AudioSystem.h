#pragma once
// Freely Engine - AudioSystem
// ECS ISystem that:
//   • OnStart  : loads clips + creates AudioEngine sources for every AudioSourceComponent
//   • OnUpdate : syncs 3D positions of spatial sources each frame
//   • OnStop   : destroys all runtime sources

#include "Freely/ECS/System.h"

namespace Freely {

class AudioSystem : public ISystem {
public:
    const char* GetName()     const override { return "AudioSystem"; }
    int         GetPriority() const override { return 2000; } // before Render

    void OnStart(Scene& scene)               override;
    void OnUpdate(Scene& scene, float dt)    override;
    void OnStop(Scene& scene)                override;
};

} // namespace Freely
