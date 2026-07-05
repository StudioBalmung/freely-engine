#include "Freely/Project/Project.h"
#include "Freely/Core/Logger.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Freely {

std::shared_ptr<Project> Project::s_ActiveProject;

Project::Project(const std::string& filepath) {
    Load(filepath);
}

bool Project::Load(const std::string& filepath) {
    std::ifstream stream(filepath);
    if (!stream.is_open()) {
        FL_ENGINE_ERROR("Failed to open project file '{0}'", filepath);
        return false;
    }

    json projectJson;
    try {
        stream >> projectJson;
    } catch (json::parse_error& e) {
        FL_ENGINE_ERROR("Failed to parse project file '{0}': {1}", filepath, e.what());
        return false;
    }

    m_Config.Name = projectJson.value("name", "UntitledProject");
    m_Config.Version = projectJson.value("version", "1.0.0");
    m_Config.EngineVersion = projectJson.value("engine_version", "0.4.3");
    m_Config.DefaultScene = projectJson.value("default_scene", "Scenes/Main.fscene");
    m_Config.AssetsRoot = projectJson.value("assets_root", "Assets/");

    if (projectJson.contains("scripting")) {
        auto scriptingJson = projectJson["scripting"];
        m_Config.Scripting.Language = scriptingJson.value("language", "lua");
        m_Config.Scripting.EntryPoint = scriptingJson.value("entry", "Scripts/main.lua");
    }

    if (projectJson.contains("build_targets") && projectJson["build_targets"].is_array()) {
        m_Config.BuildTargets.clear();
        for (const auto& target : projectJson["build_targets"]) {
            m_Config.BuildTargets.push_back(target.get<std::string>());
        }
    }

    if (projectJson.contains("plugins") && projectJson["plugins"].is_array()) {
        m_Config.Plugins.clear();
        for (const auto& plugin : projectJson["plugins"]) {
            m_Config.Plugins.push_back(plugin.get<std::string>());
        }
    }

    m_ProjectFilepath = filepath;
    return true;
}

bool Project::Save(const std::string& filepath) {
    json projectJson;
    projectJson["name"] = m_Config.Name;
    projectJson["version"] = m_Config.Version;
    projectJson["engine_version"] = m_Config.EngineVersion;
    projectJson["default_scene"] = m_Config.DefaultScene;
    projectJson["assets_root"] = m_Config.AssetsRoot;

    projectJson["scripting"] = {
        {"language", m_Config.Scripting.Language},
        {"entry", m_Config.Scripting.EntryPoint}
    };

    projectJson["build_targets"] = m_Config.BuildTargets;
    projectJson["plugins"] = m_Config.Plugins;

    std::ofstream fout(filepath);
    if (!fout.is_open()) {
        FL_ENGINE_ERROR("Failed to save project file '{0}'", filepath);
        return false;
    }

    fout << projectJson.dump(4);
    m_ProjectFilepath = filepath;
    return true;
}

std::filesystem::path Project::GetProjectDirectory() const {
    if (m_ProjectFilepath.empty())
        return "";
    return std::filesystem::path(m_ProjectFilepath).parent_path();
}

std::filesystem::path Project::GetAssetDirectory() const {
    return GetProjectDirectory() / m_Config.AssetsRoot;
}

std::shared_ptr<Project> Project::New() {
    s_ActiveProject = std::make_shared<Project>();
    return s_ActiveProject;
}

std::shared_ptr<Project> Project::LoadProject(const std::string& filepath) {
    std::shared_ptr<Project> project = std::make_shared<Project>();
    if (project->Load(filepath)) {
        s_ActiveProject = project;
        return s_ActiveProject;
    }
    return nullptr;
}

} // namespace Freely
