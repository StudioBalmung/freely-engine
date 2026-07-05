#pragma once
// Freely Engine - AudioEngine
// Thin wrapper over miniaudio (header-only, single-file).
// Supports: WAV, MP3, FLAC, OGG via dr_libs / stb_vorbis bundled in miniaudio.
//
// Usage:
//   AudioEngine::Init();
//   auto src = AudioEngine::CreateSource("Sounds/shot.wav");
//   AudioEngine::SetGain(src, 0.8f);
//   AudioEngine::SetSpatial(src, true);
//   AudioEngine::SetPosition(src, {1, 0, 3});
//   AudioEngine::Play(src);
//   ...
//   AudioEngine::Shutdown();

#include <glm/glm.hpp>
#include <string>
#include <cstdint>

namespace Freely {

using AudioHandle = uint32_t;
static constexpr AudioHandle kInvalidAudio = 0;

class AudioEngine {
public:
    static bool Init();
    static void Shutdown();
    static void Update(const glm::vec3& listenerPos,
                       const glm::vec3& listenerForward,
                       const glm::vec3& listenerUp);

    // ── Source management ────────────────────────────────────────────────
    /// Load (or reuse cached) audio file and return a source handle.
    static AudioHandle CreateSource(const std::string& filepath);
    static void        DestroySource(AudioHandle handle);

    // ── Playback ─────────────────────────────────────────────────────────
    static void Play(AudioHandle handle);
    static void Pause(AudioHandle handle);
    static void Stop(AudioHandle handle);
    static bool IsPlaying(AudioHandle handle);

    // ── Properties ───────────────────────────────────────────────────────
    static void SetLoop(AudioHandle handle, bool loop);
    static void SetGain(AudioHandle handle, float gain);       // 0.0-1.0
    static void SetPitch(AudioHandle handle, float pitch);     // 1.0 = normal
    static void SetSpatial(AudioHandle handle, bool spatial);
    static void SetPosition(AudioHandle handle, const glm::vec3& pos);
    static void SetMinMaxDistance(AudioHandle handle, float minDist, float maxDist);

    // ── Global ────────────────────────────────────────────────────────────
    static void SetMasterVolume(float volume);
    static bool IsInitialized() { return s_Initialized; }

private:
    static bool s_Initialized;
};

} // namespace Freely
