#pragma once

#include "Editor/EditorContext.h"
#include <filesystem>

namespace FreelyEditor {

class ContentBrowserPanel {
public:
    ContentBrowserPanel(EditorContext* context);
    
    void OnImGuiRender();

private:
    EditorContext* m_Context;
    std::filesystem::path m_BaseDirectory;
};

} // namespace FreelyEditor
