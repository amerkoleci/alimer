// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_PHYSICS_H_
#define ALIMER_PHYSICS_H_ 1

#if defined(ALIMER_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __declspec(dllexport)
#        else
#            define _ALIMER_EXPORT __declspec(dllimport)
#        endif
#    else
#        if defined(ALIMER_IMPLEMENTATION)
#            define _ALIMER_EXPORT __attribute__((visibility("default")))
#        else
#            define _ALIMER_EXPORT
#        endif
#    endif
#else
#    define _ALIMER_EXPORT
#endif

#ifdef __cplusplus
#    define _ALIMER_EXTERN extern "C"
#else
#    define _ALIMER_EXTERN extern
#endif

#define ALIMER_PHYSICS_API _ALIMER_EXTERN _ALIMER_EXPORT

#ifdef __cplusplus
#   define ALIMER_PHYSICS_DEFAULT_INITIALIZER(x) = x
#else
#   define ALIMER_PHYSICS_DEFAULT_INITIALIZER(x)
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Version API */
#define ALIMER_PHYSICS_VERSION_MAJOR    1
#define ALIMER_PHYSICS_VERSION_MINOR    0
#define ALIMER_PHYSICS_VERSION_PATCH	0

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
    PhysicsShapeType_ConvexHull,
    PhysicsShapeType_Mesh,

    PhysicsShapeType_Count,
    _PhysicsShapeType_Force32 = 0x7FFFFFFF
} PhysicsShapeType;

typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct Quaternion {
    float x;
    float y;
    float z;
    float w;
} Quaternion;

typedef struct PhysicsWorldConfig {
    uint32_t maxPhysicsBodies ALIMER_PHYSICS_DEFAULT_INITIALIZER(65536);
    uint32_t maxPhysicsContactConstraints ALIMER_PHYSICS_DEFAULT_INITIALIZER(131072);
    Vector3 gravity;
} PhysicsWorldConfig;

typedef struct PhysicsMaterialDesc {
    const char* name;
    float friction;
    float restitution;
} PhysicsMaterialDesc;

typedef struct PhysicsBodyDesc {
    PhysicsBodyType type;
    Vector3 position;
    Quaternion rotation;
    Vector3 linearVelocity;
    Vector3 angularVelocity;
    float mass;
    float linearDamping;
    float angularDamping;
    float gravityScale;
    bool isSensor;
    bool allowSleeping;
    bool useContinuousCollision;
    uint32_t shapeCount;
    PhysicsShape** shapes;
} PhysicsBodyDesc;

// Custom memory allocator function signatures.
typedef void* (*PhysicsAllocCallback)(size_t size, void* userData);
typedef void  (*PhysicsFreeCallback)(void* ptr, void* userData);
ALIMER_PHYSICS_API void alimerPhysicsSetAllocationCallbacks(PhysicsAllocCallback alloc, PhysicsFreeCallback free, void* userData);

ALIMER_PHYSICS_API bool alimerPhysicsInit(void);
ALIMER_PHYSICS_API void alimerPhysicsShutdown(void);

/* World */
ALIMER_PHYSICS_API PhysicsWorldConfig alimerPhysicsWorldConfigDefault(void);
ALIMER_PHYSICS_API PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* config);
ALIMER_PHYSICS_API void alimerPhysicsWorldDestroy(PhysicsWorld* world);
ALIMER_PHYSICS_API uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world);
ALIMER_PHYSICS_API uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world);
ALIMER_PHYSICS_API void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vector3* gravity);
ALIMER_PHYSICS_API void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vector3* gravity);
ALIMER_PHYSICS_API void alimerPhysicsWorldUpdate(PhysicsWorld* world, float timeStep, int collisionSteps);

/* Material */
ALIMER_PHYSICS_API PhysicsMaterial* alimerPhysicsMaterialCreate(const PhysicsMaterialDesc* desc);
ALIMER_PHYSICS_API void alimerPhysicsMaterialAddRef(PhysicsMaterial* material);
ALIMER_PHYSICS_API void alimerPhysicsMaterialRelease(PhysicsMaterial* material);

/* Shape */
ALIMER_PHYSICS_API void alimerPhysicsShapeAddRef(PhysicsShape* shape);
ALIMER_PHYSICS_API void alimerPhysicsShapeRelease(PhysicsShape* shape);
ALIMER_PHYSICS_API bool alimerPhysicsShapeIsValid(PhysicsShape* shape);
ALIMER_PHYSICS_API PhysicsShapeType alimerPhysicsShapeGetType(PhysicsShape* shape);
ALIMER_PHYSICS_API PhysicsBody* alimerPhysicsShapeGetBody(PhysicsShape* shape);

ALIMER_PHYSICS_API float alimerPhysicsShapeGetVolume(PhysicsShape* shape);
ALIMER_PHYSICS_API float alimerPhysicsShapeGetDensity(PhysicsShape* shape);

ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsShapeCreateBox(const Vector3* size, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsShapeCreateSphere(float radius, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsShapeCreateCapsule(float height, float radius, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsShapeCreateCylinder(float height, float radius, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsShapeCreateConvexHull(const Vector3* points, uint32_t pointsCount, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsShapeCreateMesh(const Vector3* vertices, uint32_t verticesCount, const uint32_t* indices, uint32_t indicesCount);

/* Body */
ALIMER_PHYSICS_API PhysicsBodyDesc alimerPhysicsBodyDescDefault(void);
ALIMER_PHYSICS_API PhysicsBody* alimerPhysicsBodyCreate(PhysicsWorld* world, const PhysicsBodyDesc* desc);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddRef(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyRelease(PhysicsBody* body);
ALIMER_PHYSICS_API bool alimerPhysicsBodyIsValid(PhysicsBody* body);

ALIMER_PHYSICS_API PhysicsWorld* alimerPhysicsBodyGetWorld(PhysicsBody* body);
ALIMER_PHYSICS_API uint32_t alimerPhysicsBodyGetID(PhysicsBody* body);

ALIMER_PHYSICS_API PhysicsBodyType alimerPhysicsBodyGetType(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodySetType(PhysicsBody* body, PhysicsBodyType value);

ALIMER_PHYSICS_API void alimerPhysicsBodyGetPosition(PhysicsBody* body, Vector3* position);
ALIMER_PHYSICS_API void alimerPhysicsBodyGetRotation(PhysicsBody* body, Quaternion* rotation);

ALIMER_PHYSICS_API void alimerPhysicsBodySetTransform(PhysicsBody* body, const Vector3* position, const Quaternion* rotation);

ALIMER_PHYSICS_API float alimerPhysicsBodyGetMass(PhysicsBody* body);
ALIMER_PHYSICS_API float alimerPhysicsBodyGetInverseMass(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyGetCenterOfMassPosition(PhysicsBody* body, Vector3* position);

ALIMER_PHYSICS_API bool alimerPhysicsBodyIsActive(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyActivateBody(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyDeactivateBody(PhysicsBody* body);

ALIMER_PHYSICS_API float alimerPhysicsBodyGetLinearDamping(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodySetLinearDamping(PhysicsBody* body, float value);

ALIMER_PHYSICS_API float alimerPhysicsBodyGetAngularDamping(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodySetAngularDamping(PhysicsBody* body, float value);

ALIMER_PHYSICS_API float alimerPhysicsBodyGetGravityScale(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodySetGravityScale(PhysicsBody* body, float value);

ALIMER_PHYSICS_API void alimerPhysicsBodyGetLinearVelocity(PhysicsBody* body, Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodySetLinearVelocity(PhysicsBody* body, const Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodyGetAngularVelocity(PhysicsBody* body, Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodySetAngularVelocity(PhysicsBody* body, const Vector3* velocity);

ALIMER_PHYSICS_API void alimerPhysicsBodyAddForce(PhysicsBody* body, const Vector3* force);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddForceAtPosition(PhysicsBody* body, const Vector3* force, const Vector3* position);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddTorque(PhysicsBody* body, const Vector3* torque);

ALIMER_PHYSICS_API void alimerPhysicsBodyAddImpulse(PhysicsBody* body, const Vector3* impulse);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddImpulseAtPosition(PhysicsBody* body, const Vector3* impulse, const Vector3* position);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddAngularImpulse(PhysicsBody* body, const Vector3* angularImpulse);
ALIMER_PHYSICS_API bool alimerPhysicsBodyApplyBuoyancyImpulse(PhysicsBody* body, const Vector3* surfacePosition, const Vector3* surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3* fluidVelocity, const Vector3* gravity, float deltaTime);

#endif /* ALIMER_PHYSICS_H_ */
