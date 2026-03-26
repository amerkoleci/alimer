// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Stopwatch.h"
#if defined(_WIN32)
#   include "Alimer/Platform/Win32/WindowsPlatform.h"
#elif defined(__APPLE__)
#   include <mach/mach_time.h>
#elif defined(__EMSCRIPTEN__)
#   include <emscripten/emscripten.h>
#else
#   include <unistd.h>
#   include <sys/time.h>
#endif

using namespace Alimer;

Stopwatch::Stopwatch()
{
    Reset();
}

void Stopwatch::Start()
{
    // Calling start on a running Stopwatch is a no-op.
    if (!isRunning)
    {
        startTimeStamp = GetTimestamp();
        isRunning = true;
    }
}

void Stopwatch::Stop()
{
    // Calling stop on a stopped Stopwatch is a no-op.
    if (isRunning)
    {
        uint64_t endTimeStamp = GetTimestamp();
        uint64_t elapsedThisPeriod = endTimeStamp - startTimeStamp;
        elapsed += elapsedThisPeriod;
        isRunning = false;

        if (elapsed < 0)
        {
            // When measuring small time periods the Stopwatch.Elapsed*
            // properties can return negative values.  This is due to
            // bugs in the basic input/output system (BIOS) or the hardware
            // abstraction layer (HAL) on machines with variable-speed CPUs
            // (e.g. Intel SpeedStep).

            elapsed = 0;
        }
    }
}

void Stopwatch::Reset()
{
    elapsed = 0;
    isRunning = false;
    startTimeStamp = 0;
}

void Stopwatch::Restart()
{
    elapsed = 0;
    startTimeStamp = GetTimestamp();
    isRunning = true;
}

uint64_t Stopwatch::GetElapsedTicks() const noexcept
{
    uint64_t timeElapsed = elapsed;

    if (isRunning)
    {
        // If the Stopwatch is running, add elapsed time since
        // the Stopwatch is started last time.
        uint64_t currentTimeStamp = GetTimestamp();
        uint64_t elapsedUntilNow = currentTimeStamp - startTimeStamp;
        timeElapsed += elapsedUntilNow;
    }

    return timeElapsed;
}

double Stopwatch::GetElapsedSeconds() const noexcept
{
    return TicksToSeconds(GetElapsedTicks());
}

uint64_t Stopwatch::GetFrequency()
{
    uint64_t frequency = 0;
    if (!frequency)
    {
#if defined(_WIN32)
        if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency))
        {
            throw std::exception();
        }
#elif defined(__APPLE__)
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        frequency = (info.denom * 1e9) / info.numer;
#else
        frequency = 1000000000;
#endif
    }

    return frequency;
}

uint64_t Stopwatch::GetTimestamp()
{
#if defined(_WIN32)
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*)&value);
    return value;
#elif defined(__APPLE__)
    return mach_absolute_time();
#elif defined(__EMSCRIPTEN__)
    return (uint64_t)(emscripten_get_now() * 1000.0);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
#endif
}
