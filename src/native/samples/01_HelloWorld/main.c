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

typedef struct TestStruct {
    bool isInitialized;
    uint32_t a;
} TestStruct;

int main(void)
{
    if (!alimerPlatformInit())
    {
        return EXIT_FAILURE;
    }

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
        .preferredBackend = GPUBackendType_Vulkan,
        .validationMode = GPUValidationMode_Enabled
    };
    GPUFactory gpuFactory = agpuFactoryCreate(&factoryDesc);
    GPUAdapter adapter = agpuFactoryGetBestAdapter(gpuFactory);
    GPUDevice device = agpuAdapterCreateDevice(adapter, NULL);
    GPUSampler* sampler = agpuSamplerCreate(device, NULL);
#endif

#if defined(ALIMER_PHYSICS)
    // Physics
    if (!alimerPhysicsInit(NULL))
    {
        return EXIT_FAILURE;
    }

    PhysicsWorldConfig physicsWorldConfig = { 0 };
    PhysicsWorld* physicsWorld = alimerPhysicsWorldCreate(&physicsWorldConfig);

    // Create floor
    PhysicsShape* floorShape = alimerPhysicsShapeCreateBox(&(Vec3) { 100.0f, 1.0f, 100.0f }, NULL);
    PhysicsBodyDesc floorBodyDesc;
    alimerPhysicsBodyDescInit(&floorBodyDesc);
    floorBodyDesc.initialTransform.position = (Vec3){ 0.0f, -1.0f, 0.0f };
    floorBodyDesc.type = PhysicsBodyType_Static;
    floorBodyDesc.shapeCount = 1;
    floorBodyDesc.shapes = &floorShape;
    PhysicsBody* floorBody = alimerPhysicsBodyCreate(physicsWorld, &floorBodyDesc);

    // Create sphere
    PhysicsShape* sphereShape = alimerPhysicsShapeCreateSphere(0.5f, NULL);
    PhysicsBodyDesc sphereBodyDesc;
    alimerPhysicsBodyDescInit(&sphereBodyDesc);
    sphereBodyDesc.initialTransform.position = (Vec3){ 0.0f, 2.0f, 0.0f };
    sphereBodyDesc.type = PhysicsBodyType_Dynamic;
    sphereBodyDesc.shapeCount = 1;
    sphereBodyDesc.shapes = &sphereShape;
    PhysicsBody* sphereBody = alimerPhysicsBodyCreate(physicsWorld, &sphereBodyDesc);
    alimerPhysicsBodySetLinearVelocity(sphereBody, &(Vec3){ 0.0f, -5.0f, 0.0f });

    float density = alimerPhysicsShapeGetDensity(sphereShape);
    float volume = alimerPhysicsShapeGetVolume(sphereShape);
    float mass = alimerPhysicsShapeGetMass(sphereShape);
    float bodyMass = alimerPhysicsBodyGetMass(sphereBody);
    float bodyInverseMass = alimerPhysicsBodyGetInverseMass(sphereBody);
    (void)density;
    (void)volume;
    (void)mass;
    (void)bodyMass;
    (void)bodyInverseMass;

    const float cDeltaTime = 1.0f / 60.0f;

    alimerPhysicsWorldOptimizeBroadPhase(physicsWorld);

    uint32_t step = 0;
    while (alimerPhysicsBodyIsActive(sphereBody))
    {
        // Next step
        ++step;


        // Output current position and velocity of the sphere
        Vec3 position, velocity;
        alimerPhysicsBodyGetCenterOfMassPosition(sphereBody, &position);
        alimerPhysicsBodyGetLinearVelocity(sphereBody, &velocity);
        printf("Step %u: Position = (%f, %f, %f), Velocity = (%f, %f, %f)\n", step, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z);

        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
        const int cCollisionSteps = 1;

        // Step the world
        alimerPhysicsWorldUpdate(physicsWorld, cDeltaTime, cCollisionSteps);
    }
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
    agpuSamplerDestroy(sampler);
    agpuDeviceRelease(device);
    agpuFactoryDestroy(gpuFactory);
#endif


#if defined(ALIMER_PHYSICS)
    alimerPhysicsBodyDestroy(sphereBody);
    alimerPhysicsBodyDestroy(floorBody);
    alimerPhysicsWorldDestroy(physicsWorld);
    alimerPhysicsShutdown();
#endif

    alimerPlatformShutdown();

    return EXIT_SUCCESS;
}
