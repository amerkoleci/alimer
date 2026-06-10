// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Audio/AudioClip.h"

namespace Alimer
{
    struct AudioSourceImpl;

    class ALIMER_API AudioSource final : public Object
    {
        ALIMER_OBJECT(AudioSource, Object);

    public:
        /// The audio source's audio state.
        enum class State : uint32_t
        {
            Stopped = 0,
            Playing,
            Paused,
        };
        /// Register object factory and properties.
        static void Register();

        /// Constructor
        AudioSource(AudioClip* clip = nullptr);

        /// Destructor
        ~AudioSource() override;

        /// Plays the audio source.
        void Play();

        /// Pauses the audio source.
        void Pause();

        /// Stops the audio source.
        void Stop();

        AudioClip* GetClip() const { return _clip.Get(); }
        void SetClip(AudioClip* clip);

        /// Gets the current state of the audio source.
        [[nodiscard]] State GetState() const noexcept;

        [[nodiscard]] bool IsValid() const { return _isValid; }


        [[nodiscard]] bool IsLooping() const noexcept { return _isLooping; }
        void SetLooping(bool looping);

        [[nodiscard]] float GetVolume() const noexcept  { return _volume; }
        void SetVolume(float volume);

        [[nodiscard]] bool IsSpatializationEnabled() const noexcept { return _spatializationEnabled; }
        void SetSpatializationEnabled(bool enabled);

    private:
        AudioSourceImpl* pImpl;
        AudioClipRef _clip;
        State _state = State::Stopped;
        bool _isValid = false;
        bool _isLooping = false;
        float _volume = 1.0f;
        bool _pitchingEnabled = false;
        bool _spatializationEnabled = true;
        bool _streamed = false;
    };
}
