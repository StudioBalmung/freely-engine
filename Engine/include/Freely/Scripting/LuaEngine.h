#pragma once
// Freely Engine - LuaEngine
// Provides a shared Lua 5.4 state with Freely ECS bindings exposed via sol2.
//
// Script lifecycle:
//   OnCreate()  - called once when scene starts
//   OnUpdate(dt)- called every frame
//   OnDestroy() - called when entity is destroyed
//   OnCollision(other) - called on physics contact
//
// Entity API exposed to Lua:
//   entity:GetPosition()         → vec3
//   entity:SetPosition(x,y,z)
//   entity:GetRotation()         → quat (w,x,y,z)
//   entity:Translate(dx,dy,dz)
//   entity:GetComponent("Tag")   → string
//   Input.IsKeyDown(key)         → bool   (GLFW key codes)
//   Input.IsMouseDown(btn)       → bool
//   Input.GetMouseDelta()        → vec2
//   Time.DeltaTime               → float
//   Log.Info(msg), Log.Warn, Log.Error

#include <string>
#include <memory>

struct lua_State;

namespace Freely {

class Scene;

class LuaEngine {
public:
    static bool  Init();
    static void  Shutdown();
    static bool  IsInitialized() { return s_Initialized; }

    /// Load and execute a script file; returns false on error.
    static bool  LoadScript(const std::string& path);

    /// Run a named Lua function with optional scene argument.
    static bool  CallFunction(const std::string& func);

    /// Bind ECS and engine APIs into the Lua state.
    static void  BindFreely(Scene& scene);

    /// Execute all ScriptComponent::OnCreate() calls.
    static void  OnSceneStart(Scene& scene);
    /// Execute all ScriptComponent::OnUpdate(dt) calls.
    static void  OnSceneUpdate(Scene& scene, float dt);
    /// Execute all ScriptComponent::OnDestroy() calls.
    static void  OnSceneStop(Scene& scene);

    static lua_State* GetState() { return s_State; }

private:
    static lua_State* s_State;
    static bool       s_Initialized;

    static void RegisterMathTypes();
    static void RegisterInputAPI();
    static void RegisterLogAPI();
    static void RegisterEntityAPI(Scene& scene);
};

} // namespace Freely
