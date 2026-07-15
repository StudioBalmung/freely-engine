#pragma once

#include "Editor/EditorContext.h"

namespace FreelyEditor {

class ToolbarPanel {
public:
    ToolbarPanel(EditorContext* context);
    
    void OnImGuiRender();

private:
    EditorContext* m_Context;
};

} // namespace FreelyEditor
