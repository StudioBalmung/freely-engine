#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace Freely {

class Logger {
public:
    static void Init();
    static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
    static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; }

private:
    static std::shared_ptr<spdlog::logger> s_EngineLogger;
    static std::shared_ptr<spdlog::logger> s_AppLogger;
};

} // namespace Freely

#define FL_ENGINE_TRACE(...)    ::Freely::Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define FL_ENGINE_INFO(...)     ::Freely::Logger::GetEngineLogger()->info(__VA_ARGS__)
#define FL_ENGINE_WARN(...)     ::Freely::Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define FL_ENGINE_ERROR(...)    ::Freely::Logger::GetEngineLogger()->error(__VA_ARGS__)
#define FL_ENGINE_CRITICAL(...) ::Freely::Logger::GetEngineLogger()->critical(__VA_ARGS__)

#define FL_TRACE(...)    ::Freely::Logger::GetAppLogger()->trace(__VA_ARGS__)
#define FL_INFO(...)     ::Freely::Logger::GetAppLogger()->info(__VA_ARGS__)
#define FL_WARN(...)     ::Freely::Logger::GetAppLogger()->warn(__VA_ARGS__)
#define FL_ERROR(...)    ::Freely::Logger::GetAppLogger()->error(__VA_ARGS__)
#define FL_CRITICAL(...) ::Freely::Logger::GetAppLogger()->critical(__VA_ARGS__)
