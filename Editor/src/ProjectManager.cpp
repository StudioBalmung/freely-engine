#include "Editor/ProjectManager.h"

#include <Freely/Project/Project.h>
#include <Freely/Core/Logger.h>

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>

#if defined(_WIN32)
    #define NOMINMAX
    #include <windows.h>
    #include <shobjidl.h>
#endif

using json = nlohmann::json;

namespace FreelyEditor {

namespace {
    std::filesystem::path GetRecentProjectsFilepath() {
        // Ideally this would be %APPDATA%/.freely/recent_projects.json or
        // ~/.freely/recent_projects.json. We keep it next to the editor binary
        // for now so it is easy to find while developing the editor itself.
        return std::filesystem::current_path() / "recent_projects.json";
    }

    std::string CurrentTimestamp() {
        const auto now = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm utcTime{};
#if defined(_WIN32)
        gmtime_s(&utcTime, &t);
#else
        gmtime_r(&t, &utcTime);
#endif
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
        return std::string(buffer);
    }
}

#if defined(_WIN32)

// Shared implementation for both the "open project file" and "pick a folder"
// dialogs: IFileOpenDialog handles both depending on whether FOS_PICKFOLDERS
// is set. COM is initialized and torn down locally so this works correctly
// whether or not the caller already has COM initialized on this thread.
static std::string ShowWin32FileDialog(bool pickFolders, const std::string& filterName, const std::string& filterPattern) {
    std::string result;

    HRESULT comInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool weOwnComInit = SUCCEEDED(comInit);

    IFileOpenDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dialog));

    if (SUCCEEDED(hr) && dialog != nullptr) {
        DWORD options = 0;
        dialog->GetOptions(&options);
        options |= FOS_FORCEFILESYSTEM;
        if (pickFolders) {
            options |= FOS_PICKFOLDERS;
        }
        dialog->SetOptions(options);

        if (!pickFolders && !filterPattern.empty()) {
            std::wstring wideName(filterName.begin(), filterName.end());
            std::wstring widePattern(filterPattern.begin(), filterPattern.end());

            COMDLG_FILTERSPEC filterSpec[1];
            filterSpec[0].pszName = wideName.c_str();
            filterSpec[0].pszSpec = widePattern.c_str();
            dialog->SetFileTypes(1, filterSpec);
            dialog->SetFileTypeIndex(1);
        }

        hr = dialog->Show(nullptr);

        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            hr = dialog->GetResult(&item);

            if (SUCCEEDED(hr) && item != nullptr) {
                PWSTR pathBuffer = nullptr;
                hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pathBuffer);

                if (SUCCEEDED(hr) && pathBuffer != nullptr) {
                    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, pathBuffer, -1, nullptr, 0, nullptr, nullptr);
                    if (sizeNeeded > 0) {
                        std::string utf8Path(static_cast<size_t>(sizeNeeded) - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pathBuffer, -1, utf8Path.data(), sizeNeeded, nullptr, nullptr);
                        result = utf8Path;
                    }
                    CoTaskMemFree(pathBuffer);
                }

                item->Release();
            }
        }

        dialog->Release();
    } else {
        FL_ENGINE_ERROR("ProjectManager: CoCreateInstance(CLSID_FileOpenDialog) failed, hr=0x{0:x}", static_cast<unsigned long>(hr));
    }

    if (weOwnComInit) {
        CoUninitialize();
    }

    return result;
}

#endif // _WIN32

std::string ProjectManager::OpenFileDialog(const std::string& filterName, const std::string& filterPattern) {
#if defined(_WIN32)
    return ShowWin32FileDialog(false, filterName, filterPattern);
#else
    FL_ENGINE_WARN("ProjectManager: native file dialogs are only implemented for Windows right now.");
    (void)filterName;
    (void)filterPattern;
    return std::string();
#endif
}

std::string ProjectManager::OpenFolderDialog() {
#if defined(_WIN32)
    return ShowWin32FileDialog(true, "", "");
#else
    FL_ENGINE_WARN("ProjectManager: native folder dialogs are only implemented for Windows right now.");
    return std::string();
#endif
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
        const std::string path = OpenFileDialog("Freely Project", "*.freely");
        if (!path.empty()) {
            m_ProjectLoaded = LoadProject(path);
            if (m_ProjectLoaded) {
                TouchRecentProject(Freely::Project::GetActive()->GetConfig().Name, path);
            }
        }
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
        const std::string folder = OpenFolderDialog();
        if (!folder.empty()) {
            strncpy(m_NewProjectPath, folder.c_str(), sizeof(m_NewProjectPath) - 1);
            m_NewProjectPath[sizeof(m_NewProjectPath) - 1] = '\0';
        }
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
        if (m_ProjectLoaded) {
            TouchRecentProject(m_NewProjectName, manifestPath.string());
        }
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
                    if (m_ProjectLoaded) {
                        TouchRecentProject(rp.Name, rp.Filepath);
                    }
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

void ProjectManager::TouchRecentProject(const std::string& name, const std::string& filepath) {
    m_RecentProjects.erase(
        std::remove_if(m_RecentProjects.begin(), m_RecentProjects.end(),
            [&filepath](const RecentProject& rp) { return rp.Filepath == filepath; }),
        m_RecentProjects.end());

    RecentProject entry;
    entry.Name = name;
    entry.Filepath = filepath;
    entry.LastOpened = CurrentTimestamp();
    m_RecentProjects.insert(m_RecentProjects.begin(), entry);

    SaveRecentProjects();
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

            // Drop stale entries pointing at projects that no longer exist on disk.
            if (!rp.Filepath.empty() && std::filesystem::exists(rp.Filepath)) {
                m_RecentProjects.push_back(rp);
            }
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
