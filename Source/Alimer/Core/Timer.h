// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"
#include <chrono>

namespace Alimer
{
    /// High-resolution timer for measuring elapsed time and frame rates.
    class ALIMER_API Timer final
    {
    public:
        /// Constructor.
        Timer() noexcept;

        /// Get the total elapsed time since the timer was created or last reset.
        [[nodiscard]] uint64_t GetElapsedTicks() const noexcept { return _totalTicks; }
        [[nodiscard]] double GetElapsedSeconds() const noexcept { return TicksToSeconds(_totalTicks); }

        /// Get the time elapsed since the last Tick call.
        [[nodiscard]] uint64_t GetDeltaTicks() const noexcept { return _deltaTicks; }
        [[nodiscard]] double GetDeltaSeconds() const noexcept { return TicksToSeconds(_deltaTicks); }

        /// Get the current frame rate.
        [[nodiscard]] uint32_t GetFrameCount() const noexcept { return _frameCount; }
        [[nodiscard]] uint32_t GetFramesPerSecond() const noexcept { return _framesPerSecond; }

        /// Update the timer state and calculate delta time.
        void Tick();

        /// Reset the elapsed time counter.
        void ResetElapsedTime();

        /// Set whether to use fixed time step mode.
        void SetFixedTimeStep(bool isFixedTimeStep) noexcept { _isFixedTimeStep = isFixedTimeStep; }
        [[nodiscard]] bool IsFixedTimeStep() const noexcept { return _isFixedTimeStep; }

        /// Set the target elapsed time for fixed time step mode.
        void SetTargetElapsedTicks(uint64_t targetElapsed) noexcept { _targetElapsedTicks = targetElapsed; }
        void SetTargetElapsedSeconds(double targetElapsed) noexcept { _targetElapsedTicks = SecondsToTicks(targetElapsed); }
        [[nodiscard]] uint64_t GetTargetElapsedTicks() const noexcept { return _targetElapsedTicks; }
        [[nodiscard]] double GetTargetElapsedSeconds() const noexcept { return TicksToSeconds(_targetElapsedTicks); }

    private:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        static constexpr uint64_t kMaxDelta = TicksPerSecond / 10; // Maximum delta of 1/10 second

        TimePoint _startTime;
        TimePoint _lastTime;

        uint64_t _totalTicks;
        uint64_t _deltaTicks;
        uint64_t _leftOverTicks;
        uint64_t _targetElapsedTicks;

        uint32_t _frameCount;
        uint32_t _framesPerSecond;
        uint32_t _framesThisSecond;
        uint64_t _secondCounter;

        bool _isFixedTimeStep;

        static uint64_t GetTicksFromTimePoint(const TimePoint& time) noexcept;
    };
}
