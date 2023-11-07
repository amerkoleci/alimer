// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef __APPLE__
#   include <TargetConditionals.h>
#endif

#if defined(__ANDROID__)
#   include <android/log.h>
#elif TARGET_OS_IOS || TARGET_OS_TV
#   include <sys/syslog.h>
#elif TARGET_OS_MAC || defined(__linux__)
#   include <unistd.h>
#   include <vector>
#elif defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <Windows.h>
#   include <strsafe.h>
#   include <memory>
#elif defined(__EMSCRIPTEN__)
#  include <emscripten.h>
#endif

void DefaultLogCallback(LogLevel level, const char* message, void* userData);

#if defined(_DEBUG)
LogLevel s_logLevel = LogLevel_Debug;
#else
LogLevel s_logLevel = LogLevel_Info;
#endif
AlimerLogCallback s_logCallback = DefaultLogCallback;
void* s_logUserData = nullptr;

static const char* s_logLevelPrefixes[LogLevel_Off+1] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
    "OFF"
};

LogLevel Alimer_GetLogLevel(void)
{
    return s_logLevel;
}

void Alimer_SetLogLevel(LogLevel level)
{
    s_logLevel = level;
}

void Alimer_SetLogCallback(AlimerLogCallback callback, void* userData)
{
    s_logCallback = callback;
    s_logUserData = userData;
}

Bool32 Alimer_ShouldLog(LogLevel level)
{
    if (!s_logCallback || s_logLevel == LogLevel_Off)
        return false;

    return level >= s_logLevel;
}

void Alimer_Log(LogLevel level, const char* message)
{
    if (!Alimer_ShouldLog(level))
        return;

    s_logCallback(level, message, s_logUserData);
}

void Alimer_LogInfo(const char* format, ...)
{
    if (!Alimer_ShouldLog(LogLevel_Info))
        return;

    char message[ALIMER_MAX_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(LogLevel_Info, message, s_logUserData);
}

#if defined(__ANDROID__)
constexpr int GetPriority(LogLevel level) noexcept
{
    switch (level)
    {
        case LogLevel_Trace:    return ANDROID_LOG_VERBOSE;
        case LogLevel_Debug:    return ANDROID_LOG_DEBUG;
        case LogLevel_Info:     return ANDROID_LOG_INFO;
        case LogLevel_Warn:     return ANDROID_LOG_WARN;
        case LogLevel_Error:    return ANDROID_LOG_ERROR;
        case LogLevel_Fatal:    return ANDROID_LOG_FATAL;
        default:                return ANDROID_LOG_DEFAULT;
    }
}
#elif TARGET_OS_IOS || TARGET_OS_TV
constexpr int GetPriority(LogLevel level) noexcept
{
    switch (level)
    {
        case LogLevel_Trace:    return 0;
        case LogLevel_Debug:    return LOG_DEBUG;
        case LogLevel_Info:     return LOG_INFO;
        case LogLevel_Warn:     return LOG_WARNING;
        case LogLevel_Error:    return LOG_ERR;
        case LogLevel_Fatal:    return LOG_CRIT;
        default: return 0;
    }
}
#elif TARGET_OS_MAC || defined(__linux__)
constexpr int GetPriority(LogLevel level) noexcept
{
    switch (level)
    {
        case LogLevel_Trace:
        case LogLevel_Debug:
        case LogLevel_Info:
            return STDOUT_FILENO;
        case LogLevel_Warn:
        case LogLevel_Error:
        case LogLevel_Fatal:
            return STDERR_FILENO;
        default:
            return STDOUT_FILENO;
    }
}
#elif defined(__EMSCRIPTEN__)
constexpr int GetLogFlags(LogLevel level) noexcept
{
    switch (level)
    {
        case LogLevel_Info:     return EM_LOG_CONSOLE | EM_LOG_INFO;
        case LogLevel_Warn:     return EM_LOG_CONSOLE | EM_LOG_WARN;
        case LogLevel_Error:    return EM_LOG_CONSOLE | EM_LOG_ERROR;
        case LogLevel_Fatal:    return EM_LOG_CONSOLE | EM_LOG_ERROR | EM_LOG_C_STACK | EM_LOG_JS_STACK;
        default:
            return EM_LOG_CONSOLE
    }
}
#elif defined(_WIN32) && defined(_DEBUG)
static WORD LogLevelColors[LogLevel_Off + 1] = {
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,        // white
    FOREGROUND_GREEN | FOREGROUND_BLUE,                         // cyan
    FOREGROUND_GREEN,                                           // green
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,   // intense yellow
    FOREGROUND_RED | FOREGROUND_INTENSITY,                      // intense red
    BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // intense white on red background
    0
};

inline WORD SetForegroundColor(HANDLE handle, WORD attribs)
{
    CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
    if (!::GetConsoleScreenBufferInfo(handle, &orig_buffer_info))
    {
        // just return white if failed getting console info
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    // change only the foreground bits (lowest 4 bits)
    auto new_attribs = attribs | (orig_buffer_info.wAttributes & 0xfff0);
    auto ignored = ::SetConsoleTextAttribute(handle, new_attribs);
    (void)(ignored);
    return orig_buffer_info.wAttributes; // return orig attribs
}

inline void WriteText(HANDLE handle, const char* text)
{
    auto ignored = ::WriteConsoleA(handle, text, static_cast<DWORD>(strlen(text)), nullptr, nullptr);
    (void)(ignored);
}
#endif

void DefaultLogCallback(LogLevel level, const char* message, void* userData)
{
    ALIMER_UNUSED(userData);

#if defined(__ANDROID__)
    __android_log_print(GetPriority(level), "Alimer", "%s", message);
#elif TARGET_OS_IOS || TARGET_OS_TV
    syslog(GetPriority(level), "%s", message);
#elif TARGET_OS_MAC || defined(__linux__)
    const int fd = GetPriority(level);
    std::vector<char> output(str.begin(), str.end());
    output.push_back('\n');

    std::size_t offset = 0;
    while (offset < output.size())
    {
        auto written = write(fd, output.data() + offset, output.size() - offset);
        while (written == -1 && errno == EINTR)
            written = write(fd, output.data() + offset, output.size() - offset);

        if (written == -1)
            return;

        offset += static_cast<std::size_t>(written);
    }
#elif defined(__EMSCRIPTEN__)
    emscripten_log(GetLogFlags(level), "%s", message);
#elif defined(_WIN32)
    const auto charCount = MultiByteToWideChar(CP_UTF8, 0, message, -1, nullptr, 0);
    if (charCount == 0)
        return;

    auto buffer = std::make_unique<WCHAR[]>(static_cast<std::size_t>(charCount) + 1); // +1 for the newline
    if (MultiByteToWideChar(CP_UTF8, 0, message, -1, buffer.get(), charCount) == 0)
        return;

    if (FAILED(StringCchCatW(buffer.get(), charCount + 1, L"\n")))
        return;

    OutputDebugStringW(buffer.get());
#   if defined(_DEBUG)
    HANDLE handle = INVALID_HANDLE_VALUE;
    switch (level)
    {
        case LogLevel_Trace:
        case LogLevel_Debug:
        case LogLevel_Info:
            handle = GetStdHandle(STD_OUTPUT_HANDLE);
            break;

        case LogLevel_Warn:
        case LogLevel_Error:
        case LogLevel_Fatal:
            handle = GetStdHandle(STD_ERROR_HANDLE);
            break;

        default:
            return;
    }

    auto origAttribs = SetForegroundColor(handle, LogLevelColors[(uint32_t)level]);
    WriteText(handle, s_logLevelPrefixes[(uint32_t)level]);
    // reset to original colors
    ::SetConsoleTextAttribute(handle, origAttribs);
    WriteConsoleW(handle, buffer.get(), static_cast<DWORD>(wcslen(buffer.get())), nullptr, nullptr);
#   endif
#endif
}
