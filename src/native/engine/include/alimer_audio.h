// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_AUDIO_H_
#define ALIMER_AUDIO_H_ 1

#include "alimer_types.h"
#include <stdbool.h>

/* Forward */
typedef struct AudioDevice AudioDevice;
typedef struct AudioEngine AudioEngine;
typedef struct AudioClip AudioClip;

/* Enums */
typedef enum AudioDeviceType {
    AudioDeviceType_Playback,
    AudioDeviceType_Capture,

    _AudioDeviceType_Count,
    _AudioDeviceType_Force32 = 0x7FFFFFFF
} AudioDeviceType;

typedef enum VolumeUnit {
    VolumeUnit_Linear,
    VolumeUnit_Decibels
} VolumeUnit;

typedef enum AudioFormat {
    AudioFormat_Unknown,
    AudioFormat_Unsigned8 = 1,
    AudioFormat_Signed16 = 2,
    AudioFormat_Signed24 = 3,
    AudioFormat_Signed32 = 4,
    AudioFormat_Float32 = 5,

    _AudioFormat_Count,
    _AudioFormat_Force32 = 0x7FFFFFFF
} AudioFormat;


typedef enum AudioPanMode {
    AudioPanMode_Balance,
    AudioPanMode_Pan,

    _AudioPanMode_Count,
    _AudioPanMode_Force32 = 0x7FFFFFFF
} AudioPanMode;

typedef enum AudioAttenuationModel {
    AudioAttenuationModel_None,
    AudioAttenuationModel_Inverse,
    AudioAttenuationModel_Linear,
    AudioAttenuationModel_Exponential,

    _AudioAttenuationModel_Count,
    _AudioAttenuationModel_Force32 = 0x7FFFFFFF
} AudioAttenuationModel;

typedef enum AudioPositioning {
    AudioPositioning_Absolute,
    AudioPositioning_Relative,

    _AudioPositioning_Count,
    _AudioPositioning_Force32 = 0x7FFFFFFF
} AudioPositioning;

/* Structs */
typedef struct AudioEngineConfig {
    AudioDevice* playbackDevice DEFAULT_INITIALIZER(nullptr);
    /// Audio output channel count.
    uint32_t channelCount DEFAULT_INITIALIZER(2);
    /// Audio output sample rate.
    uint32_t sampleRate DEFAULT_INITIALIZER(48000);
} AudioEngineConfig;

typedef struct AudioContextConfig {
    bool noAudio DEFAULT_INITIALIZER(false);
} AudioContextConfig;

/* Callbacks */
typedef void AudioDeviceCallback(AudioDevice* device, void* userdata);

/* AudioDevice */
ALIMER_API AudioDeviceType alimerAudioDeviceGetType(AudioDevice* device);
ALIMER_API const char* alimerAudioDeviceGetName(AudioDevice* device);
ALIMER_API bool alimerAudioDeviceIsDefault(AudioDevice* device);

/* AudioEngine */
ALIMER_API AudioEngine* alimerAudioEngineCreate(const AudioEngineConfig* config);
ALIMER_API void alimerAudioEngineDestroy(AudioEngine* engine);

/* AudioClip */
ALIMER_API AudioClip* alimerAudioClipCreate(const char* filepath);
ALIMER_API AudioClip* alimerAudioClipCreateFromMemory(const void* pData, size_t dataSize);
ALIMER_API void alimerAudioClipAddRef(AudioClip* clip);
ALIMER_API void alimerAudioClipRelease(AudioClip* clip);
ALIMER_API AudioFormat alimerAudioClipGetFormat(AudioClip* clip);
ALIMER_API uint32_t alimerAudioClipGetChannelCount(AudioClip* clip);
ALIMER_API uint32_t alimerAudioClipGetSampleRate(AudioClip* clip);
ALIMER_API uint64_t alimerAudioClipGetFrameCount(AudioClip* clip);
ALIMER_API uint32_t alimerAudioClipGetStride(AudioClip* clip);

#endif /* ALIMER_AUDIO_H_ */
