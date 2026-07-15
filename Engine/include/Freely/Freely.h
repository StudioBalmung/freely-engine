#pragma once
// Freely Engine - Master include

// --- Core --------------------------------------------------------------------
#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Input.h"
#include "Core/Logger.h"

// --- ECS ---------------------------------------------------------------------
#include "ECS/Components.h"
#include "ECS/Scene.h"
#include "ECS/System.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/Render2DSystem.h"

// --- 3D Renderer -------------------------------------------------------------
#include "Renderer/Renderer.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Texture.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/UniformBuffer.h"

// --- 2D Renderer -------------------------------------------------------------
#include "Renderer2D/Renderer2D.h"
#include "Renderer2D/Font.h"
#include "Renderer2D/SpriteSheet.h"
#include "Renderer2D/Animation2D.h"

// --- Scene -------------------------------------------------------------------
#include "Scene/Camera.h"
#include "Scene/Mesh.h"
#include "Scene/Material.h"
#include "Scene/Light.h"

// --- Asset -------------------------------------------------------------------
#include "Asset/AssetManager.h"

// --- Audio -------------------------------------------------------------------
#include "Audio/AudioEngine.h"

// --- Physics (ECS system) ----------------------------------------------------
#include "ECS/Systems/PhysicsSystem.h"

// --- ECS Systems (complete set) -----------------------------------------------
#include "ECS/Systems/AudioSystem.h"
#include "ECS/Systems/PhysicsSystem.h"

// --- Scripting ----------------------------------------------------------------
#include "Scripting/LuaEngine.h"
