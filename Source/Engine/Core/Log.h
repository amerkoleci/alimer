// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include "Core/Module.h"
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

    class ALIMER_API Logger final : public Module<Logger>
    {
    public:
        Logger();
        ~Logger();

         void Log(LogLevel level, const std::string& message);
         void Trace(const std::string& message);
         void Debug(const std::string& message);
         void Info(const std::string& message);
         void Warn(const std::string& message);
         void Error(const std::string& message);
         void Critical(const std::string& message);
    };

    /** Provides easier access to log module. */
    ALIMER_API Logger& gLog();
}

#define LOGT(...) spdlog::trace(__VA_ARGS__)
#define LOGD(...) spdlog::debug(__VA_ARGS__)
#define LOGI(...) spdlog::info(__VA_ARGS__)
#define LOGW(...) spdlog::warn(__VA_ARGS__)
#define LOGE(...) spdlog::error("[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define LOGF(...) do { \
		spdlog::critical("[{}:{}] {}", __FILE__, __LINE__, fmt::format(__VA_ARGS__)); \
        ALIMER_DEBUG_BREAK(); \
    } while (0)
