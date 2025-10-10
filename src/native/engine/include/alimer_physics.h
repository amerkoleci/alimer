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

typedef enum PhysicsBodyType {
    PhysicsBodyType_Static,
    PhysicsBodyType_Kinematic,
    PhysicsBodyType_Dynamic,

    _PhysicsBodyType_Count,
    _PhysicsBodyType_Force32 = 0x7FFFFFFF
} PhysicsBodyType;

typedef enum PhysicsShapeType {
    PhysicsShapeType_Box,
    PhysicsShapeType_Sphere,
    PhysicsShapeType_Capsule,
    PhysicsShapeType_Cylinder,
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


typedef struct PhysicsBodyTransform {
    Vector3 position;
    Quaternion rotation;
} PhysicsBodyTransform;

typedef struct PhysicsBodyDesc {
    PhysicsBodyType type;
    PhysicsBodyTransform initialTransform;
    float mass;
    float linearDamping;
    float angularDamping;
    float gravityScale;
    bool isSensor;
    bool allowSleeping;
    bool continuous;
    uint32_t shapeCount;
    PhysicsShape** shapes;
} PhysicsBodyDesc;

ALIMER_API bool alimerPhysicsInit(const PhysicsConfig* config);
ALIMER_API void alimerPhysicsShutdown(void);

/* World */
ALIMER_API PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* config);
ALIMER_API void alimerPhysicsWorldDestroy(PhysicsWorld* world);
ALIMER_API uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world);
ALIMER_API uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world);
ALIMER_API void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vector3* gravity);
ALIMER_API void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vector3* gravity);
ALIMER_API bool alimerPhysicsWorldUpdate(PhysicsWorld* world, float deltaTime, int collisionSteps);
ALIMER_API void alimerPhysicsWorldOptimizeBroadPhase(PhysicsWorld* world);

/* Material */
ALIMER_API PhysicsMaterial* alimerPhysicsMaterialCreate(const char* name, float friction, float restitution);
ALIMER_API uint32_t alimerPhysicsMaterialAddRef(PhysicsMaterial* material);
ALIMER_API uint32_t alimerPhysicsMaterialRelease(PhysicsMaterial* material);

/* Shape */
ALIMER_API void alimerPhysicsShapeAddRef(PhysicsShape* shape);
ALIMER_API void alimerPhysicsShapeRelease(PhysicsShape* shape);
ALIMER_API bool alimerPhysicsShapeIsValid(PhysicsShape* shape);
ALIMER_API PhysicsShapeType alimerPhysicsShapeGetType(PhysicsShape* shape);
ALIMER_API PhysicsBody* alimerPhysicsShapeGetBody(PhysicsShape* shape);
ALIMER_API void* alimerPhysicsShapeGetUserData(PhysicsShape* shape);
ALIMER_API void alimerPhysicsShapeSetUserData(PhysicsShape* shape, void* userdata);
ALIMER_API float alimerPhysicsShapeGetVolume(PhysicsShape* shape);
ALIMER_API float alimerPhysicsShapeGetDensity(PhysicsShape* shape);
ALIMER_API float alimerPhysicsShapeGetMass(PhysicsShape* shape);

ALIMER_API PhysicsShape* alimerPhysicsShapeCreateBox(const Vector3* size, PhysicsMaterial* material);
ALIMER_API PhysicsShape* alimerPhysicsShapeCreateSphere(float radius, PhysicsMaterial* material);
ALIMER_API PhysicsShape* alimerPhysicsShapeCreateCapsule(float height, float radius, PhysicsMaterial* material);
ALIMER_API PhysicsShape* alimerPhysicsShapeCreateCylinder(float height, float radius, PhysicsMaterial* material);

/* Body */
ALIMER_API void alimerPhysicsBodyDescInit(PhysicsBodyDesc* desc);
ALIMER_API PhysicsBody* alimerPhysicsBodyCreate(PhysicsWorld* world, const PhysicsBodyDesc* desc);
ALIMER_API void alimerPhysicsBodyAddRef(PhysicsBody* body);
ALIMER_API void alimerPhysicsBodyRelease(PhysicsBody* body);
ALIMER_API bool alimerPhysicsBodyIsValid(PhysicsBody* body);
ALIMER_API bool alimerPhysicsBodyIsActive(PhysicsBody* body);
ALIMER_API PhysicsWorld* alimerPhysicsBodyGetWorld(PhysicsBody* body);
ALIMER_API uint32_t alimerPhysicsBodyGetID(PhysicsBody* body);
ALIMER_API void alimerPhysicsBodyGetTransform(PhysicsBody* body, PhysicsBodyTransform* transform);
ALIMER_API void alimerPhysicsBodySetTransform(PhysicsBody* body, const PhysicsBodyTransform* transform);
ALIMER_API void alimerPhysicsBodyGetWorldTransform(PhysicsBody* body, Matrix4x4* transform);

ALIMER_API void alimerPhysicsBodyGetLinearVelocity(PhysicsBody* body, Vector3* velocity);
ALIMER_API void alimerPhysicsBodySetLinearVelocity(PhysicsBody* body, const Vector3* velocity);
ALIMER_API void alimerPhysicsBodyGetAngularVelocity(PhysicsBody* body, Vector3* velocity);
ALIMER_API void alimerPhysicsBodySetAngularVelocity(PhysicsBody* body, const Vector3* velocity);

#endif /* ALIMER_PHYSICS_H_ */
