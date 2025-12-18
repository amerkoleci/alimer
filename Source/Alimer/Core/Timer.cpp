// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Timer.h"

namespace Alimer
{
    Timer::Timer() noexcept
        : _startTime(Clock::now())
        , _lastTime(_startTime)
        , _totalTicks(0)
        , _deltaTicks(0)
        , _leftOverTicks(0)
        , _targetElapsedTicks(TicksPerSecond / 60) // Default to 60 FPS
        , _frameCount(0)
        , _framesPerSecond(0)
        , _framesThisSecond(0)
        , _secondCounter(0)
        , _isFixedTimeStep(false)
    {
    }

    uint64_t Timer::GetTicksFromTimePoint(const TimePoint& time) noexcept
    {
        const auto duration = time.time_since_epoch();
        const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        return static_cast<uint64_t>(microseconds * 10); // Convert microseconds to ticks (1 tick = 100 nanoseconds)
    }

    void Timer::Tick()
    {
        // Query the current time
        const TimePoint currentTime = Clock::now();

        uint64_t timeDelta = GetTicksFromTimePoint(currentTime) - GetTicksFromTimePoint(_lastTime);

        _lastTime = currentTime;
        _secondCounter += timeDelta;

        // Clamp excessively large time deltas (e.g., after a pause in the debugger)
        if (timeDelta > kMaxDelta)
        {
            timeDelta = kMaxDelta;
        }

        if (_isFixedTimeStep)
        {
            // Fixed timestep update logic
            if (static_cast<uint64_t>(Abs(static_cast<int64_t>(timeDelta - _targetElapsedTicks))) < TicksPerSecond / 4000)
            {
                // Clamp to exactly target elapsed time to prevent jitter
                timeDelta = _targetElapsedTicks;
            }

            _leftOverTicks += timeDelta;

            while (_leftOverTicks >= _targetElapsedTicks)
            {
                _deltaTicks = _targetElapsedTicks;
                _totalTicks += _targetElapsedTicks;
                _leftOverTicks -= _targetElapsedTicks;
                _frameCount++;

                _framesThisSecond++;
            }
        }
        else
        {
            // Variable timestep update logic
            _deltaTicks = timeDelta;
            _totalTicks += timeDelta;
            _leftOverTicks = 0;
            _frameCount++;

            _framesThisSecond++;
        }

        // Track the current framerate
        if (_secondCounter >= TicksPerSecond)
        {
            _framesPerSecond = _framesThisSecond;
            _framesThisSecond = 0;
            _secondCounter %= TicksPerSecond;
        }
    }

    void Timer::ResetElapsedTime()
    {
        _startTime = Clock::now();
        _lastTime = _startTime;
        _totalTicks = 0;
        _deltaTicks = 0;
        _leftOverTicks = 0;
        _frameCount = 0;
        _framesPerSecond = 0;
        _framesThisSecond = 0;
        _secondCounter = 0;
    }
}
