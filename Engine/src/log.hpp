#pragma once

#include <Engine/prescy_engine_platform.hpp>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace prescyengine {

class Log {
public:
    enum class LogLevel {
        trace,
        debug,
        info,
        warn,
        error,
        critical
    };

    static void init();
    static std::shared_ptr<spdlog::logger> getLogger();

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

}

#define E_TRACE(...) ::prescyengine::Log::getLogger()->trace(__VA_ARGS__)
#define E_DEBUG(...) ::prescyengine::Log::getLogger()->debug(__VA_ARGS__)
#define E_INFO(...) ::prescyengine::Log::getLogger()->info(__VA_ARGS__)
#define E_WARN(...) ::prescyengine::Log::getLogger()->warn(__VA_ARGS__)
#define E_ERROR(...) ::prescyengine::Log::getLogger()->error(__VA_ARGS__)
#define E_CRITICAL(...) ::prescyengine::Log::getLogger()->critical(__VA_ARGS__)

#ifdef _WIN32
#define API_CALL() E_TRACE("Called " __FUNCTION__ "() : " __FILE__ ":{}", __LINE__)
#else
#define API_CALL()
#endif
