#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FreelyEditor {

struct RecentProject {
    std::string Name;
    std::string Filepath;
    std::string LastOpened; // timestamp
};

class ProjectManager {
public:
    ProjectManager() = default;
    ~ProjectManager() = default;

    void Init();
    void OnImGuiRender(bool* isOpen);

    bool IsProjectLoaded() const { return m_ProjectLoaded; }
    
    // Static to load from command line arg
    static bool LoadProject(const std::filesystem::path& path);

private:
    void DrawNewProjectDialog();
    void DrawRecentProjectsList();
    void LoadRecentProjects();
    void SaveRecentProjects();

    bool m_ProjectLoaded = false;
    bool m_ShowNewProjectDialog = false;

    std::vector<RecentProject> m_RecentProjects;
    
    // New Project State
    char m_NewProjectName[256] = "NewProject";
    char m_NewProjectPath[512] = "";
};

} // namespace FreelyEditor
