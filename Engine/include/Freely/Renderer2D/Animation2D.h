#pragma once
// Freely Engine 0.4.2 — Animation2D
// Flip-book frame animation over a SpriteSheet.
// Each AnimationClip stores a sequence of tile coords and a per-clip FPS.
// The Animator2D component holds the active clip set; the Render2DSystem
// ticks it every frame and writes the current frame into SpriteRendererComponent.

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace Freely {

class SpriteSheet;

// ─── A single animation clip ─────────────────────────────────────────────────
struct AnimationClip2D {
    std::string              Name;
    std::shared_ptr<SpriteSheet> Sheet;   // may differ per clip (multi-atlas)
    std::vector<glm::vec2>   Frames;      // tile coordinates in the sheet
    float                    FPS     = 12.0f;
    bool                     Loop    = true;
};

// ─── Runtime animator state ───────────────────────────────────────────────────
class Animator2D {
public:
    void AddClip(AnimationClip2D clip);
    void Play(const std::string& name);
    void Stop();
    void Pause();
    void Resume();

    /// Advance time; returns true when the frame changed (dirty flag).
    bool Tick(float deltaTime);

    const std::string& GetCurrentClipName() const { return m_CurrentClip; }
    int   GetCurrentFrameIndex()            const { return m_FrameIndex; }
    const glm::vec2& GetCurrentTileCoord()  const;
    std::shared_ptr<SpriteSheet> GetCurrentSheet() const;

    bool IsPlaying() const { return m_Playing; }
    bool IsLooping() const;

private:
    std::unordered_map<std::string, AnimationClip2D> m_Clips;
    std::string m_CurrentClip;
    int         m_FrameIndex  = 0;
    float       m_Accumulator = 0.0f;
    bool        m_Playing     = false;
};

} // namespace Freely
