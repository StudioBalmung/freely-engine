#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace Freely {

struct ProjectConfig {
    std::string Name = "UntitledProject";
    std::string Version = "1.0.0";
    std::string EngineVersion = "0.4.4";
    std::string DefaultScene = "Scenes/Main.fscene";
    std::string AssetsRoot = "Assets/";
    
    struct ScriptingConfig {
        std::string Language = "lua";
        std::string EntryPoint = "Scripts/main.lua";
    } Scripting;

    std::vector<std::string> BuildTargets = {"windows_x64"};
    std::vector<std::string> Plugins;
};

class Project {
public:
    Project() = default;
    Project(const std::string& filepath);

    bool Load(const std::string& filepath);
    bool Save(const std::string& filepath);

    const ProjectConfig& GetConfig() const { return m_Config; }
    ProjectConfig& GetConfig() { return m_Config; }

    std::filesystem::path GetProjectDirectory() const;
    std::filesystem::path GetAssetDirectory() const;

    static std::shared_ptr<Project> New();
    static std::shared_ptr<Project> LoadProject(const std::string& filepath);
    
    static std::shared_ptr<Project> GetActive() { return s_ActiveProject; }
    static void SetActive(std::shared_ptr<Project> project) { s_ActiveProject = project; }

private:
    ProjectConfig m_Config;
    std::string m_ProjectFilepath;

    static std::shared_ptr<Project> s_ActiveProject;
};

} // namespace Freely
