#pragma once

#include "Editor/EditorContext.h"
#include <vector>
#include <string>
#include <mutex>

namespace FreelyEditor {

struct LogMessage {
    int Level; // 0=Trace, 1=Info, 2=Warn, 3=Error, 4=Critical
    std::string Message;
};

class ConsolePanel {
public:
    ConsolePanel(EditorContext* context);
    ~ConsolePanel();
    
    void OnImGuiRender();

    static void AddLog(int level, const std::string& message);

private:
    EditorContext* m_Context;
    
    static std::vector<LogMessage> s_Messages;
    static std::mutex s_MessageMutex;
    
    bool m_AutoScroll = true;
};

} // namespace FreelyEditor
