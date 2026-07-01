#include "Freely/Asset/AssetManager.h"
#include "Freely/Core/Logger.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace Freely {

// ─── Static member definitions ────────────────────────────────────────────────
std::string AssetManager::s_ProjectRoot = ".";
uint64_t    AssetManager::s_NextHandle  = 1;

std::unordered_map<std::string, uint64_t>                AssetManager::s_PathToHandle;
std::unordered_map<uint64_t,    std::string>             AssetManager::s_HandleToPath;
std::unordered_map<uint64_t, std::shared_ptr<Texture2D>> AssetManager::s_Textures;
std::unordered_map<uint64_t, std::shared_ptr<Mesh>>      AssetManager::s_Meshes;
std::unordered_map<uint64_t, std::shared_ptr<Material>>  AssetManager::s_Materials;
std::unordered_map<uint64_t, std::shared_ptr<Font>>      AssetManager::s_Fonts;

// ─── Lifecycle ────────────────────────────────────────────────────────────────
void AssetManager::Init(const std::string& projectRoot) {
    s_ProjectRoot = projectRoot;
    FL_ENGINE_INFO("AssetManager initialized. Root: '{}'", projectRoot);
}

void AssetManager::Shutdown() {
    UnloadAll();
    FL_ENGINE_INFO("AssetManager shutdown.");
}

uint64_t AssetManager::NextHandle() {
    return s_NextHandle++;
}

std::string AssetManager::ResolveAbsolute(const std::string& rel) {
    if (fs::path(rel).is_absolute()) return rel;
    return (fs::path(s_ProjectRoot) / rel).string();
}

// ─── Generic helpers ──────────────────────────────────────────────────────────
std::string AssetManager::GetPath(uint64_t handle) {
    auto it = s_HandleToPath.find(handle);
    return (it != s_HandleToPath.end()) ? it->second : "";
}

uint64_t AssetManager::GetHandle(const std::string& path) {
    auto it = s_PathToHandle.find(path);
    return (it != s_PathToHandle.end()) ? it->second : 0;
}

bool AssetManager::IsLoaded(uint64_t handle) {
    return s_Textures.count(handle) || s_Meshes.count(handle)
        || s_Materials.count(handle) || s_Fonts.count(handle);
}

void AssetManager::Unload(uint64_t handle) {
    s_Textures.erase(handle);
    s_Meshes.erase(handle);
    s_Materials.erase(handle);
    s_Fonts.erase(handle);
    auto path = s_HandleToPath[handle];
    s_HandleToPath.erase(handle);
    if (!path.empty()) s_PathToHandle.erase(path);
}

void AssetManager::UnloadAll() {
    s_Textures.clear(); s_Meshes.clear();
    s_Materials.clear(); s_Fonts.clear();
    s_PathToHandle.clear(); s_HandleToPath.clear();
}

// ─── Textures ─────────────────────────────────────────────────────────────────
uint64_t AssetManager::ImportTexture(const std::string& path) {
    auto norm = fs::path(path).lexically_normal().string();
    auto it = s_PathToHandle.find(norm);
    if (it != s_PathToHandle.end()) return it->second;

    uint64_t h = NextHandle();
    s_PathToHandle[norm] = h;
    s_HandleToPath[h]   = norm;
    return h;
}

std::shared_ptr<Texture2D> AssetManager::GetTexture(const std::string& path) {
    return GetTexture(ImportTexture(path));
}

std::shared_ptr<Texture2D> AssetManager::GetTexture(uint64_t handle) {
    if (!handle) return nullptr;
    auto it = s_Textures.find(handle);
    if (it != s_Textures.end()) return it->second;

    std::string path = ResolveAbsolute(GetPath(handle));
    if (path.empty() || !fs::exists(path)) {
        FL_ENGINE_WARN("AssetManager: texture not found '{}'", path);
        return nullptr;
    }
    auto tex = std::make_shared<Texture2D>(path);
    s_Textures[handle] = tex;
    return tex;
}

// ─── Meshes ───────────────────────────────────────────────────────────────────
uint64_t AssetManager::ImportMesh(const std::string& path) {
    auto norm = fs::path(path).lexically_normal().string();
    auto it = s_PathToHandle.find(norm);
    if (it != s_PathToHandle.end()) return it->second;
    uint64_t h = NextHandle();
    s_PathToHandle[norm] = h; s_HandleToPath[h] = norm;
    return h;
}

std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& path) {
    return GetMesh(ImportMesh(path));
}

std::shared_ptr<Mesh> AssetManager::GetMesh(uint64_t handle) {
    if (!handle) return nullptr;
    auto it = s_Meshes.find(handle);
    if (it != s_Meshes.end()) return it->second;

    std::string path = ResolveAbsolute(GetPath(handle));
    if (path.empty() || !fs::exists(path)) {
        FL_ENGINE_WARN("AssetManager: mesh not found '{}'", path);
        return nullptr;
    }
    auto mesh = Mesh::LoadFromFile(path);
    if (mesh) s_Meshes[handle] = mesh;
    return mesh;
}

// ─── Materials ────────────────────────────────────────────────────────────────
uint64_t AssetManager::ImportMaterial(const std::string& path) {
    auto norm = fs::path(path).lexically_normal().string();
    auto it = s_PathToHandle.find(norm);
    if (it != s_PathToHandle.end()) return it->second;
    uint64_t h = NextHandle();
    s_PathToHandle[norm] = h; s_HandleToPath[h] = norm;
    return h;
}

std::shared_ptr<Material> AssetManager::GetMaterial(const std::string& path) {
    return GetMaterial(ImportMaterial(path));
}

std::shared_ptr<Material> AssetManager::GetMaterial(uint64_t handle) {
    if (!handle) return nullptr;
    auto it = s_Materials.find(handle);
    if (it != s_Materials.end()) return it->second;
    // TODO: deserialize .fmat JSON
    auto mat = std::make_shared<Material>();
    s_Materials[handle] = mat;
    return mat;
}

// ─── Fonts ────────────────────────────────────────────────────────────────────
uint64_t AssetManager::ImportFont(const std::string& path, float size) {
    // Key: path + size to allow same font at different sizes
    std::string key = path + "@" + std::to_string((int)size);
    auto it = s_PathToHandle.find(key);
    if (it != s_PathToHandle.end()) return it->second;
    uint64_t h = NextHandle();
    s_PathToHandle[key] = h; s_HandleToPath[h] = key;
    return h;
}

std::shared_ptr<Font> AssetManager::GetFont(const std::string& path, float size) {
    return GetFont(ImportFont(path, size));
}

std::shared_ptr<Font> AssetManager::GetFont(uint64_t handle) {
    if (!handle) return nullptr;
    auto it = s_Fonts.find(handle);
    if (it != s_Fonts.end()) return it->second;

    std::string key = GetPath(handle); // "path@size"
    auto at = key.rfind('@');
    if (at == std::string::npos) return nullptr;
    std::string path = ResolveAbsolute(key.substr(0, at));
    float size = std::stof(key.substr(at + 1));

    if (!fs::exists(path)) { FL_ENGINE_WARN("AssetManager: font not found '{}'", path); return nullptr; }
    auto font = std::make_shared<Font>(path, size);
    s_Fonts[handle] = font;
    return font;
}

} // namespace Freely
