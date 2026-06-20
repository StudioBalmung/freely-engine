#include "Editor/ProjectManager.h"
#include <Freely/Project/Project.h>
#include <Freely/Core/Logger.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace FreelyEditor {

static std::filesystem::path GetRecentProjectsFilepath() {
    // Ideally this would be %APPDATA%/.freely/recent_projects.json or ~/.freely/recent_projects.json
    // We'll put it in the editor directory for simplicity here.
    return std::filesystem::current_path() / "recent_projects.json";
}

void ProjectManager::Init() {
    LoadRecentProjects();
    // Default path for new projects
    std::string defaultPath = (std::filesystem::current_path() / "Projects").string();
    strncpy(m_NewProjectPath, defaultPath.c_str(), sizeof(m_NewProjectPath) - 1);
}

void ProjectManager::OnImGuiRender(bool* isOpen) {
    if (m_ProjectLoaded) {
        if (isOpen) *isOpen = false;
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::Begin("Freely Project Manager", isOpen, flags);

    ImGui::Columns(2, "ProjectManagerColumns");
    ImGui::SetColumnWidth(0, 250.0f);

    // Left Column: Buttons
    if (ImGui::Button("New Project", ImVec2(-1, 40))) {
        m_ShowNewProjectDialog = true;
    }
    if (ImGui::Button("Open Project", ImVec2(-1, 40))) {
        // Here we'd open a native file dialog. For stubbing, we'll just log.
        FL_ENGINE_WARN("Open Project native file dialog not fully implemented");
        // Example:
        // std::string path = FileDialogs::OpenFile("Freely Project (*.freely)\0*.freely\0");
        // if (!path.empty()) LoadProject(path);
    }
    
    ImGui::NextColumn();

    // Right Column: Recent Projects or New Project Dialog
    if (m_ShowNewProjectDialog) {
        DrawNewProjectDialog();
    } else {
        DrawRecentProjectsList();
    }

    ImGui::Columns(1);
    ImGui::End();
}

void ProjectManager::DrawNewProjectDialog() {
    ImGui::Text("Create New Project");
    ImGui::Separator();
    
    ImGui::Spacing();
    ImGui::InputText("Project Name", m_NewProjectName, sizeof(m_NewProjectName));
    ImGui::InputText("Location", m_NewProjectPath, sizeof(m_NewProjectPath));
    
    if (ImGui::Button("Browse...")) {
        // Open folder dialog
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Create", ImVec2(120, 30))) {
        std::filesystem::path projectPath = std::filesystem::path(m_NewProjectPath) / m_NewProjectName;
        std::filesystem::create_directories(projectPath);
        std::filesystem::create_directories(projectPath / "Assets");
        std::filesystem::create_directories(projectPath / "Scenes");
        std::filesystem::create_directories(projectPath / "Scripts");

        std::filesystem::path manifestPath = projectPath / (std::string(m_NewProjectName) + ".freely");
        
        auto project = Freely::Project::New();
        project->GetConfig().Name = m_NewProjectName;
        project->Save(manifestPath.string());

        m_ProjectLoaded = LoadProject(manifestPath);
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Cancel", ImVec2(120, 30))) {
        m_ShowNewProjectDialog = false;
    }
}

void ProjectManager::DrawRecentProjectsList() {
    ImGui::Text("Recent Projects");
    ImGui::Separator();

    if (m_RecentProjects.empty()) {
        ImGui::TextDisabled("No recent projects.");
    } else {
        for (size_t i = 0; i < m_RecentProjects.size(); i++) {
            auto& rp = m_RecentProjects[i];
            ImGui::PushID((int)i);
            
            if (ImGui::Selectable(rp.Name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 50))) {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    m_ProjectLoaded = LoadProject(rp.Filepath);
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", rp.Filepath.c_str());
            }
            
            ImGui::PopID();
        }
    }
}

bool ProjectManager::LoadProject(const std::filesystem::path& path) {
    if (Freely::Project::LoadProject(path.string())) {
        FL_ENGINE_INFO("Loaded project: {0}", path.string());
        return true;
    }
    return false;
}

void ProjectManager::LoadRecentProjects() {
    auto path = GetRecentProjectsFilepath();
    if (!std::filesystem::exists(path)) return;

    std::ifstream stream(path);
    json j;
    stream >> j;

    if (j.contains("recent_projects") && j["recent_projects"].is_array()) {
        for (const auto& item : j["recent_projects"]) {
            RecentProject rp;
            rp.Name = item.value("name", "Unknown");
            rp.Filepath = item.value("filepath", "");
            rp.LastOpened = item.value("last_opened", "");
            m_RecentProjects.push_back(rp);
        }
    }
}

void ProjectManager::SaveRecentProjects() {
    json j;
    json arr = json::array();
    
    for (const auto& rp : m_RecentProjects) {
        arr.push_back({
            {"name", rp.Name},
            {"filepath", rp.Filepath},
            {"last_opened", rp.LastOpened}
        });
    }
    
    j["recent_projects"] = arr;
    
    std::ofstream stream(GetRecentProjectsFilepath());
    stream << j.dump(4);
}

} // namespace FreelyEditor
