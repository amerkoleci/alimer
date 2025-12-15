// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#if defined(ALIMER_CSHARP_BINDINGS)
#include "Alimer/CSharp/Bindings.h"

namespace Alimer::Scripting
{
    void LogCallback(LogLevel level, const char* loggerName, const char* message);
}
#endif

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#elif defined(__ANDROID__)
#include <android/log.h>
#include <spdlog/sinks/android_sink.h>
#endif
#include "spdlog/sinks/callback_sink.h"

using namespace Alimer;

namespace
{
#if defined(ALIMER_CSHARP_BINDINGS)
    constexpr LogLevel FromLogLevel(spdlog::level::level_enum level)
    {
        switch (level)
        {
        case spdlog::level::trace:
            return LogLevel::Trace;
        case spdlog::level::debug:
            return LogLevel::Debug;
        case spdlog::level::info:
            return LogLevel::Info;
        case spdlog::level::warn:
            return LogLevel::Warn;
        case spdlog::level::err:
            return LogLevel::Error;
        case spdlog::level::critical:
            return LogLevel::Critical;
        case spdlog::level::off:
            return LogLevel::Off;
        default:
            ALIMER_UNREACHABLE();
            return LogLevel::Count;
        }
    }
#endif

    constexpr spdlog::level::level_enum ConvertLogLevel(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::Trace:
                return spdlog::level::trace;
            case LogLevel::Debug:
                return spdlog::level::debug;
            case LogLevel::Info:
                return spdlog::level::info;
            case LogLevel::Warn:
                return spdlog::level::warn;
            case LogLevel::Error:
                return spdlog::level::err;
            case LogLevel::Critical:
                return spdlog::level::critical;
            case LogLevel::Off:
                return spdlog::level::off;
            default:
                ALIMER_UNREACHABLE();
                return spdlog::level::off;
        }
    }
}

#if defined(_DEBUG)
LogLevel Log::level = LogLevel::Debug;
#else
LogLevel Log::level = LogLevel::Info;
#endif

void Log::Init()
{
#if defined(_WIN32)
    {
        if (!AttachConsole(ATTACH_PARENT_PROCESS))
        {
            if (GetLastError() != ERROR_ACCESS_DENIED)
            {
                if (!AllocConsole())
                {
                    return;
                }
            }
        }

        FILE* fp;
        freopen_s(&fp, "conin$", "r", stdin);
        freopen_s(&fp, "conout$", "w", stdout);
        freopen_s(&fp, "conout$", "w", stderr);
    }
#endif

    Vector<spdlog::sink_ptr> sinks;
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#ifdef __ANDROID__
    sinks.push_back(std::make_shared<spdlog::sinks::android_sink_mt>("Alimer"));
#endif
    sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Alimer.log", true));
    //sinks.emplace_back(std::make_shared<ExternalConsoleSink>(true));

#if defined(ALIMER_CSHARP_BINDINGS)
    auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg)
    {
        Scripting::LogCallback(FromLogLevel(msg.level), msg.logger_name.data(), msg.payload.data());
    });
    sinks.emplace_back(callback_sink);
#endif

    auto logger = std::make_shared<spdlog::logger>("Alimer", sinks.begin(), sinks.end());

    logger->set_level(ConvertLogLevel(level));
    logger->set_pattern("[%^%l%$] %v");
    spdlog::set_default_logger(logger);

#if !__EMSCRIPTEN__
    spdlog::flush_every(std::chrono::seconds(5));
#endif

    spdlog::info("Log initialized");
}

void Log::Shutdown()
{
    spdlog::shutdown();
}

void Log::SetLevel(LogLevel newLevel)
{
    level = newLevel;
    spdlog::set_level(ConvertLogLevel(newLevel));
}

bool Log::ShouldLog(LogLevel level)
{
    spdlog::level::level_enum spd_level = ConvertLogLevel(level);

    return spdlog::should_log(spd_level);
}

void Log::Trace(const std::string& message)
{
    if (!spdlog::should_log(spdlog::level::trace))
        return;

    spdlog::trace(message);
}

void Log::Debug(const std::string& message)
{
    if (!spdlog::should_log(spdlog::level::debug))
        return;

    spdlog::debug(message);
}

void Log::Info(const std::string& message)
{
    if (!spdlog::should_log(spdlog::level::info))
        return;

    spdlog::info(message);
}

void Log::Warn(const std::string& message)
{
    if (!spdlog::should_log(spdlog::level::warn))
        return;

    spdlog::warn(message);
}

void Log::Error(const std::string& message)
{
    if (!spdlog::should_log(spdlog::level::err))
        return;

    spdlog::error(message);
}

void Log::Critical(const std::string& message)
{
    if (!spdlog::should_log(spdlog::level::critical))
        return;

    spdlog::critical(message);
}
