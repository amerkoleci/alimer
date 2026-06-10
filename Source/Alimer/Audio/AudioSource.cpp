// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/MemoryStream.h"
#include "Alimer/Audio/AudioSource.h"
#include "Alimer/Audio/Audio.h"
#include <miniaudio.h>

namespace Alimer
{
    struct AudioSourceImpl final
    {
        ma_sound* handle = nullptr;
    };
}

using namespace Alimer;

void AudioSource::Register()
{
    RegisterFactory<AudioSource>();
}

AudioSource::AudioSource(AudioClip* clip)
    : pImpl(new AudioSourceImpl())
{
    pImpl->handle = (ma_sound*)ma_malloc(sizeof(ma_sound), nullptr);
    SetClip(clip);
}

AudioSource::~AudioSource()
{
    if (pImpl->handle)
    {
        ma_sound_uninit(pImpl->handle);
        ma_free(pImpl->handle, nullptr);
    }

    SafeDelete(pImpl);
}

void AudioSource::SetClip(AudioClip* clip)
{
    if (_clip.Get() == clip)
        return;

    _clip = clip;

    if (_isValid)
    {
        ma_sound_uninit(pImpl->handle);
    }

    // TODO: Handle MA_SOUND_FLAG_NO_PITCH and MA_SOUND_FLAG_NO_SPATIALIZATION
    ma_uint32 flags = 0;
    if (_streamed)
        flags |= MA_SOUND_FLAG_STREAM;

    if (_isLooping)
        flags |= MA_SOUND_FLAG_LOOPING;

    if(!_pitchingEnabled)
        flags |= MA_SOUND_FLAG_NO_PITCH;

    if (!_spatializationEnabled)
        flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
    
    ma_result result = ma_sound_init_from_data_source(Audio::GetEngine(), clip->GetDecoder(), flags, nullptr, pImpl->handle);
    if (result != MA_SUCCESS)
    {
        // An error occurred.
        return;
    }

    _isValid = true;
    _volume = ma_sound_get_volume(pImpl->handle);
}

AudioSource::State AudioSource::GetState() const noexcept
{
    return _state;
}

void AudioSource::Play()
{
    if (_state == State::Playing)
        return;

    ma_sound_start(pImpl->handle);
    _state = ma_sound_is_playing(pImpl->handle) == MA_TRUE ? State::Playing : State::Stopped;
}

void AudioSource::Pause()
{
    if (_state == State::Paused)
        return;

    ma_sound_stop(pImpl->handle);
    _state = State::Paused;
}

void AudioSource::Stop()
{
    if (_state == State::Stopped)
        return;

    ma_sound_stop(pImpl->handle);
    ma_sound_seek_to_pcm_frame(pImpl->handle, 0);

    _state = State::Stopped;
}

void AudioSource::SetLooping(bool looping)
{
    _isLooping = looping;
    if (_isValid)
    {
        ma_sound_set_looping(pImpl->handle, looping);
    }
}

void AudioSource::SetVolume(float volume)
{
    _volume = volume;
    if (_isValid)
    {
        ma_sound_set_volume(pImpl->handle, volume);
    }
}

void AudioSource::SetSpatializationEnabled(bool enabled)
{
    _spatializationEnabled = enabled;
    if (_isValid)
    {
        ma_sound_set_spatialization_enabled(pImpl->handle, enabled);
    }
}
