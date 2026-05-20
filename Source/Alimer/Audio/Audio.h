// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Math/Vector3.h"
struct ma_engine;

namespace Alimer
{
    class ALIMER_API AudioListener final : public Object
    {
        ALIMER_OBJECT(AudioListener, Object);

        friend class Audio;
    public:
        bool IsEnabled() const;
        void SetEnabled(bool value);

        Vector3 GetPosition() const;
        void SetPosition(const Vector3& value);

        Vector3 GetDirection() const;
        void SetDirection(const Vector3& value);

        Vector3 GetVelocity() const;
        void SetVelocity(const Vector3& value);

    private:
        AudioListener(uint32_t listenerIndex);

        uint32_t listenerIndex = 0;
    };

    struct AudioConfig
    {
        float masterVolume = 1.0f;
        /// Audio output channel count.
        uint32_t channelCount = 0;
        /// Audio output sample rate.
        uint32_t sampleRate = 0;
    };

    class ALIMER_API Audio final 
    {
    public:
        static bool Initialize(const AudioConfig* config = nullptr);
        static void Shutdown();
        static void Suspend();
        static void Resume();

        static float GetMasterVolume();
        static void SetMasterVolume(float volume);
        static void SetMasterVolumeInDecibels(float decibels);

        /// Returns the number of channels going into the mastering voice
        static uint32_t GetOutputChannels();

        /// Returns the sample rate going into the mastering voice
        static uint32_t GetOutputSampleRate();

        [[nodiscard]] static ma_engine* GetEngine();

        static Vector<AudioListener*> Listeners;
    };
}
