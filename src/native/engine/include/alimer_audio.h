// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_AUDIO_H_
#define ALIMER_AUDIO_H_ 1

#include "alimer_platform.h"

/* Forward */
typedef struct AudioDevice AudioDevice;
typedef struct AudioEngine AudioEngine;
typedef struct AudioClip AudioClip;
typedef struct AudioSource AudioSource;

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
typedef struct AudioConfig {
    AudioDevice* playbackDevice DEFAULT_INITIALIZER(nullptr);
    /// Audio output channel count.
    uint32_t channelCount DEFAULT_INITIALIZER(2);
    /// Audio output sample rate.
    uint32_t sampleRate DEFAULT_INITIALIZER(48000);
} AudioConfig;

/* Callbacks */
typedef void AudioDeviceCallback(AudioDevice* device, void* userdata);

/* AudioContext */
ALIMER_API bool alimerAudioInit(void);
ALIMER_API void alimerAudioShutdown(void);
ALIMER_API void alimerAudioEnumerateDevices(AudioDeviceCallback* callback, void* userdata);

/* AudioDevice */
ALIMER_API AudioDeviceType alimerAudioDeviceGetType(AudioDevice* device);
ALIMER_API const char* alimerAudioDeviceGetName(AudioDevice* device);
ALIMER_API bool alimerAudioDeviceIsDefault(AudioDevice* device);

/* AudioEngine */
ALIMER_API AudioEngine* alimerAudioEngineCreate(const AudioConfig* config);
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
ALIMER_API uint64_t alimerAudioEngineGetTimeInPCMFrames(AudioEngine* engine);
ALIMER_API uint64_t alimerAudioEngineGetTimeInMilliseconds(AudioEngine* engine);
ALIMER_API void alimerAudioEngineSetTimeInPCMFrames(AudioEngine* engine, uint64_t value);
ALIMER_API void alimerAudioEngineSetTimeInMilliseconds(AudioEngine* engine, uint64_t value);

/* AudioListener */
ALIMER_API uint32_t alimerAudioEngineGetListenerCount(AudioEngine* engine);
ALIMER_API void alimerAudioEngineListenerSetPosition(AudioEngine* engine, uint32_t listenerIndex, const Vector3* position);
ALIMER_API void alimerAudioEngineListenerGetPosition(const AudioEngine* engine, uint32_t listenerIndex, Vector3* result);
ALIMER_API bool alimerAudioEngineListenerIsEnabled(AudioEngine* engine, uint32_t listenerIndex);
ALIMER_API void alimerAudioEngineListenerSetEnabled(AudioEngine* engine, uint32_t listenerIndex, bool enabled);

/* AudioClip */
ALIMER_API AudioClip* alimerAudioClipCreate(const char* filepath);
ALIMER_API AudioClip* alimerAudioClipCreateFromMemory(const void* pData, size_t dataSize);
ALIMER_API uint32_t alimerAudioClipAddRef(AudioClip* clip);
ALIMER_API uint32_t alimerAudioClipRelease(AudioClip* clip);

/* AudioSource */
ALIMER_API AudioSource* alimerAudioSourceCreate(AudioEngine* engine, AudioClip* clip);
ALIMER_API uint32_t alimerAudioSourceAddRef(AudioSource* source);
ALIMER_API uint32_t alimerAudioSourceRelease(AudioSource* source);
ALIMER_API void alimerAudioSourcePlay(AudioSource* source);
ALIMER_API void alimerAudioSourceStop(AudioSource* source);
ALIMER_API bool alimerAudioSourceIsPlaying(AudioSource* source);

#endif /* ALIMER_AUDIO_H_ */
