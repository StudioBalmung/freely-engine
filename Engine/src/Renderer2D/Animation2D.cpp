#include "Freely/Renderer2D/Animation2D.h"
#include "Freely/Renderer2D/SpriteSheet.h"
#include "Freely/Core/Logger.h"

namespace Freely {

void Animator2D::AddClip(AnimationClip2D clip) {
    std::string name = clip.Name;
    m_Clips[name] = std::move(clip);
}

void Animator2D::Play(const std::string& name) {
    if (m_Clips.find(name) == m_Clips.end()) {
        FL_ENGINE_WARN("Animator2D: clip '{}' not found.", name);
        return;
    }
    if (m_CurrentClip == name && m_Playing) return; // already playing
    m_CurrentClip  = name;
    m_FrameIndex   = 0;
    m_Accumulator  = 0.0f;
    m_Playing      = true;
}

void Animator2D::Stop() {
    m_Playing     = false;
    m_FrameIndex  = 0;
    m_Accumulator = 0.0f;
}

void Animator2D::Pause()  { m_Playing = false; }
void Animator2D::Resume() { m_Playing = true;  }

bool Animator2D::Tick(float deltaTime) {
    if (!m_Playing || m_CurrentClip.empty()) return false;

    auto it = m_Clips.find(m_CurrentClip);
    if (it == m_Clips.end()) return false;
    const auto& clip = it->second;
    if (clip.Frames.empty()) return false;

    m_Accumulator += deltaTime;
    float frameDuration = 1.0f / clip.FPS;
    if (m_Accumulator < frameDuration) return false;

    m_Accumulator -= frameDuration;
    int prev = m_FrameIndex;
    m_FrameIndex++;

    if (m_FrameIndex >= (int)clip.Frames.size()) {
        if (clip.Loop)  m_FrameIndex = 0;
        else           { m_FrameIndex = (int)clip.Frames.size() - 1; m_Playing = false; }
    }

    return (m_FrameIndex != prev);
}

const glm::vec2& Animator2D::GetCurrentTileCoord() const {
    static const glm::vec2 kZero{0, 0};
    auto it = m_Clips.find(m_CurrentClip);
    if (it == m_Clips.end()) return kZero;
    const auto& frames = it->second.Frames;
    if (frames.empty()) return kZero;
    int idx = std::max(0, std::min(m_FrameIndex, (int)frames.size() - 1));
    return frames[idx];
}

std::shared_ptr<SpriteSheet> Animator2D::GetCurrentSheet() const {
    auto it = m_Clips.find(m_CurrentClip);
    if (it == m_Clips.end()) return nullptr;
    return it->second.Sheet;
}

bool Animator2D::IsLooping() const {
    auto it = m_Clips.find(m_CurrentClip);
    return (it != m_Clips.end()) && it->second.Loop;
}

} // namespace Freely
