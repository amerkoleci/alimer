// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/MemoryStream.h"
#include "Alimer/Audio/AudioSource.h"
#include "Alimer/Audio/Audio.h"
#include "Alimer/Application.h"
#include <miniaudio.h>

namespace Alimer
{
    struct AudioSourceImpl final
    {
        ma_sound* handle = nullptr;
    };

    constexpr AudioPanMode FromMiniaudio(ma_pan_mode value)
    {
        switch (value)
        {
            case ma_pan_mode_balance: return AudioPanMode::Balance;
            case ma_pan_mode_pan:     return AudioPanMode::Pan;

            default:
                ALIMER_UNREACHABLE();
                return AudioPanMode::Balance;
        }
    }

    constexpr Vector3 FromMiniaudio(const ma_vec3f& vec)
    {
        return Vector3(vec.x, vec.y, vec.z);
    }

    constexpr ma_pan_mode ToMiniaudio(AudioPanMode value)
    {
        switch (value)
        {
            case AudioPanMode::Balance: return ma_pan_mode_balance;
            case AudioPanMode::Pan:     return ma_pan_mode_pan;

            default:
                ALIMER_UNREACHABLE();
                return ma_pan_mode_balance;
        }
    }
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

    if (!_pitchingEnabled)
        flags |= MA_SOUND_FLAG_NO_PITCH;

    if (!_spatializationEnabled)
        flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;

    ma_result result = ma_sound_init_from_data_source(GetPrimaryAudioEngine()->GetEngine(), clip->GetDecoder(), flags, nullptr, pImpl->handle);
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

bool AudioSource::IsAtEnd() const
{
    return ma_sound_at_end(pImpl->handle) == MA_TRUE;
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

float AudioSource::GetPan() const
{
    return _isValid ? ma_sound_get_pan(pImpl->handle) : 0.0f;
}

void AudioSource::SetPan(float value)
{
    if (_isValid)
    {
        ma_sound_set_pan(pImpl->handle, value);
    }
}

AudioPanMode AudioSource::GetPanMode() const
{
    return _isValid ? FromMiniaudio(ma_sound_get_pan_mode(pImpl->handle)) : AudioPanMode::Balance;
}

void AudioSource::SetPanMode(AudioPanMode value)
{
    if (_isValid)
    {
        ma_sound_set_pan_mode(pImpl->handle, ToMiniaudio(value));
    }
}

float AudioSource::GetPitch() const
{
    return _isValid ? ma_sound_get_pitch(pImpl->handle) : 1.0f;
}

void AudioSource::SetPitch(float value)
{
    if (_isValid)
    {
        ma_sound_set_pitch(pImpl->handle, value);
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

Vector3 AudioSource::GetPosition() const
{
    if (_isValid)
    {
        return FromMiniaudio(ma_sound_get_position(pImpl->handle));
    }
    return Vector3::Zero;
}

void AudioSource::SetPosition(const Vector3& position)
{
    //_position = position;
    if (_isValid)
    {
        ma_sound_set_position(pImpl->handle, position.x, position.y, position.z);
    }
}

Vector3 AudioSource::GetDirection() const
{
    if (_isValid)
    {
        return FromMiniaudio(ma_sound_get_direction(pImpl->handle));
    }

    return Vector3::Forward;
}

void AudioSource::SetDirection(const Vector3& direction)
{
    //_direction = direction;
    if (_isValid)
    {
        ma_sound_set_direction(pImpl->handle, direction.x, direction.y, direction.z);
    }
}

Vector3 AudioSource::GetVelocity() const
{
    if (_isValid)
    {
        return FromMiniaudio(ma_sound_get_velocity(pImpl->handle));
    }
    return Vector3::Zero;
}

void AudioSource::SetVelocity(const Vector3& velocity)
{
    //_velocity = velocity;
    if (_isValid)
    {
        ma_sound_set_velocity(pImpl->handle, velocity.x, velocity.y, velocity.z);
    }
}
