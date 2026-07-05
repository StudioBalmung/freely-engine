#pragma once
#include "Editor/EditorContext.h"
#include <filesystem>

namespace FreelyEditor {

class ContentBrowserPanel {
public:
    explicit ContentBrowserPanel(EditorContext* context);
    void OnImGuiRender();

private:
    void DrawTopBar();
    void DrawFolderTree(const std::filesystem::path& root);
    void DrawGrid();

    EditorContext*         m_Context;
    std::filesystem::path  m_BaseDirectory;
    std::filesystem::path  m_CurrentDirectory;

    float  m_ThumbnailSize = 72.0f;
    float  m_Padding       = 12.0f;
    char   m_SearchBuffer[128] = {};
};

} // namespace FreelyEditor
