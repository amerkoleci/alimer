// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_PHYSICS_H_
#define ALIMER_PHYSICS_H_ 1

#include "alimer.h"

/* Forward */
typedef struct PhysicsWorld PhysicsWorld;
typedef struct PhysicsBody PhysicsBody;
typedef struct PhysicsShape PhysicsShape;
typedef struct PhysicsMaterial PhysicsMaterial;

typedef enum PhysicsShapeType {
    PhysicsShapeType_Box,
    PhysicsShapeType_Sphere,
    PhysicsShapeType_Capsule,
    PhysicsShapeType_Cylindex,
    PhysicsShapeType_Convex,
    PhysicsShapeType_Mesh,
    PhysicsShapeType_Terrain,

    PhysicsShapeType_Count,
    _PhysicsShapeType_Force32 = 0x7FFFFFFF
} PhysicsShapeType;

typedef struct PhysicsWorldConfig {
    uint32_t maxBodies;
    uint32_t maxBodyPairs;
} PhysicsWorldConfig;

typedef struct PhysicsConfig {
    uint32_t tempAllocatorInitSize;
    uint32_t maxPhysicsJobs;
    uint32_t maxPhysicsBarriers;
} PhysicsConfig;

ALIMER_API bool alimerPhysicsInit(const PhysicsConfig* config);
ALIMER_API void alimerPhysicsShutdown(void);

/* World */
ALIMER_API PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* config);
ALIMER_API void alimerPhysicsWorldDestroy(PhysicsWorld* world);
ALIMER_API uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world);
ALIMER_API uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world);
ALIMER_API void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vector3* gravity);
ALIMER_API void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vector3* gravity);
ALIMER_API void alimerPhysicsWorldUpdate(PhysicsWorld* world, float deltaTime, int collisionSteps);

/* Shape */
ALIMER_API void alimerPhysicsShapeAddRef(PhysicsShape* shape);
ALIMER_API void alimerPhysicsShapeRelease(PhysicsShape* shape);
ALIMER_API bool alimerPhysicsShapeIsValid(PhysicsShape* shape);
ALIMER_API PhysicsShapeType alimerPhysicsShapeGetType(PhysicsShape* shape);
ALIMER_API PhysicsShape* alimerPhysicsCreateBoxShape(float dimensions[3]);

/* Body */
ALIMER_API PhysicsBody* alimerPhysicsBodyCreate(PhysicsWorld* world, PhysicsShape* shape);
ALIMER_API void alimerPhysicsBodyAddRef(PhysicsBody* body);
ALIMER_API void alimerPhysicsBodyRelease(PhysicsBody* body);
ALIMER_API void alimerPhysicsBodyGetPositionAndRotation(PhysicsBody* body, Vector3* position, Quaternion* rotation);
ALIMER_API void alimerPhysicsBodySetPositionAndRotation(PhysicsBody* body, const Vector3* position, const Quaternion* rotation);
ALIMER_API void alimerPhysicsBodyGetWorldTransform(PhysicsBody* body, Matrix4x4* transform);

#endif /* ALIMER_PHYSICS_H_ */
