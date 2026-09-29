// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_image.h"
#if defined(ALIMER_AUDIO)
#include "alimer_audio.h"
#endif

#if defined(ALIMER_GPU)
#include "alimer_gpu.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include <assert.h>

#define ALIMER_UNUSED(x) (void)(x)

#if defined(ALIMER_AUDIO) && defined(TEST_AUDIO)
static void OnAudioDeviceCallback(AudioDevice* device, void* userdata)
{
    AudioDeviceType type = alimerAudioDeviceGetType(device);
    const char* name = alimerAudioDeviceGetName(device);
    Bool32 isDefault = alimerAudioDeviceIsDefault(device);
    ALIMER_UNUSED(type);
    ALIMER_UNUSED(name);
    ALIMER_UNUSED(isDefault);
    ALIMER_UNUSED(userdata);
}
#endif

int main(void)
{
#if defined(ALIMER_AUDIO) && defined(TEST_AUDIO)
    if (!alimerAudioInit())
    {
        return EXIT_FAILURE;
    }

    alimerAudioEnumerateDevices(OnAudioDeviceCallback, NULL);
    AudioEngine* engine = alimerAudioEngineCreate(NULL);

    AudioClip* clip1 = alimerAudioClipCreate("audio/shortcuts.ogg");
    AudioClip* clip2 = alimerAudioClipCreate("audio/BGM.mp3");

    // Source 1 with clip 1
    AudioSource* source1 = alimerAudioSourceCreate(engine, clip1);
    alimerAudioSourcePlay(source1);

    // Source 2 with clip 2
    AudioSource* source2 = alimerAudioSourceCreate(engine, clip2);
    alimerAudioSourcePlay(source2);

    alimerAudioClipRelease(clip1);
    alimerAudioClipRelease(clip2);
#endif

    Image* image = alimerImageCreate1D(PixelFormat_RGBA8Unorm, 512, 1, 0);
    assert(alimerImageGetMipLevelCount(image) == 10);
    alimerImageDestroy(image);

#if defined(ALIMER_GPU)
    const GPUFactoryDesc factoryDesc = {
        .preferredBackend = GPUBackendType_D3D12,
        .validationMode = GPUValidationMode_Enabled
    };
    GPUFactory* gpuFactory = agpuFactoryCreate(&factoryDesc);
    GPUAdapter* adapter = agpuFactoryGetBestAdapter(gpuFactory);
    GPUDevice device = agpuDeviceCreate(adapter, NULL);
    GPUSampler* sampler = agpuSamplerCreate(device, NULL);
#endif

#if defined(ALIMER_AUDIO) && defined(TEST_AUDIO)
    while (alimerAudioSourceIsPlaying(source2))
    {

    }

    alimerAudioSourceRelease(source1);
    alimerAudioSourceRelease(source2);
    alimerAudioEngineDestroy(engine);
    alimerAudioShutdown();
#endif

#if defined(ALIMER_GPU)
    agpuSamplerRelease(sampler);
    agpuDeviceRelease(device);
    agpuFactoryDestroy(gpuFactory);
#endif

    return EXIT_SUCCESS;
}
