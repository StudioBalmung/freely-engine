#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FreelyEditor {

struct RecentProject {
    std::string Name;
    std::string Filepath;
    std::string LastOpened; // ISO-8601 timestamp
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

    // Native OS dialogs. Returns an empty string if the user cancelled or the
    // platform has no native dialog implementation wired up yet.
    static std::string OpenFileDialog(const std::string& filterName, const std::string& filterPattern);
    static std::string OpenFolderDialog();

private:
    void DrawNewProjectDialog();
    void DrawRecentProjectsList();
    void LoadRecentProjects();
    void SaveRecentProjects();
    void TouchRecentProject(const std::string& name, const std::string& filepath);

    bool m_ProjectLoaded = false;
    bool m_ShowNewProjectDialog = false;

    std::vector<RecentProject> m_RecentProjects;

    // New Project State
    char m_NewProjectName[256] = "NewProject";
    char m_NewProjectPath[512] = "";
};

} // namespace FreelyEditor
