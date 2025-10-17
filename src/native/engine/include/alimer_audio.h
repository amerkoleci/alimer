// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_AUDIO_H_
#define ALIMER_AUDIO_H_ 1

#include "alimer.h"

/* Forward */
typedef struct AudioContext AudioContext;
typedef struct AudioEngine AudioEngine;

/* Enums */
typedef enum AudioDeviceType {
    AudioDeviceType_Playback,
    AudioDeviceType_Capture,

    _AudioDeviceType_Count,
    _AudioDeviceType_Force32 = 0x7FFFFFFF
} AudioDeviceType;

typedef enum AudioEngineState {
    AudioEngineState_Uninitialized,
    AudioEngineState_Stopped,
    AudioEngineState_Started,
    AudioEngineState_Starting,
    AudioEngineState_Stopping,

    _AudioEngineState_Count,
    _AudioEngineState_Force32 = 0x7FFFFFFF
} AudioEngineState;

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

/* Structs */
typedef struct AudioDevice {
    size_t idSize;
    const void* id;
    const char* name;
    Bool32 isDefault;
} AudioDevice;

typedef struct AudioConfig {
    /// Audio output channel count.
    uint32_t channelCount DEFAULT_INITIALIZER(0);
    /// Audio output sample rate.
    uint32_t sampleRate DEFAULT_INITIALIZER(0);
} AudioConfig;

/* Callbacks */
typedef void AudioDeviceCallback(AudioDevice* device, void* userdata);

ALIMER_API AudioContext* alimerAudioContextInit(void);
ALIMER_API void alimerAudioContextDestroy(AudioContext* context);
ALIMER_API void alimerAudioContextEnumerateDevices(AudioContext* context, AudioDeviceType type, AudioDeviceCallback* callback, void* userdata);

ALIMER_API AudioEngine* alimerAudioEngineCreate(AudioContext* context, const AudioConfig* config);
ALIMER_API void alimerAudioEngineDestroy(AudioEngine* engine);
ALIMER_API void alimerAudioEngineStart(AudioEngine* engine);
ALIMER_API void alimerAudioEngineStop(AudioEngine* engine);

ALIMER_API AudioEngineState alimerAudioEngineGetState(AudioEngine* engine);
ALIMER_API float alimerAudioEngineGetMasterVolume(AudioEngine* engine, VolumeUnit unit);
ALIMER_API void alimerAudioEngineSetMasterVolume(AudioEngine* engine, float value, VolumeUnit unit);
ALIMER_API float alimerAudioEngineGetVolume(AudioEngine* engine, VolumeUnit unit);
ALIMER_API void alimerAudioEngineSetVolume(AudioEngine* engine, float value, VolumeUnit unit);
ALIMER_API uint32_t alimerAudioEngineGetChannelCount(AudioEngine* engine);
ALIMER_API uint32_t alimerAudioEngineGetSampleRate(AudioEngine* engine);

#endif /* ALIMER_AUDIO_H_ */
