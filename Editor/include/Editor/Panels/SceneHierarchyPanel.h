#pragma once

#include "Editor/EditorContext.h"
#include <entt/entt.hpp>

namespace FreelyEditor {

class SceneHierarchyPanel {
public:
    SceneHierarchyPanel(EditorContext* context);

    void OnImGuiRender();

private:
    void DrawEntityNode(entt::entity entity);
    void HandleDragDropReparent(entt::entity targetEntity);

    EditorContext* m_Context;
};

} // namespace FreelyEditor
