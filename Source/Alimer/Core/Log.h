// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/String.h"
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace Alimer
{
    enum class LogLevel : uint32_t
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
        Off = 6,
        Count
    };

    class ALIMER_API Log final
    {
    public:
        static void Init();
        static void Shutdown();

        static bool ShouldLog(LogLevel level);
        static void Trace(const std::string& message);
        static void Debug(const std::string& message);
        static void Info(const std::string& message);
        static void Warn(const std::string& message);
        static void Error(const std::string& message);
        static void Critical(const std::string& message);

        static LogLevel GetLevel() { return level; }
        static void SetLevel(LogLevel newLevel);

    private:
        static LogLevel level;
    };
}

#define LOGV(...) spdlog::trace(__VA_ARGS__);
#define LOGD(...) spdlog::debug(__VA_ARGS__);
#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn("{}: {}", ALIMER_FUNCTION_NAME, std::format(__VA_ARGS__));

#if defined(ALIMER_ERRORS_AS_WARNINGS)
#define LOGE(...) spdlog::warn("{}: {}", ALIMER_FUNCTION_NAME, std::format(__VA_ARGS__));
#else
#define LOGE(...) do \
    { \
        spdlog::error("[{}:{}] {}: {}", __FILE__, __LINE__, ALIMER_FUNCTION_NAME, std::format(__VA_ARGS__)); \
        ALIMER_BREAKPOINT; \
        std::exit(-1); \
    } while (0)
#endif /* defined(ALIMER_ERRORS_AS_WARNINGS) */

#define LOGF(...) do \
    { \
        spdlog::critical("[{}:{}] {}: {}", __FILE__, __LINE__, ALIMER_FUNCTION_NAME, std::format(__VA_ARGS__)); \
        ALIMER_BREAKPOINT; \
        std::exit(-1); \
    } while (0)
