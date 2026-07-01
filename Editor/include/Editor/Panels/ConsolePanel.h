#pragma once
// Freely Editor 0.4.2 — ConsolePanel
// Renders spdlog output in the ImGui console window.
// The ConsoleSink registers itself into both engine and app spdlog loggers
// so every FL_ENGINE_* and FL_* call flows here automatically.

#include "Editor/EditorContext.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <vector>
#include <string>
#include <mutex>

namespace FreelyEditor {

struct LogMessage {
    int Level;          // spdlog level: 0=trace,1=debug,2=info,3=warn,4=error,5=critical
    std::string Message;
};

// ─── Custom spdlog sink ───────────────────────────────────────────────────────
template<typename Mutex>
class ConsoleSink : public spdlog::sinks::base_sink<Mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override {}
};

using ConsoleSink_mt = ConsoleSink<std::mutex>;

// ─── ConsolePanel ─────────────────────────────────────────────────────────────
class ConsolePanel {
public:
    explicit ConsolePanel(EditorContext* context);
    ~ConsolePanel();

    void OnImGuiRender();

    /// Install the console sink into spdlog loggers.  Call once after Logger::Init().
    static void InstallSink();

    static void AddLog(int level, const std::string& message);
    static void Clear();

private:
    EditorContext* m_Context;

    static std::vector<LogMessage> s_Messages;
    static std::mutex              s_MessageMutex;

    bool m_AutoScroll = true;
    bool m_ShowTrace  = false;
    bool m_ShowInfo   = true;
    bool m_ShowWarn   = true;
    bool m_ShowError  = true;

    static std::shared_ptr<ConsoleSink_mt> s_Sink;
};

} // namespace FreelyEditor
