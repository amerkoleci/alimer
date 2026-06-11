// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <stdint.h>

namespace Alimer
{
    enum class AudioDeviceType
    {
        Playback,
        Capture,
    };

    enum class AudioEngineState
    {
        Uninitialized,
        Stopped,
        Started,
        Starting,
        Stopping,
    };

    enum class VolumeUnit
    {
        Linear,
        Decibels
    };

    enum class AudioFormat
    {
        /// 8-bit unsigned integer, range [0, 255]
        Uint8,
        /// 16-bit signed integer, range [-32768, 32767].
        Sint16,
        /// 24-bit signed integer (tightly packed), range [-8388608, 8388607].
        Sint24,
        /// 32-bit signed integer, range [-2147483648, 2147483647].
        Sint32,
        /// 32-bit floating point, range [-1, 1].
        Float32,

        Count
    };

    enum class AudioPanMode
    {
        Balance,
        Pan,
    };

    class AudioClip;
    class AudioSource;
    class AudioEngine;
    using AudioClipRef = SharedPtr<AudioClip>;
    using AudioSourceRef = SharedPtr<AudioSource>;
    using AudioEngineRef = SharedPtr<AudioEngine>;

    constexpr uint32_t GetSampleSize(AudioFormat format)
    {
        switch (format)
        {
            case AudioFormat::Uint8: return 1;
            case AudioFormat::Sint16: return 2;
            case AudioFormat::Sint24: return 3;
            case AudioFormat::Sint32: return 4;
            case AudioFormat::Float32: return 4;
            default: return 0;
        }
    }
}
