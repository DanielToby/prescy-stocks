#pragma once

#include <Engine/prescy_engine_platform.hpp>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace prescy {

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

#define E_TRACE(...) ::prescy::Log::getLogger()->trace(__VA_ARGS__)
#define E_DEBUG(...) ::prescy::Log::getLogger()->debug(__VA_ARGS__)
#define E_INFO(...) ::prescy::Log::getLogger()->info(__VA_ARGS__)
#define E_WARN(...) ::prescy::Log::getLogger()->warn(__VA_ARGS__)
#define E_ERROR(...) ::prescy::Log::getLogger()->error(__VA_ARGS__)
#define E_CRITICAL(...) ::prescy::Log::getLogger()->critical(__VA_ARGS__)

#define API_CALL() E_TRACE("Called " __FUNCTION__ "() : " __FILE__ ":{}", __LINE__)
