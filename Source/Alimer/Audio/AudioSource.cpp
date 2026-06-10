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
    _clip = clip;

    // TODO: Handle MA_SOUND_FLAG_NO_PITCH and MA_SOUND_FLAG_NO_SPATIALIZATION
    ma_uint32 flags = 0;
    if (_streamed)
        flags |= MA_SOUND_FLAG_STREAM;
    
    ma_result result = ma_sound_init_from_data_source(Audio::GetEngine(), clip->GetDecoder(), flags, nullptr, pImpl->handle);
    if (result != MA_SUCCESS)
    {
        // An error occurred.
        return;
    }

    //ma_sound_set_looping(&pImpl->sound, true);
}

AudioSource::State AudioSource::GetState() const noexcept
{
    return _state;
}

void AudioSource::Play()
{
    const ma_result result = ma_sound_start(pImpl->handle);
    if (result != MA_SUCCESS)
    {
        // An error occurred.
        return;
    }
}
