#include "Editor/Panels/ContentBrowserPanel.h"

#include <Freely/Core/Logger.h>
#include <Freely/Project/Project.h>
#include <Freely/Asset/AssetManager.h>

#include <imgui.h>
#include <filesystem>
#include <algorithm>
#include <string>

namespace fs = std::filesystem;

namespace FreelyEditor {

// ─── File type helpers ────────────────────────────────────────────────────────
enum class AssetType { Unknown, Directory, Scene, Texture, Mesh, Audio, Script, Font, Material };

static AssetType ClassifyFile(const fs::path& path) {
    if (fs::is_directory(path)) return AssetType::Directory;
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".freely")                         return AssetType::Scene;
    if (ext == ".png"  || ext == ".jpg" ||
        ext == ".jpeg" || ext == ".tga" ||
        ext == ".bmp"  || ext == ".hdr")          return AssetType::Texture;
    if (ext == ".obj"  || ext == ".fbx" ||
        ext == ".gltf" || ext == ".glb")          return AssetType::Mesh;
    if (ext == ".wav"  || ext == ".mp3" ||
        ext == ".ogg"  || ext == ".flac")         return AssetType::Audio;
    if (ext == ".lua"  || ext == ".cs")           return AssetType::Script;
    if (ext == ".ttf"  || ext == ".otf")          return AssetType::Font;
    if (ext == ".fmat")                           return AssetType::Material;
    return AssetType::Unknown;
}

struct AssetTypeStyle {
    const char* Label;
    ImVec4      Color;
};
static AssetTypeStyle GetStyle(AssetType t) {
    switch (t) {
    case AssetType::Directory: return {"  ", {0.90f, 0.80f, 0.30f, 1.f}};
    case AssetType::Scene:     return {"  ", {0.40f, 0.80f, 1.00f, 1.f}};
    case AssetType::Texture:   return {"  ", {0.80f, 0.60f, 1.00f, 1.f}};
    case AssetType::Mesh:      return {"  ", {0.30f, 0.90f, 0.60f, 1.f}};
    case AssetType::Audio:     return {"  ", {1.00f, 0.60f, 0.30f, 1.f}};
    case AssetType::Script:    return {"  ", {0.50f, 1.00f, 0.50f, 1.f}};
    case AssetType::Font:      return {"  ", {1.00f, 0.85f, 0.50f, 1.f}};
    case AssetType::Material:  return {"  ", {0.70f, 0.50f, 0.90f, 1.f}};
    default:                   return {"  ", {0.70f, 0.70f, 0.70f, 1.f}};
    }
}

// ─── Constructor / Destructor ─────────────────────────────────────────────────
ContentBrowserPanel::ContentBrowserPanel(EditorContext* context)
    : m_Context(context)
{}

// ─── Render ───────────────────────────────────────────────────────────────────
void ContentBrowserPanel::OnImGuiRender() {
    ImGui::Begin("Content Browser");

    auto project = Freely::Project::GetActive();
    if (!project) {
        ImGui::TextDisabled("No project open.");
        ImGui::End();
        return;
    }

    m_BaseDirectory = project->GetAssetDirectory();
    if (m_CurrentDirectory.empty() || !fs::exists(m_CurrentDirectory))
        m_CurrentDirectory = m_BaseDirectory;

    // ── Top bar ───────────────────────────────────────────────────────────
    DrawTopBar();

    ImGui::Separator();

    // ── Two-column layout: tree | grid ───────────────────────────────────
    ImGui::Columns(2, "ContentLayout", false);
    ImGui::SetColumnWidth(0, 200.f);

    DrawFolderTree(m_BaseDirectory);

    ImGui::NextColumn();
    DrawGrid();

    ImGui::Columns(1);
    ImGui::End();
}

// ─── Top bar ─────────────────────────────────────────────────────────────────
void ContentBrowserPanel::DrawTopBar() {
    // Back button
    bool atRoot = (m_CurrentDirectory == m_BaseDirectory);
    if (atRoot) ImGui::BeginDisabled();
    if (ImGui::Button("←")) m_CurrentDirectory = m_CurrentDirectory.parent_path();
    if (atRoot) ImGui::EndDisabled();

    ImGui::SameLine();

    // Breadcrumb
    fs::path rel = fs::relative(m_CurrentDirectory, m_BaseDirectory.parent_path());
    ImGui::TextDisabled("%s", rel.string().c_str());

    ImGui::SameLine();
    ImGui::SetNextItemWidth(160.f);
    ImGui::InputText("##Search", m_SearchBuffer, sizeof(m_SearchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("✕")) m_SearchBuffer[0] = '\0';

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120.f);
    ImGui::SliderFloat("##TSize", &m_ThumbnailSize, 40.f, 128.f, "%.0f px");
}

// ─── Folder tree (left pane) ──────────────────────────────────────────────────
void ContentBrowserPanel::DrawFolderTree(const fs::path& root) {
    for (auto& entry : fs::directory_iterator(root)) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (m_CurrentDirectory == entry.path()) flags |= ImGuiTreeNodeFlags_Selected;

        bool hasSubdirs = false;
        for (auto& sub : fs::directory_iterator(entry.path()))
            if (sub.is_directory()) { hasSubdirs = true; break; }
        if (!hasSubdirs) flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx(name.c_str(), flags, "%s %s", "▶", name.c_str());
        if (ImGui::IsItemClicked()) m_CurrentDirectory = entry.path();
        if (open) {
            DrawFolderTree(entry.path());
            ImGui::TreePop();
        }
    }
}

// ─── Asset grid (right pane) ──────────────────────────────────────────────────
void ContentBrowserPanel::DrawGrid() {
    float panelW  = ImGui::GetContentRegionAvail().x;
    float cellSz  = m_ThumbnailSize + m_Padding;
    int   columns = std::max(1, (int)(panelW / cellSz));

    ImGui::Columns(columns, nullptr, false);

    std::string filter(m_SearchBuffer);
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    try {
        for (auto& entry : fs::directory_iterator(m_CurrentDirectory)) {
            const auto& path  = entry.path();
            std::string fname = path.filename().string();

            // Filter
            if (!filter.empty()) {
                std::string lname = fname;
                std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
                if (lname.find(filter) == std::string::npos) continue;
            }

            AssetType  atype  = ClassifyFile(path);
            AssetTypeStyle sty = GetStyle(atype);

            ImGui::PushID(fname.c_str());

            // Colored icon button
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.3f,.3f,.3f,.4f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(.4f,.4f,.4f,.6f));
            ImGui::PushStyleColor(ImGuiCol_Text, sty.Color);

            ImGui::Button(sty.Label, {m_ThumbnailSize, m_ThumbnailSize});

            ImGui::PopStyleColor(4);

            // ── Drag source → scene hierarchy ──────────────────────────────
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                std::string pathStr = path.string();
                if (atype == AssetType::Scene)
                    ImGui::SetDragDropPayload("ASSET_SCENE",    pathStr.c_str(), pathStr.size()+1);
                else if (atype == AssetType::Texture)
                    ImGui::SetDragDropPayload("ASSET_TEXTURE",  pathStr.c_str(), pathStr.size()+1);
                else if (atype == AssetType::Mesh)
                    ImGui::SetDragDropPayload("ASSET_MESH",     pathStr.c_str(), pathStr.size()+1);
                else if (atype == AssetType::Audio)
                    ImGui::SetDragDropPayload("ASSET_AUDIO",    pathStr.c_str(), pathStr.size()+1);
                else if (atype == AssetType::Script)
                    ImGui::SetDragDropPayload("ASSET_SCRIPT",   pathStr.c_str(), pathStr.size()+1);
                else
                    ImGui::SetDragDropPayload("ASSET_GENERIC",  pathStr.c_str(), pathStr.size()+1);

                ImGui::PushStyleColor(ImGuiCol_Text, sty.Color);
                ImGui::Text("%s  %s", sty.Label, fname.c_str());
                ImGui::PopStyleColor();
                ImGui::EndDragDropSource();
            }

            // Double-click: navigate into dirs, or open assets
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (atype == AssetType::Directory) {
                    m_CurrentDirectory /= path.filename();
                } else if (atype == AssetType::Scene) {
                    // Tell editor to load scene
                    FL_ENGINE_INFO("ContentBrowser: open scene '{}'", path.string());
                }
            }

            // Right-click context
            if (ImGui::BeginPopupContextItem("AssetCtx")) {
                if (ImGui::MenuItem("Copy Path")) {
                    ImGui::SetClipboardText(path.string().c_str());
                }
                if (atype == AssetType::Texture || atype == AssetType::Mesh ||
                    atype == AssetType::Audio   || atype == AssetType::Font) {
                    if (ImGui::MenuItem("Import")) {
                        if (atype == AssetType::Texture)
                            Freely::AssetManager::ImportTexture(path.string());
                        else if (atype == AssetType::Mesh)
                            Freely::AssetManager::ImportMesh(path.string());
                        else if (atype == AssetType::Audio)
                            FL_ENGINE_INFO("Audio import: {}", path.string());
                        else if (atype == AssetType::Font)
                            Freely::AssetManager::ImportFont(path.string());
                    }
                }
                ImGui::EndPopup();
            }

            // Label (truncated)
            ImGui::TextUnformatted(fname.size() > 14
                ? (fname.substr(0, 12) + "…").c_str()
                : fname.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }
    } catch (const fs::filesystem_error& e) {
        ImGui::TextColored({1,0.3f,0.3f,1}, "Error: %s", e.what());
    }

    ImGui::Columns(1);
}

} // namespace FreelyEditor
