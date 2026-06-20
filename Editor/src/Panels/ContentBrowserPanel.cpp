#include "Editor/Panels/ContentBrowserPanel.h"
#include <Freely/Project/Project.h>
#include <imgui.h>

namespace FreelyEditor {

ContentBrowserPanel::ContentBrowserPanel(EditorContext* context)
    : m_Context(context)
{
}

void ContentBrowserPanel::OnImGuiRender() {
    ImGui::Begin("Content Browser");

    auto project = Freely::Project::GetActive();
    if (!project) {
        ImGui::Text("No project open.");
        ImGui::End();
        return;
    }

    m_BaseDirectory = project->GetAssetDirectory();

    if (m_Context->CurrentContentBrowserDirectory.empty() || 
        !std::filesystem::exists(m_Context->CurrentContentBrowserDirectory)) {
        m_Context->CurrentContentBrowserDirectory = m_BaseDirectory;
    }

    if (m_Context->CurrentContentBrowserDirectory != m_BaseDirectory) {
        if (ImGui::Button("<- Back")) {
            m_Context->CurrentContentBrowserDirectory = m_Context->CurrentContentBrowserDirectory.parent_path();
        }
    }

    static float padding = 16.0f;
    static float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    for (auto& directoryEntry : std::filesystem::directory_iterator(m_Context->CurrentContentBrowserDirectory)) {
        const auto& path = directoryEntry.path();
        auto relativePath = std::filesystem::relative(path, m_BaseDirectory);
        std::string filenameString = relativePath.filename().string();

        ImGui::PushID(filenameString.c_str());
        
        // Placeholder for an icon
        ImGui::Button(directoryEntry.is_directory() ? "[Dir]" : "[File]", { thumbnailSize, thumbnailSize });

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (directoryEntry.is_directory()) {
                m_Context->CurrentContentBrowserDirectory /= path.filename();
            } else {
                // Open file based on extension
            }
        }
        
        ImGui::TextWrapped("%s", filenameString.c_str());

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
    
    ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);

    ImGui::End();
}

} // namespace FreelyEditor
