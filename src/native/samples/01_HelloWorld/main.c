// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

//#define TEST_PHYSICS

#include "alimer_image.h"
#if defined(ALIMER_AUDIO)
#include "alimer_audio.h"
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
#if defined(ALIMER_AUDIO)
    AudioContext* context = alimerAudioContextInit();
    alimerAudioContextEnumerateDevices(context, OnAudioDeviceCallback, NULL);
    AudioEngine* engine = alimerAudioEngineCreate(context, NULL);
#endif

    Image* image = alimerImageCreate1D(PixelFormat_RGBA8Unorm, 512, 1, 0);
    assert(alimerImageGetMipLevelCount(image) == 10);
    alimerImageDestroy(image);

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

#if defined(ALIMER_AUDIO)
    alimerAudioEngineDestroy(engine);
    alimerAudioContextDestroy(context);
#endif

#if defined(ALIMER_PHYSICS)
    alimerPhysicsWorldDestroy(physicsWorld);
    alimerPhysicsShutdown();
#endif
    return EXIT_SUCCESS;
}
