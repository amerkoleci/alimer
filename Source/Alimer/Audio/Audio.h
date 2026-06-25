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
        AudioDeviceId* playbackDeviceId = nullptr;
        /// Audio output channel count.
        uint32_t channelCount = 0;
        /// Audio output sample rate.
        uint32_t sampleRate = 0;
    };

    struct AudioEngineImpl;

    class ALIMER_API AudioEngine final : public RefCounted
    {
    public:
        AudioEngine(const AudioConfig* config = nullptr);
        ~AudioEngine() override;

        /// Returns the number of channels going into the mastering voice
        [[nodiscard]] uint32_t GetOutputChannelCount() const;

        /// Returns the sample rate going into the mastering voice
        [[nodiscard]] uint32_t GetOutputSampleRate() const;

        /// Returns the current state of the audio engine.
        [[nodiscard]] AudioEngineState GetState() const;

        /// Get the number of audio listeners supported by the engine.
        [[nodiscard]] uint32_t GetListenerCount() const { return _listenerCount; }

        void Suspend();
        void Resume();

        [[nodiscard]] float GetMasterVolume(VolumeUnit unit = VolumeUnit::Linear) const;
        void SetMasterVolume(float volume, VolumeUnit unit = VolumeUnit::Linear);

        [[nodiscard]] float GetVolume(VolumeUnit unit = VolumeUnit::Linear) const;
        void SetVolume(float volume, VolumeUnit unit = VolumeUnit::Linear);

        [[nodiscard]] uint64_t GetTimeInPCMFrames() const;
        [[nodiscard]] uint64_t GetTimeInMilliseconds() const;
        void SetTimeInPCMFrames(uint64_t value);
        void SetTimeInMilliseconds(uint64_t value);

        /* Listener */
        [[nodiscard]] bool IsListenerEnabled(uint32_t listenerIndex) const;
        void SetListenerEnabled(uint32_t listenerIndex, bool enabled);

        [[nodiscard]] Vector3 GetListenerPosition(uint32_t listenerIndex) const;
        void SetListenerPosition(uint32_t listenerIndex, const Vector3& value);

        [[nodiscard]] Vector3 GetListenerDirection(uint32_t listenerIndex) const;
        void SetListenerDirection(uint32_t listenerIndex, const Vector3& value);

        [[nodiscard]] Vector3 GetListenerVelocity(uint32_t listenerIndex) const;
        void SetListenerVelocity(uint32_t listenerIndex, const Vector3& value);

        [[nodiscard]] Vector3 GetListenerWorldUp(uint32_t listenerIndex) const;
        void SetListenerWorldUp(uint32_t listenerIndex, const Vector3& value);

        void GetListenerCone(uint32_t listenerIndex, float& innerAngleInRadians, float& outerAngleInRadians, float& outerGain) const;
        void SetListenerCone(uint32_t listenerIndex, float innerAngleInRadians, float outerAngleInRadians, float outerGain);

        [[nodiscard]] ma_engine* GetEngine() const;

    protected:
        AudioEngineImpl* _impl;
        uint32_t _listenerCount = 0;
    };

    class ALIMER_API Audio final
    {
    public:
        static Vector<AudioDevice> PlaybackDevices;
        static Vector<AudioDevice> CaptureDevices;

        static bool Initialize();
        static void Shutdown();
    };
}
