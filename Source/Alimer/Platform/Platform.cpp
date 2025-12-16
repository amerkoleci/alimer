// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Platform/Platform.h"
#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#include <array>
#elif defined(__EMSCRIPTEN__)
#include "Alimer/Core/Log.h"
#elif defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif
#include <thread>

using namespace Alimer;

PlatformID Platform::GetID()
{
#if ALIMER_PLATFORM_WINDOWS
    return PlatformID::Windows;
#elif ALIMER_PLATFORM_XBOX_ONE
    return PlatformID::XboxOne;
#elif ALIMER_PLATFORM_XBOX_SCARLETT
    return PlatformID::XboxScarlett;
#elif ALIMER_PLATFORM_LINUX
    return PlatformID::Linux;
#elif ALIMER_PLATFORM_ANDROID
    return PlatformID::Android;
#elif ALIMER_PLATFORM_MACOS
    return PlatformID::MacOS;
#elif ALIMER_PLATFORM_IOS
    return PlatformID::iOS;
#elif ALIMER_PLATFORM_BROWSER
    return PlatformID::Browser;
#else
#   error "Unknown platform"
#endif
}

PlatformFamily Platform::GetFamily()
{
#if ALIMER_PLATFORM_FAMILY_MOBILE
    return PlatformFamily::Mobile;
#elif ALIMER_PLATFORM_FAMILY_DESKTOP
    return PlatformFamily::Desktop;
#elif ALIMER_PLATFORM_FAMILY_CONSOLE
    return PlatformFamily::Console;
#else
    return PlatformFamily::Unknown;
#endif
}

uint32_t Platform::GetNumPhysicalCPUs()
{
    uint32_t cpuCount = 0;
#if defined(__EMSCRIPTEN__)
    cpuCount = std::thread::hardware_concurrency();
    return cpuCount;
#elif defined(_WIN32)
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        const size_t entryCount = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> entries(entryCount);
        if (GetLogicalProcessorInformation(entries.data(), &bufferSize))
        {
            for (const auto& entry : entries)
            {
                if (entry.Relationship == RelationProcessorCore)
                    cpuCount++;
            }
        }
    }
#endif

    // This is as good as it gets on remaining platforms.
    if (cpuCount == 0)
        cpuCount = std::thread::hardware_concurrency();

    return Max(1u, cpuCount);
}
