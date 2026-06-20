#pragma once

#include <Freely/ECS/Scene.h>
#include <Freely/Project/Project.h>
#include <memory>
#include <filesystem>

namespace FreelyEditor {

enum class EditorState {
    Edit = 0,
    Play = 1,
    Simulate = 2
};

struct EditorContext {
    // Current Active Scene
    std::shared_ptr<Freely::Scene> ActiveScene;
    std::shared_ptr<Freely::Scene> EditorScene;
    
    // Selection state
    entt::entity SelectedEntity = entt::null;
    
    // Gizmo state
    int GizmoType = -1; // -1 = none, 0 = translate, 1 = rotate, 2 = scale
    
    // Editor layout state
    EditorState State = EditorState::Edit;
    
    // File system state
    std::filesystem::path CurrentContentBrowserDirectory;
};

} // namespace FreelyEditor
