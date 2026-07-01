#include "Freely/Core/Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Freely {

std::shared_ptr<spdlog::logger> Logger::s_EngineLogger;
std::shared_ptr<spdlog::logger> Logger::s_AppLogger;

void Logger::Init() {
    spdlog::set_pattern("%^[%T] %n: %v%$");

    s_EngineLogger = spdlog::stdout_color_mt("FREELY");
    s_EngineLogger->set_level(spdlog::level::trace);

    s_AppLogger = spdlog::stdout_color_mt("APP");
    s_AppLogger->set_level(spdlog::level::trace);
}

} // namespace Freely
