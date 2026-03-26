// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    class ALIMER_API Stopwatch final
    {
    public:
        Stopwatch();

        void Start();
        void Stop();
        void Reset();
        void Restart();

        bool IsRunning() const noexcept { return isRunning; }
        uint64_t GetElapsedTicks() const noexcept;
        double GetElapsedSeconds() const noexcept;

        static uint64_t GetFrequency();
        static uint64_t GetTimestamp();

    private:
        bool isRunning{ false };
        uint64_t elapsed{};
        uint64_t startTimeStamp{};
    };
}
