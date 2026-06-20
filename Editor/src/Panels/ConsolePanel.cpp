#include "Editor/Panels/ConsolePanel.h"
#include <imgui.h>

namespace FreelyEditor {

std::vector<LogMessage> ConsolePanel::s_Messages;
std::mutex ConsolePanel::s_MessageMutex;

ConsolePanel::ConsolePanel(EditorContext* context)
    : m_Context(context)
{
}

ConsolePanel::~ConsolePanel()
{
}

void ConsolePanel::AddLog(int level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_MessageMutex);
    s_Messages.push_back({ level, message });
    if (s_Messages.size() > 1000) {
        s_Messages.erase(s_Messages.begin());
    }
}

void ConsolePanel::OnImGuiRender() {
    ImGui::Begin("Console");

    if (ImGui::Button("Clear")) {
        std::lock_guard<std::mutex> lock(s_MessageMutex);
        s_Messages.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

    ImGui::Separator();

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lock(s_MessageMutex);
    for (const auto& log : s_Messages) {
        ImVec4 color;
        switch (log.Level) {
            case 0: color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); break; // Trace (Gray)
            case 1: color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break; // Info (White)
            case 2: color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break; // Warn (Yellow)
            case 3: color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break; // Error (Red)
            case 4: color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break; // Critical (Red)
            default: color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(log.Message.c_str());
        ImGui::PopStyleColor();
    }

    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}

} // namespace FreelyEditor
