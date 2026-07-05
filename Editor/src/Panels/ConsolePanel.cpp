#include "Editor/Panels/ConsolePanel.h"
#include <Freely/Core/Logger.h>

#include <spdlog/sinks/base_sink.h>
#include <imgui.h>

namespace FreelyEditor {

// ─── Static members ───────────────────────────────────────────────────────────
std::vector<LogMessage>               ConsolePanel::s_Messages;
std::mutex                            ConsolePanel::s_MessageMutex;
std::shared_ptr<ConsoleSink_mt>       ConsolePanel::s_Sink;

// ─── Sink implementation ──────────────────────────────────────────────────────
template<typename Mutex>
void ConsoleSink<Mutex>::sink_it_(const spdlog::details::log_msg& msg) {
    // Format: [HH:MM:SS] text
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    std::string str = fmt::to_string(formatted);
    // Strip trailing newline
    if (!str.empty() && str.back() == '\n') str.pop_back();

    ConsolePanel::AddLog((int)msg.level, str);
}

// ─── Public API ───────────────────────────────────────────────────────────────
void ConsolePanel::InstallSink() {
    s_Sink = std::make_shared<ConsoleSink_mt>();
    s_Sink->set_level(spdlog::level::trace);

    // Install into both engine and app loggers
    auto& engLogger = Freely::Logger::GetEngineLogger();
    auto& appLogger = Freely::Logger::GetAppLogger();
    if (engLogger) engLogger->sinks().push_back(s_Sink);
    if (appLogger) appLogger->sinks().push_back(s_Sink);
}

void ConsolePanel::AddLog(int level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_MessageMutex);
    s_Messages.push_back({ level, message });
    if (s_Messages.size() > 2000)
        s_Messages.erase(s_Messages.begin(), s_Messages.begin() + 500);
}

void ConsolePanel::Clear() {
    std::lock_guard<std::mutex> lock(s_MessageMutex);
    s_Messages.clear();
}

ConsolePanel::ConsolePanel(EditorContext* context)
    : m_Context(context)
{}

ConsolePanel::~ConsolePanel() {}

// ─── Render ───────────────────────────────────────────────────────────────────
void ConsolePanel::OnImGuiRender() {
    ImGui::Begin("Console");

    // ── Toolbar ──────────────────────────────────────────────────────────
    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60); ImGui::Checkbox("Trace", &m_ShowTrace);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60); ImGui::Checkbox("Info",  &m_ShowInfo);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60); ImGui::Checkbox("Warn",  &m_ShowWarn);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60); ImGui::Checkbox("Error", &m_ShowError);

    ImGui::Separator();

    // ── Log list ──────────────────────────────────────────────────────────
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    {
        std::lock_guard<std::mutex> lock(s_MessageMutex);
        for (const auto& log : s_Messages) {
            // Filter
            if (log.Level == 0 && !m_ShowTrace) continue;
            if (log.Level == 1 && !m_ShowInfo)  continue; // debug
            if (log.Level == 2 && !m_ShowInfo)  continue; // info
            if (log.Level == 3 && !m_ShowWarn)  continue;
            if (log.Level >= 4 && !m_ShowError) continue;

            ImVec4 color;
            const char* prefix;
            switch (log.Level) {
                case 0: color = {0.55f, 0.55f, 0.55f, 1.f}; prefix = "[T] "; break;
                case 1: color = {0.55f, 0.55f, 0.55f, 1.f}; prefix = "[D] "; break;
                case 2: color = {1.00f, 1.00f, 1.00f, 1.f}; prefix = "[I] "; break;
                case 3: color = {1.00f, 0.90f, 0.20f, 1.f}; prefix = "[W] "; break;
                case 4: color = {1.00f, 0.35f, 0.35f, 1.f}; prefix = "[E] "; break;
                default:color = {1.00f, 0.00f, 0.60f, 1.f}; prefix = "[!] "; break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(prefix);
            ImGui::SameLine(0, 0);
            ImGui::TextUnformatted(log.Message.c_str());
            ImGui::PopStyleColor();
        }
    }

    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

} // namespace FreelyEditor
