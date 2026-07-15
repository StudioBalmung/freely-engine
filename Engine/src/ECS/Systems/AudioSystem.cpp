#include "Freely/ECS/Systems/AudioSystem.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Audio/AudioEngine.h"

namespace Freely {

void AudioSystem::OnCreate(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<AudioSourceComponent>();

    for (auto e : view) {
        auto& ac = view.get<AudioSourceComponent>(e);
        if (!ac.Spatial) continue;

        // Create audio source
        ac.RuntimeSourceHandle = AudioEngine::CreateSource(ac.ClipPath);
        if (ac.RuntimeSourceHandle == 0) continue;

        AudioEngine::SetLoop((AudioHandle)ac.RuntimeSourceHandle, ac.Loop);
        AudioEngine::SetGain((AudioHandle)ac.RuntimeSourceHandle, ac.Volume);
        AudioEngine::SetPitch((AudioHandle)ac.RuntimeSourceHandle, ac.Pitch);
        AudioEngine::SetSpatial((AudioHandle)ac.RuntimeSourceHandle, ac.Spatial);
        AudioEngine::SetMinMaxDistance((AudioHandle)ac.RuntimeSourceHandle,
                                       ac.MinDistance, ac.MaxDistance);

        if (ac.PlayOnAwake)
            AudioEngine::Play((AudioHandle)ac.RuntimeSourceHandle);
    }
}

void AudioSystem::OnUpdate(Scene& scene, float /*dt*/) {
    if (!AudioEngine::IsInitialized()) return;

    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<TransformComponent, AudioSourceComponent>();

    for (auto e : view) {
        auto [tf, ac] = view.get<TransformComponent, AudioSourceComponent>(e);
        if (!ac.RuntimeSourceHandle || !ac.Spatial) continue;
        AudioEngine::SetPosition((AudioHandle)ac.RuntimeSourceHandle, tf.Position);
    }
}

void AudioSystem::OnDestroy(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<AudioSourceComponent>();

    for (auto e : view) {
        auto& ac = view.get<AudioSourceComponent>(e);
        if (!ac.RuntimeSourceHandle) continue;
        AudioEngine::Stop((AudioHandle)ac.RuntimeSourceHandle);
        AudioEngine::DestroySource((AudioHandle)ac.RuntimeSourceHandle);
        ac.RuntimeSourceHandle = 0;
    }
}

} // namespace Freely