// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Audio/AudioClip.h"

namespace Alimer
{
    class AudioSource;
    using AudioSourceRef = SharedPtr<AudioSource>;

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

        AudioSource(AudioClip* clip = nullptr);

        /// Destructor
        ~AudioSource() override;

        /// Plays the audio source.
        void Play();

        AudioClip* GetClip() const { return _clip.Get(); }
        void SetClip(AudioClip* clip);

        /// Gets the current state of the audio source.
        State GetState() const noexcept;

    private:
        AudioSourceImpl* pImpl;
        AudioClipRef _clip;
        State _state = State::Stopped;
    };
}
