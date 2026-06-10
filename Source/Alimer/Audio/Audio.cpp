// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/Audio/Audio.h"
#include "Alimer/Audio/AudioClip.h"
#include "Alimer/Audio/AudioSource.h"
#include "Alimer/Audio/Components/AudioSourceComponent.h"
#include "Alimer/Audio/Components/AudioListenerComponent.h"

ALIMER_DISABLE_WARNINGS()
#define STB_VORBIS_HEADER_ONLY
//#define STB_VORBIS_NO_STDIO
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_WASAPI
#define MA_ENABLE_ALSA
#define MA_ENABLE_COREAUDIO
#define MA_ENABLE_OPENSL
#define MA_ENABLE_WEBAUDIO
//#define MA_NO_DECODING
#define MA_NO_ENCODING
//#define MA_NO_RESOURCE_MANAGER
#define MA_NO_GENERATION
//#define MA_NO_NODE_GRAPH
//#define MA_NO_ENGINE
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
ALIMER_ENABLE_WARNINGS()

using namespace Alimer;

namespace
{
    static inline Vector3 ToVector3(const ma_vec3f& v)
    {
        return { v.x, v.y, v.z };
    }
}

static struct
{
    std::atomic_bool initialized;
    ma_engine engine{};
    uint32_t masterChannels;
    uint32_t masterRate;
    uint32_t listenerCount;
} g_audio;

Vector<AudioListener*> Audio::Listeners;

bool Audio::Initialize(const AudioConfig* config)
{
    if (g_audio.initialized.load())
        return true;

    ma_engine_config ma_config = ma_engine_config_init();
    ma_config.channels = (config != nullptr && config->channelCount > 0) ? config->channelCount : 0;
    ma_config.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 0;

    ma_result result = ma_engine_init(&ma_config, &g_audio.engine);
    if (result != MA_SUCCESS)
    {
        LOGE("Failed to initialize audio engine!");
        return false;
    }

    AudioClip::Register();
    AudioSource::Register();
    AudioSourceComponent::Register();
    AudioListenerComponent::Register();

    if (config != nullptr)
    {
        SetVolume(config->masterVolume);
    }

    g_audio.masterChannels = ma_engine_get_channels(&g_audio.engine);
    g_audio.masterRate = ma_engine_get_sample_rate(&g_audio.engine);
    g_audio.listenerCount =  ma_engine_get_listener_count(&g_audio.engine);

    // Init listeners
    Listeners.resize(g_audio.listenerCount);
    for (uint32_t i = 0; i < g_audio.listenerCount; ++i)
    {
        Listeners[i] = new AudioListener(i);
    }

    g_audio.initialized.store(true);

    LOGI("Audio Initialized");
    return true;
}

void Audio::Shutdown()
{
    if (!g_audio.initialized.load())
        return;

    for (uint32_t i = 0; i < g_audio.listenerCount; ++i)
    {
        delete Listeners[i];
        Listeners[i] = nullptr;
    }

    ma_engine_uninit(&g_audio.engine);
    g_audio.initialized.store(false);
    memset(&g_audio, 0, sizeof(g_audio));
}

void Audio::Suspend()
{
    if (ma_engine_stop(&g_audio.engine) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to suspend");
    }
}

void Audio::Resume()
{
    if (ma_engine_start(&g_audio.engine) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to resume");
    }
}

float Audio::GetVolume(VolumeUnit unit)
{
    const float volume = ma_engine_get_volume(&g_audio.engine);
    return (unit == VolumeUnit::Linear) ? volume : ma_volume_linear_to_db(volume);
}

void Audio::SetVolume(float volume, VolumeUnit unit)
{
    if (unit == VolumeUnit::Decibels)
        volume = ma_volume_db_to_linear(volume);

    if (ma_engine_set_volume(&g_audio.engine, volume) != MA_SUCCESS)
    {
        LOGE("Audio: Failed to set master volume");
    }
}

uint32_t Audio::GetOutputChannels() 
{
    return g_audio.masterChannels;
}

uint32_t Audio::GetOutputSampleRate()
{
    return g_audio.masterRate;
}

ma_engine* Audio::GetEngine()
{
    return &g_audio.engine;
}

/* AudioListener */
AudioListener::AudioListener(uint32_t listenerIndex_)
    : listenerIndex(listenerIndex_)
{
}

bool AudioListener::IsEnabled() const
{
    return ma_engine_listener_is_enabled(&g_audio.engine, listenerIndex);
}

void AudioListener::SetEnabled(bool value)
{
    ma_engine_listener_set_enabled(&g_audio.engine, listenerIndex, value);
}

//MA_API void ma_engine_listener_set_cone(ma_engine* pEngine, ma_uint32 listenerIndex, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
//MA_API void ma_engine_listener_get_cone(const ma_engine* pEngine, ma_uint32 listenerIndex, float* pInnerAngleInRadians, float* pOuterAngleInRadians, float* pOuterGain);
//MA_API void ma_engine_listener_set_world_up(ma_engine* pEngine, ma_uint32 listenerIndex, float x, float y, float z);
//MA_API ma_vec3f ma_engine_listener_get_world_up(const ma_engine* pEngine, ma_uint32 listenerIndex);

Vector3 AudioListener::GetPosition() const
{
    return ToVector3(ma_engine_listener_get_position(&g_audio.engine, listenerIndex));
}

void AudioListener::SetPosition(const Vector3& value)
{
    ma_engine_listener_set_position(&g_audio.engine, listenerIndex, value.x, value.y, value.z);
}

Vector3 AudioListener::GetDirection() const
{
    return ToVector3(ma_engine_listener_get_direction(&g_audio.engine, listenerIndex));
}

void AudioListener::SetDirection(const Vector3& value)
{
    ma_engine_listener_set_direction(&g_audio.engine, listenerIndex, value.x, value.y, value.z);
}

Vector3 AudioListener::GetVelocity() const
{
    return ToVector3(ma_engine_listener_get_velocity(&g_audio.engine, listenerIndex));
}

void AudioListener::SetVelocity(const Vector3& value)
{
    ma_engine_listener_set_velocity(&g_audio.engine, listenerIndex, value.x, value.y, value.z);
}
