// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

//#define TEST_PHYSICS

#include "alimer_image.h"
#if defined(ALIMER_AUDIO)
#include "alimer_audio.h"
#endif

#if defined(ALIMER_GPU)
#include "alimer_gpu.h"
#endif

#if defined(ALIMER_PHYSICS)
#include "alimer_physics.h"
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

#if defined(ALIMER_AUDIO)
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
#if defined(ALIMER_AUDIO) && TODO
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
        .preferredBackend = GPUBackendType_Vulkan,
        .validationMode = GPUValidationMode_Enabled
    };
    GPUFactory gpuFactory =  agpuCreateFactory(&factoryDesc);
#endif

#if defined(ALIMER_PHYSICS)
    // Physics
    PhysicsConfig physicsConfig = { 0 };
    if (!alimerPhysicsInit(&physicsConfig))
    {
        return EXIT_FAILURE;
    }

    PhysicsWorldConfig physicsWorldConfig = { 0 };
    PhysicsWorld* physicsWorld = alimerPhysicsWorldCreate(&physicsWorldConfig);
#endif

#if defined(ALIMER_AUDIO) && TODO
    while (true)
    {

    }

    alimerAudioSourceRelease(source1);
    alimerAudioSourceRelease(source2);
    alimerAudioEngineDestroy(engine);
    alimerAudioShutdown();
#endif

#if defined(ALIMER_GPU)
    agpuFactoryDestroy(gpuFactory);
#endif


#if defined(ALIMER_PHYSICS)
    alimerPhysicsWorldDestroy(physicsWorld);
    alimerPhysicsShutdown();
#endif
    return EXIT_SUCCESS;
}
