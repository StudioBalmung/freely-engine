// Freely Engine 0.4.2 — AudioEngine (miniaudio backend)
// miniaudio is a single-header library (no separate compile unit needed).
// We define MA_IMPLEMENTATION once here.

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "Freely/Audio/AudioEngine.h"
#include "Freely/Core/Logger.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>

namespace Freely {

// ─── Internal state ───────────────────────────────────────────────────────────
bool AudioEngine::s_Initialized = false;

namespace AudioInternal {

struct Source {
    ma_sound  Sound{};
    bool      Loaded  = false;
    bool      Spatial = false;
};

static ma_engine                                        s_Engine{};
static std::unordered_map<AudioHandle, Source>          s_Sources;
static std::unordered_map<std::string, ma_sound_group>  s_Groups;
static uint32_t                                         s_NextHandle = 1;
static std::mutex                                       s_Mutex;

} // namespace AudioInternal

// ─── Init / Shutdown ─────────────────────────────────────────────────────────
bool AudioEngine::Init() {
    ma_engine_config cfg = ma_engine_config_init();
    cfg.listenerCount = 1;

    ma_result result = ma_engine_init(&cfg, &AudioInternal::s_Engine);
    if (result != MA_SUCCESS) {
        FL_ENGINE_ERROR("AudioEngine: ma_engine_init failed ({})", (int)result);
        return false;
    }
    s_Initialized = true;
    FL_ENGINE_INFO("AudioEngine initialized (miniaudio v{}).", MA_VERSION_STRING);
    return true;
}

void AudioEngine::Shutdown() {
    if (!s_Initialized) return;
    {
        std::lock_guard<std::mutex> lock(AudioInternal::s_Mutex);
        for (auto& [h, src] : AudioInternal::s_Sources) {
            if (src.Loaded) ma_sound_uninit(&src.Sound);
        }
        AudioInternal::s_Sources.clear();
    }
    ma_engine_uninit(&AudioInternal::s_Engine);
    s_Initialized = false;
    FL_ENGINE_INFO("AudioEngine shut down.");
}

void AudioEngine::Update(const glm::vec3& pos, const glm::vec3& fwd, const glm::vec3& up) {
    if (!s_Initialized) return;
    ma_engine_listener_set_position(&AudioInternal::s_Engine, 0, pos.x, pos.y, pos.z);
    ma_engine_listener_set_direction(&AudioInternal::s_Engine, 0, fwd.x, fwd.y, fwd.z);
    ma_engine_listener_set_world_up(&AudioInternal::s_Engine, 0, up.x, up.y, up.z);
}

// ─── Source management ────────────────────────────────────────────────────────
AudioHandle AudioEngine::CreateSource(const std::string& filepath) {
    if (!s_Initialized) return kInvalidAudio;

    std::lock_guard<std::mutex> lock(AudioInternal::s_Mutex);

    AudioHandle h = AudioInternal::s_NextHandle++;
    auto& src = AudioInternal::s_Sources[h];

    ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC;
    ma_result result = ma_sound_init_from_file(
        &AudioInternal::s_Engine, filepath.c_str(), flags, nullptr, nullptr, &src.Sound);

    if (result != MA_SUCCESS) {
        FL_ENGINE_ERROR("AudioEngine: failed to load '{}' ({})", filepath, (int)result);
        AudioInternal::s_Sources.erase(h);
        return kInvalidAudio;
    }

    src.Loaded  = true;
    src.Spatial = false;
    return h;
}

void AudioEngine::DestroySource(AudioHandle handle) {
    if (!handle) return;
    std::lock_guard<std::mutex> lock(AudioInternal::s_Mutex);
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end()) return;
    if (it->second.Loaded) ma_sound_uninit(&it->second.Sound);
    AudioInternal::s_Sources.erase(it);
}

// ─── Playback ─────────────────────────────────────────────────────────────────
void AudioEngine::Play(AudioHandle handle) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_seek_to_pcm_frame(&it->second.Sound, 0);
    ma_sound_start(&it->second.Sound);
}

void AudioEngine::Pause(AudioHandle handle) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_stop(&it->second.Sound);
}

void AudioEngine::Stop(AudioHandle handle) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_stop(&it->second.Sound);
    ma_sound_seek_to_pcm_frame(&it->second.Sound, 0);
}

bool AudioEngine::IsPlaying(AudioHandle handle) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return false;
    return ma_sound_is_playing(&it->second.Sound);
}

// ─── Properties ──────────────────────────────────────────────────────────────
void AudioEngine::SetLoop(AudioHandle handle, bool loop) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_set_looping(&it->second.Sound, loop ? MA_TRUE : MA_FALSE);
}

void AudioEngine::SetGain(AudioHandle handle, float gain) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_set_volume(&it->second.Sound, gain);
}

void AudioEngine::SetPitch(AudioHandle handle, float pitch) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_set_pitch(&it->second.Sound, pitch);
}

void AudioEngine::SetSpatial(AudioHandle handle, bool spatial) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    it->second.Spatial = spatial;
    ma_sound_set_spatialization_enabled(&it->second.Sound, spatial ? MA_TRUE : MA_FALSE);
}

void AudioEngine::SetPosition(AudioHandle handle, const glm::vec3& pos) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_set_position(&it->second.Sound, pos.x, pos.y, pos.z);
}

void AudioEngine::SetMinMaxDistance(AudioHandle handle, float minDist, float maxDist) {
    auto it = AudioInternal::s_Sources.find(handle);
    if (it == AudioInternal::s_Sources.end() || !it->second.Loaded) return;
    ma_sound_set_min_distance(&it->second.Sound, minDist);
    ma_sound_set_max_distance(&it->second.Sound, maxDist);
}

void AudioEngine::SetMasterVolume(float volume) {
    if (!s_Initialized) return;
    ma_engine_set_volume(&AudioInternal::s_Engine, volume);
}

} // namespace Freely
