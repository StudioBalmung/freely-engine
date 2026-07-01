#pragma once
// Freely Engine 0.4.2 — AssetManager
// Central registry mapping asset paths → UUID → loaded resource.
// Supports Texture2D, Mesh, Material, Font.  All loads are synchronous
// on first access; a future async queue is pre-declared.

#include "Freely/ECS/Components.h"    // UUID
#include "Freely/Renderer/Texture.h"
#include "Freely/Scene/Mesh.h"
#include "Freely/Scene/Material.h"
#include "Freely/Renderer2D/Font.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace Freely {

class AssetManager {
public:
    // ── Lifecycle ────────────────────────────────────────────────────────
    static void Init(const std::string& projectRoot = ".");
    static void Shutdown();

    // ── Textures ─────────────────────────────────────────────────────────
    /// Returns cached texture, or loads it on first call.
    static std::shared_ptr<Texture2D> GetTexture(const std::string& path);
    static std::shared_ptr<Texture2D> GetTexture(uint64_t handle);
    static uint64_t                   ImportTexture(const std::string& path);

    // ── Meshes ────────────────────────────────────────────────────────────
    static std::shared_ptr<Mesh>      GetMesh(const std::string& path);
    static std::shared_ptr<Mesh>      GetMesh(uint64_t handle);
    static uint64_t                   ImportMesh(const std::string& path);

    // ── Materials ─────────────────────────────────────────────────────────
    static std::shared_ptr<Material>  GetMaterial(const std::string& path);
    static std::shared_ptr<Material>  GetMaterial(uint64_t handle);
    static uint64_t                   ImportMaterial(const std::string& path);

    // ── Fonts ─────────────────────────────────────────────────────────────
    static std::shared_ptr<Font>      GetFont(const std::string& path, float size = 32.0f);
    static std::shared_ptr<Font>      GetFont(uint64_t handle);
    static uint64_t                   ImportFont(const std::string& path, float size = 32.0f);

    // ── Query ─────────────────────────────────────────────────────────────
    static std::string  GetPath(uint64_t handle);
    static uint64_t     GetHandle(const std::string& path);
    static bool         IsLoaded(uint64_t handle);
    static void         Unload(uint64_t handle);
    static void         UnloadAll();

    // ── Asset directory ───────────────────────────────────────────────────
    static std::string  ResolveAbsolute(const std::string& relativePath);
    static const std::string& GetProjectRoot() { return s_ProjectRoot; }

private:
    static uint64_t NextHandle();

    static std::string s_ProjectRoot;
    static uint64_t    s_NextHandle;

    // path → handle (for deduplication)
    static std::unordered_map<std::string, uint64_t>               s_PathToHandle;
    // handle → path
    static std::unordered_map<uint64_t, std::string>               s_HandleToPath;

    // Typed caches
    static std::unordered_map<uint64_t, std::shared_ptr<Texture2D>> s_Textures;
    static std::unordered_map<uint64_t, std::shared_ptr<Mesh>>      s_Meshes;
    static std::unordered_map<uint64_t, std::shared_ptr<Material>>  s_Materials;
    static std::unordered_map<uint64_t, std::shared_ptr<Font>>      s_Fonts;
};

} // namespace Freely
