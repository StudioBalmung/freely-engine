// Freely Engine 0.4.2 — LuaEngine implementation
// Uses the raw Lua 5.4 C API to avoid any sol2 dependency.
// All Freely APIs are registered as light C functions in the "Freely" table.

#include "Freely/Scripting/LuaEngine.h"
#include "Freely/ECS/Scene.h"
#include "Freely/ECS/Components.h"
#include "Freely/Core/Logger.h"
#include "Freely/Core/Input.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

namespace Freely {

// ─── Statics ──────────────────────────────────────────────────────────────────
lua_State* LuaEngine::s_State       = nullptr;
bool       LuaEngine::s_Initialized = false;
static Scene* s_BoundScene = nullptr;

// ─── Error handler ────────────────────────────────────────────────────────────
static int LuaPanic(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    FL_ENGINE_CRITICAL("Lua panic: {}", msg ? msg : "(null)");
    return 0;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────
bool LuaEngine::Init() {
    if (s_Initialized) return true;
    s_State = luaL_newstate();
    if (!s_State) { FL_ENGINE_ERROR("LuaEngine: luaL_newstate failed."); return false; }
    lua_atpanic(s_State, LuaPanic);
    luaL_openlibs(s_State);
    RegisterMathTypes();
    RegisterInputAPI();
    RegisterLogAPI();
    s_Initialized = true;
    FL_ENGINE_INFO("LuaEngine initialized (Lua {}).", LUA_VERSION);
    return true;
}

void LuaEngine::Shutdown() {
    if (!s_Initialized) return;
    lua_close(s_State);
    s_State       = nullptr;
    s_Initialized = false;
    s_BoundScene  = nullptr;
    FL_ENGINE_INFO("LuaEngine shutdown.");
}

// ─── Script loading ───────────────────────────────────────────────────────────
bool LuaEngine::LoadScript(const std::string& path) {
    if (!s_Initialized) return false;
    int err = luaL_loadfile(s_State, path.c_str());
    if (err) {
        FL_ENGINE_ERROR("Lua load '{}': {}", path, lua_tostring(s_State, -1));
        lua_pop(s_State, 1);
        return false;
    }
    err = lua_pcall(s_State, 0, LUA_MULTRET, 0);
    if (err) {
        FL_ENGINE_ERROR("Lua exec '{}': {}", path, lua_tostring(s_State, -1));
        lua_pop(s_State, 1);
        return false;
    }
    return true;
}

bool LuaEngine::CallFunction(const std::string& func) {
    if (!s_Initialized) return false;
    lua_getglobal(s_State, func.c_str());
    if (!lua_isfunction(s_State, -1)) { lua_pop(s_State, 1); return false; }
    if (lua_pcall(s_State, 0, 0, 0)) {
        FL_ENGINE_ERROR("Lua call '{}': {}", func, lua_tostring(s_State, -1));
        lua_pop(s_State, 1);
        return false;
    }
    return true;
}

// ─── Bind scene ───────────────────────────────────────────────────────────────
void LuaEngine::BindFreely(Scene& scene) {
    s_BoundScene = &scene;
    RegisterEntityAPI(scene);
}

// ─── Log API ──────────────────────────────────────────────────────────────────
static int lua_LogInfo(lua_State* L)  { FL_INFO("{}",  lua_tostring(L,1)); return 0; }
static int lua_LogWarn(lua_State* L)  { FL_WARN("{}",  lua_tostring(L,1)); return 0; }
static int lua_LogError(lua_State* L) { FL_ERROR("{}", lua_tostring(L,1)); return 0; }

void LuaEngine::RegisterLogAPI() {
    lua_newtable(s_State);
    lua_pushcfunction(s_State, lua_LogInfo);  lua_setfield(s_State, -2, "Info");
    lua_pushcfunction(s_State, lua_LogWarn);  lua_setfield(s_State, -2, "Warn");
    lua_pushcfunction(s_State, lua_LogError); lua_setfield(s_State, -2, "Error");
    lua_setglobal(s_State, "Log");
}

// ─── Math types ───────────────────────────────────────────────────────────────
void LuaEngine::RegisterMathTypes() {
    // Minimal vec3 constructor: vec3(x,y,z) → table{x,y,z}
    const char* mathSrc = R"LUA(
function vec3(x,y,z) return {x=x or 0, y=y or 0, z=z or 0} end
function vec2(x,y)   return {x=x or 0, y=y or 0} end
function quat(w,x,y,z) return {w=w or 1, x=x or 0, y=y or 0, z=z or 0} end
)LUA";
    luaL_dostring(s_State, mathSrc);
}

// ─── Input API ────────────────────────────────────────────────────────────────
static int lua_IsKeyDown(lua_State* L) {
    int key = (int)lua_tointeger(L, 1);
    lua_pushboolean(L, InputManager::IsKeyDown(key));
    return 1;
}
static int lua_IsMouseDown(lua_State* L) {
    int btn = (int)lua_tointeger(L, 1);
    lua_pushboolean(L, InputManager::IsMouseButtonDown(btn));
    return 1;
}
static int lua_GetMouseDelta(lua_State* L) {
    auto [dx, dy] = InputManager::GetMouseDelta();
    lua_pushnumber(L, dx);
    lua_pushnumber(L, dy);
    return 2;
}

void LuaEngine::RegisterInputAPI() {
    lua_newtable(s_State);
    lua_pushcfunction(s_State, lua_IsKeyDown);    lua_setfield(s_State, -2, "IsKeyDown");
    lua_pushcfunction(s_State, lua_IsMouseDown);  lua_setfield(s_State, -2, "IsMouseDown");
    lua_pushcfunction(s_State, lua_GetMouseDelta);lua_setfield(s_State, -2, "GetMouseDelta");
    lua_setglobal(s_State, "Input");
}

// ─── Entity API ───────────────────────────────────────────────────────────────
// Push entity ID as light userdata
static void PushEntity(lua_State* L, entt::entity e) {
    lua_pushlightuserdata(L, (void*)(uintptr_t)(uint32_t)e);
}
static entt::entity GetEntity(lua_State* L, int idx) {
    return (entt::entity)(uint32_t)(uintptr_t)lua_touserdata(L, idx);
}

static int lua_EntityGetPosition(lua_State* L) {
    entt::entity e = GetEntity(L, 1);
    if (!s_BoundScene) return 0;
    auto& reg = s_BoundScene->GetRegistry().GetEnttRegistry();
    if (!reg.all_of<TransformComponent>(e)) return 0;
    auto& tf = reg.get<TransformComponent>(e);
    lua_pushnumber(L, tf.Position.x);
    lua_pushnumber(L, tf.Position.y);
    lua_pushnumber(L, tf.Position.z);
    return 3;
}

static int lua_EntitySetPosition(lua_State* L) {
    entt::entity e = GetEntity(L, 1);
    if (!s_BoundScene) return 0;
    auto& reg = s_BoundScene->GetRegistry().GetEnttRegistry();
    if (!reg.all_of<TransformComponent>(e)) return 0;
    auto& tf = reg.get<TransformComponent>(e);
    tf.Position.x = (float)lua_tonumber(L, 2);
    tf.Position.y = (float)lua_tonumber(L, 3);
    tf.Position.z = (float)lua_tonumber(L, 4);
    tf.Dirty = true;
    return 0;
}

static int lua_EntityTranslate(lua_State* L) {
    entt::entity e = GetEntity(L, 1);
    if (!s_BoundScene) return 0;
    auto& reg = s_BoundScene->GetRegistry().GetEnttRegistry();
    if (!reg.all_of<TransformComponent>(e)) return 0;
    auto& tf = reg.get<TransformComponent>(e);
    tf.Position.x += (float)lua_tonumber(L, 2);
    tf.Position.y += (float)lua_tonumber(L, 3);
    tf.Position.z += (float)lua_tonumber(L, 4);
    tf.Dirty = true;
    return 0;
}

static int lua_FindEntityByName(lua_State* L) {
    if (!s_BoundScene) { lua_pushnil(L); return 1; }
    const char* name = lua_tostring(L, 1);
    auto e = s_BoundScene->FindEntityByName(name);
    if (e == entt::null) lua_pushnil(L);
    else PushEntity(L, e);
    return 1;
}

void LuaEngine::RegisterEntityAPI(Scene& scene) {
    // Freely table
    lua_newtable(s_State);

    lua_pushcfunction(s_State, lua_EntityGetPosition); lua_setfield(s_State, -2, "GetPosition");
    lua_pushcfunction(s_State, lua_EntitySetPosition); lua_setfield(s_State, -2, "SetPosition");
    lua_pushcfunction(s_State, lua_EntityTranslate);   lua_setfield(s_State, -2, "Translate");
    lua_pushcfunction(s_State, lua_FindEntityByName);  lua_setfield(s_State, -2, "FindEntity");

    lua_setglobal(s_State, "Freely");
}

// ─── Scene lifecycle hooks ────────────────────────────────────────────────────
void LuaEngine::OnSceneStart(Scene& scene) {
    if (!s_Initialized) return;
    BindFreely(scene);

    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<ScriptComponent>();
    for (auto e : view) {
        auto& sc = view.get<ScriptComponent>(e);
        if (sc.ScriptPath.empty() || sc.Language != "lua") continue;
        if (!LoadScript(sc.ScriptPath)) continue;

        // Call OnCreate(entity)
        lua_getglobal(s_State, "OnCreate");
        if (lua_isfunction(s_State, -1)) {
            PushEntity(s_State, e);
            if (lua_pcall(s_State, 1, 0, 0)) {
                FL_ENGINE_ERROR("Lua OnCreate: {}", lua_tostring(s_State, -1));
                lua_pop(s_State, 1);
            }
        } else lua_pop(s_State, 1);
    }
}

void LuaEngine::OnSceneUpdate(Scene& scene, float dt) {
    if (!s_Initialized) return;

    // Push Time.DeltaTime
    lua_getglobal(s_State, "Time");
    if (lua_istable(s_State, -1)) {
        lua_pushnumber(s_State, dt); lua_setfield(s_State, -2, "DeltaTime");
    }
    lua_pop(s_State, 1);

    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<ScriptComponent>();
    for (auto e : view) {
        auto& sc = view.get<ScriptComponent>(e);
        if (sc.Language != "lua") continue;

        lua_getglobal(s_State, "OnUpdate");
        if (!lua_isfunction(s_State, -1)) { lua_pop(s_State, 1); continue; }
        PushEntity(s_State, e);
        lua_pushnumber(s_State, dt);
        if (lua_pcall(s_State, 2, 0, 0)) {
            FL_ENGINE_ERROR("Lua OnUpdate: {}", lua_tostring(s_State, -1));
            lua_pop(s_State, 1);
        }
    }
}

void LuaEngine::OnSceneStop(Scene& scene) {
    if (!s_Initialized) return;
    auto& reg = scene.GetRegistry().GetEnttRegistry();
    auto view = reg.view<ScriptComponent>();
    for (auto e : view) {
        auto& sc = view.get<ScriptComponent>(e);
        if (sc.Language != "lua") continue;
        lua_getglobal(s_State, "OnDestroy");
        if (lua_isfunction(s_State, -1)) {
            PushEntity(s_State, e);
            if (lua_pcall(s_State, 1, 0, 0)) {
                FL_ENGINE_ERROR("Lua OnDestroy: {}", lua_tostring(s_State, -1));
                lua_pop(s_State, 1);
            }
        } else lua_pop(s_State, 1);
    }
}

} // namespace Freely
