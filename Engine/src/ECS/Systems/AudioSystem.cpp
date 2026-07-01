#include "Freely/ECS/Systems/AudioSystem.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Audio/AudioEngine.h"

namespace Freely {

void AudioSystem::OnStart(Scene& scene) {
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<AudioSourceComponent>();

    for (auto e : view) {
        auto& ac = view.get<AudioSourceComponent>(e);
        if (ac.ClipPath.empty()) continue;

        AudioHandle h = AudioEngine::CreateSource(ac.ClipPath);
        ac.RuntimeSourceHandle = h;

        if (h == kInvalidAudio) continue;

        AudioEngine::SetLoop(h,    ac.Loop);
        AudioEngine::SetGain(h,    ac.Volume);
        AudioEngine::SetPitch(h,   ac.Pitch);
        AudioEngine::SetSpatial(h, ac.Spatial);
        AudioEngine::SetMinMaxDistance(h, ac.MinDistance, ac.MaxDistance);

        if (ac.PlayOnAwake)
            AudioEngine::Play(h);
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

void AudioSystem::OnStop(Scene& scene) {
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
