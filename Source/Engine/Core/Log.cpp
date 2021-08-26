// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/Log.h"
#ifdef __ANDROID__
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/stdout_color_sinks.h>
#endif
#include <spdlog/sinks/basic_file_sink.h>

#define LOGGER_FORMAT "[%^%l%$] %v"

namespace alimer
{
    namespace
    {
        [[nodiscard]] constexpr spdlog::level::level_enum ConvertLogLevel(LogLevel level)
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

        [[nodiscard]] constexpr LogLevel ConvertLogLevel(spdlog::level::level_enum level)
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
                return LogLevel::Off;
            }
        }

        //template<typename Mutex>
        //class MessageForwarderSink : public spdlog::sinks::base_sink<Mutex>
        //{
        //protected:
        //    void sink_it_(const spdlog::details::log_msg& msg) override
        //    {
        //        Scripting::Callback(CALLBACK_LOG_WRITE, 0, 0, (int)ConvertLogLevel(msg.level), msg.payload.data());
        //    }
        //
        //    void flush_() override { }
        //};
        //
        //using MessageForwarderSink_mt = MessageForwarderSink<std::mutex>;
        //using MessageForwarderSink_st = MessageForwarderSink<spdlog::details::null_mutex>;
    }
    Logger::Logger()
    {
        std::vector<spdlog::sink_ptr> sinks;
#ifdef __ANDROID__
        sinks.push_back(std::make_shared<spdlog::sinks::android_sink_mt>("Alimer"));
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Log.txt", true));
#else
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

#if defined(NDEBUG)
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Log.txt", true));
#endif

#endif
        //sinks.push_back(std::make_shared<MessageForwarderSink_mt>());

        auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());

#ifdef _DEBUG
        logger->set_level(spdlog::level::debug);
#else
        logger->set_level(spdlog::level::info);
#endif

        logger->set_pattern(LOGGER_FORMAT);
        spdlog::set_default_logger(logger);

#if !defined(__EMSCRIPTEN__)
        spdlog::flush_every(std::chrono::seconds(5));
#endif

        LOGI("Logger initialized");
    }

    Logger::~Logger()
    {
        spdlog::shutdown();
    }

    void Logger::Log(LogLevel level, const std::string& message)
    {
        spdlog::log(ConvertLogLevel(level), message);
    }

    void Logger::Trace(const std::string& message)
    {
        spdlog::trace(message);
    }

    void Logger::Debug(const std::string& message)
    {
        spdlog::debug(message);
    }

    void Logger::Info(const std::string& message)
    {
        spdlog::info(message);
    }

    void Logger::Warn(const std::string& message)
    {
        spdlog::warn(message);
    }

    void Logger::Error(const std::string& message)
    {
        spdlog::error(message);
    }

    void Logger::Critical(const std::string& message)
    {
        spdlog::critical(message);
    }

    Logger& gLog()
    {
        return Logger::Instance();
    }
}
