// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer.h"
#include <stdarg.h>
#include <stdio.h>

#if defined(__ANDROID__)
#   include <android/log.h>
#elif defined(__EMSCRIPTEN__)
#   include <emscripten/emscripten.h>
#elif ALIMER_PLATFORM_IOS || ALIMER_PLATFORM_TVOS
#   include <sys/syslog.h>
#elif ALIMER_PLATFORM_MACOS || ALIMER_PLATFORM_LINUX
#   include <errno.h>
#   include <unistd.h>
#elif ALIMER_PLATFORM_WINDOWS
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <Windows.h>
#   include <strsafe.h>
#endif

[[maybe_unused]] static const char* s_logLevelPrefixes[LogLevel_Count + 1] = {
    "OFF",
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
    NULL
};

#if defined(__ANDROID__)
int GetPriority(LogLevel level)
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
#elif defined(__EMSCRIPTEN__)
int GetLogFlags(LogLevel level)
{
    switch (level)
    {
        case LogLevel_Debug:    return EM_LOG_CONSOLE | EM_LOG_DEBUG;
        case LogLevel_Info:     return EM_LOG_CONSOLE | EM_LOG_INFO;
        case LogLevel_Warn:     return EM_LOG_CONSOLE | EM_LOG_WARN;
        case LogLevel_Error:    return EM_LOG_CONSOLE | EM_LOG_ERROR;
        case LogLevel_Fatal:    return EM_LOG_CONSOLE | EM_LOG_ERROR | EM_LOG_C_STACK | EM_LOG_JS_STACK;
        default:
            return EM_LOG_CONSOLE;
    }
}
#elif TARGET_OS_IOS || TARGET_OS_TV
int GetPriority(LogLevel level)
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
int GetPriority(LogLevel level)
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
#elif defined(_WIN32) && defined(_DEBUG)
static WORD LogLevelColors[LogLevel_Count] = {
    0, // Off
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,        // white
    FOREGROUND_GREEN | FOREGROUND_BLUE,                         // cyan
    FOREGROUND_GREEN,                                           // green
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,   // intense yellow
    FOREGROUND_RED | FOREGROUND_INTENSITY,                      // intense red
    BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY // intense white on red background
};

inline WORD SetForegroundColor(HANDLE handle, WORD attribs)
{
    CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
    if (!GetConsoleScreenBufferInfo(handle, &orig_buffer_info))
    {
        // just return white if failed getting console info
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    // change only the foreground bits (lowest 4 bits)
    WORD new_attribs = attribs | (orig_buffer_info.wAttributes & 0xfff0);
    BOOL ignored = SetConsoleTextAttribute(handle, new_attribs);
    ALIMER_UNUSED(ignored);
    return orig_buffer_info.wAttributes; // return orig attribs
}

inline void WriteText(HANDLE handle, const char* text)
{
    BOOL ignored = WriteConsoleA(handle, text, (DWORD)strlen(text), NULL, NULL);
    ALIMER_UNUSED(ignored);
}
#endif

static void DefaultLogCallback(LogCategory category, LogLevel level, const char* message, void* userData)
{
    ALIMER_UNUSED(category);
    ALIMER_UNUSED(userData);

#if defined(__ANDROID__)
    __android_log_print(GetPriority(level), "Alimer", "%s", message);
#elif defined(__EMSCRIPTEN__)
    emscripten_log(GetLogFlags(level), "%s", message);
#elif TARGET_OS_IOS || TARGET_OS_TV
    syslog(GetPriority(level), "%s", message);
#elif TARGET_OS_MAC || defined(__linux__)
    const int fd = GetPriority(level);
    size_t messageLength = strlen(message);
    char* str = (char*)ALIMER_ALLOCN(char, messageLength + 1); // +1 for the newline
    str[messageLength - 1] = '\n';

    size_t offset = 0;
    while (offset < messageLength)
    {
        ssize_t written = write(fd, str + offset, messageLength - offset);
        while (written == -1 && errno == EINTR)
            written = write(fd, str + offset, messageLength - offset);

        if (written == -1)
            return;

        offset += (size_t)written;
    }
#elif defined(_WIN32)
    const int charCount = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    if (charCount == 0)
        return;

    wchar_t* buffer = ALIMER_ALLOCN(wchar_t, charCount + 1); // +1 for the newline
    if (MultiByteToWideChar(CP_UTF8, 0, message, -1, buffer, charCount) == 0)
    {
        alimerFree(buffer);
        return;
    }

    if (FAILED(StringCchCatW(buffer, charCount + 1, L"\n")))
    {
        alimerFree(buffer);
        return;
    }

    OutputDebugStringW(buffer);
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

    WriteText(handle, "[");
    const WORD origAttribs = SetForegroundColor(handle, LogLevelColors[(uint32_t)level]);
    WriteText(handle, s_logLevelPrefixes[(uint32_t)level]);
    // reset to original colors
    SetConsoleTextAttribute(handle, origAttribs);
    WriteText(handle, "] ");
    WriteConsoleW(handle, buffer, (DWORD)(charCount + 1), NULL, NULL);
    alimerFree(buffer);
#   endif
#endif
}

#if defined(_DEBUG)
LogLevel s_logLevel = LogLevel_Debug;
#else
LogLevel s_logLevel = LogLevel_Info;
#endif
AlimerLogCallback s_logCallback = DefaultLogCallback;
void* s_logUserData = NULL;

LogLevel alimerGetLogLevel(void)
{
    return s_logLevel;
}

void alimerSetLogLevel(LogLevel level)
{
    s_logLevel = level;
}

void alimerSetLogCallback(AlimerLogCallback callback, void* userData)
{
    s_logCallback = callback;
    s_logUserData = userData;
}

static bool alimerShouldLog(LogLevel level)
{
    if (!s_logCallback || s_logLevel == LogLevel_Off)
        return false;

    return level >= s_logLevel;
}

void alimerLog(LogCategory category, LogLevel level, const char* message)
{
    if (!alimerShouldLog(level))
        return;

    s_logCallback(category, level, message, s_logUserData);
}

void alimerLogFormat(LogCategory category, LogLevel level, const char* format, ...)
{
    if (!alimerShouldLog(level))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, level, message, s_logUserData);
}

void alimerLogInfo(LogCategory category, const char* format, ...)
{
    if (!alimerShouldLog(LogLevel_Info))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, LogLevel_Info, message, s_logUserData);
}

void alimerLogDebug(LogCategory category, const char* format, ...)
{
    if (!alimerShouldLog(LogLevel_Debug))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, LogLevel_Debug, message, s_logUserData);
}

void alimerLogTrace(LogCategory category, const char* format, ...)
{
    if (!alimerShouldLog(LogLevel_Trace))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, LogLevel_Trace, message, s_logUserData);
}

void alimerLogWarn(LogCategory category, const char* format, ...)
{
    if (!alimerShouldLog(LogLevel_Warn))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, LogLevel_Warn, message, s_logUserData);
}

void alimerLogError(LogCategory category, const char* format, ...)
{
    if (!alimerShouldLog(LogLevel_Error))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, LogLevel_Error, message, s_logUserData);
    ALIMER_DEBUG_BREAK();
}

void alimerLogFatal(LogCategory category, const char* format, ...)
{
    if (!alimerShouldLog(LogLevel_Fatal))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(category, LogLevel_Fatal, message, s_logUserData);
    ALIMER_DEBUG_BREAK();
}
