#pragma once

#include "Editor/EditorContext.h"
#include <entt/entt.hpp>

namespace FreelyEditor {

class InspectorPanel {
public:
    InspectorPanel(EditorContext* context);
    
    void OnImGuiRender();

private:
    void DrawComponents(entt::entity entity);

    EditorContext* m_Context;
};

} // namespace FreelyEditor
