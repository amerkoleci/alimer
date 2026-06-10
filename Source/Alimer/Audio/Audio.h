// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Math/Vector3.h"
#include "Alimer/Audio/Types.h"
#include <array>
struct ma_engine;

namespace Alimer
{
    struct AudioDeviceId
    {
        alignas(8) std::array<char, 256> data;
    };

    struct AudioDevice final
    {
        AudioDeviceId deviceId;
        AudioDeviceType deviceType;
        std::array<char, 256> deviceName;
        bool isDefault;
    };

    struct AudioConfig
    {
        AudioDevice* playbackDevice = nullptr;
        /// Audio output channel count.
        uint32_t channelCount = 0;
        /// Audio output sample rate.
        uint32_t sampleRate = 0;
    };

    class ALIMER_API Audio final 
    {
    public:
        static Vector<AudioDevice> PlaybackDevices;
        static Vector<AudioDevice> CaptureDevices;

        static bool Initialize();
        static void Shutdown();
        static bool InitEngine(const AudioConfig* config = nullptr);
        static void ShutdownEngine();
        static void Suspend();
        static void Resume();

        static float GetVolume(VolumeUnit unit = VolumeUnit::Linear);
        static void SetVolume(float volume, VolumeUnit unit = VolumeUnit::Linear);

        /// Returns the number of channels going into the mastering voice
        [[nodiscard]] static uint32_t GetOutputChannelCount();

        /// Returns the sample rate going into the mastering voice
        [[nodiscard]] static uint32_t GetOutputSampleRate();

        /* Listener */
        [[nodiscard]] static uint32_t GetListenerCount();
        [[nodiscard]] static bool IsListenerEnabled(uint32_t listenerIndex);
        static void SetListenerEnabled(uint32_t listenerIndex, bool enabled);

        [[nodiscard]] static Vector3 GetListenerPosition(uint32_t listenerIndex);
        static void SetListenerPosition(uint32_t listenerIndex, const Vector3& value);

        [[nodiscard]] static Vector3 GetListenerDirection(uint32_t listenerIndex);
        static void SetListenerDirection(uint32_t listenerIndex, const Vector3& value);

        [[nodiscard]] static Vector3 GetListenerVelocity(uint32_t listenerIndex);
        static void SetListenerVelocity(uint32_t listenerIndex, const Vector3& value);

        [[nodiscard]] static Vector3 GetListenerWorldUp(uint32_t listenerIndex);
        static void SetListenerWorldUp(uint32_t listenerIndex, const Vector3& value);

        static void SetListenerCone(uint32_t listenerIndex, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
        static void GetListenerCone(uint32_t listenerIndex, float& innerAngleInRadians, float& outerAngleInRadians, float& outerGain);

        [[nodiscard]] static ma_engine* GetEngine();
    };
}
