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
    PhysicsShapeType_Terrain,

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

/// 4x4 row-major matrix: 32 bit floating point components
typedef struct Matrix4x4 {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} Matrix4x4;

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

ALIMER_PHYSICS_API bool alimerPhysicsInit(const PhysicsConfig* config);
ALIMER_PHYSICS_API void alimerPhysicsShutdown(void);

/* World */
ALIMER_PHYSICS_API PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* config);
ALIMER_PHYSICS_API void alimerPhysicsWorldDestroy(PhysicsWorld* world);
ALIMER_PHYSICS_API uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world);
ALIMER_PHYSICS_API uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world);
ALIMER_PHYSICS_API void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vector3* gravity);
ALIMER_PHYSICS_API void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vector3* gravity);
ALIMER_PHYSICS_API bool alimerPhysicsWorldUpdate(PhysicsWorld* world, float deltaTime, int collisionSteps);
ALIMER_PHYSICS_API void alimerPhysicsWorldOptimizeBroadPhase(PhysicsWorld* world);

/* Material */
ALIMER_PHYSICS_API PhysicsMaterial* alimerPhysicsMaterialCreate(const char* name, float friction, float restitution);
ALIMER_PHYSICS_API uint32_t alimerPhysicsMaterialAddRef(PhysicsMaterial* material);
ALIMER_PHYSICS_API uint32_t alimerPhysicsMaterialRelease(PhysicsMaterial* material);

/* Shape */
ALIMER_PHYSICS_API void alimerPhysicsShapeAddRef(PhysicsShape* shape);
ALIMER_PHYSICS_API void alimerPhysicsShapeRelease(PhysicsShape* shape);
ALIMER_PHYSICS_API bool alimerPhysicsShapeIsValid(PhysicsShape* shape);
ALIMER_PHYSICS_API PhysicsShapeType alimerPhysicsShapeGetType(PhysicsShape* shape);
ALIMER_PHYSICS_API PhysicsBody* alimerPhysicsShapeGetBody(PhysicsShape* shape);
ALIMER_PHYSICS_API void* alimerPhysicsShapeGetUserData(PhysicsShape* shape);
ALIMER_PHYSICS_API void alimerPhysicsShapeSetUserData(PhysicsShape* shape, void* userdata);
ALIMER_PHYSICS_API float alimerPhysicsShapeGetVolume(PhysicsShape* shape);
ALIMER_PHYSICS_API float alimerPhysicsShapeGetDensity(PhysicsShape* shape);
ALIMER_PHYSICS_API float alimerPhysicsShapeGetMass(PhysicsShape* shape);

ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsCreateBoxShape(const Vector3* size, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsCreateSphereShape(float radius, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsCreateCapsuleShape(float height, float radius, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsCreateCylinderShape(float height, float radius, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsCreateConvexHullShape(const Vector3* points, uint32_t pointsCount, PhysicsMaterial* material);
ALIMER_PHYSICS_API PhysicsShape* alimerPhysicsCreateMeshShape(const Vector3* vertices, uint32_t verticesCount, const uint32_t* indices, uint32_t indicesCount);

/* Body */
ALIMER_PHYSICS_API void alimerPhysicsBodyDescInit(PhysicsBodyDesc* desc);
ALIMER_PHYSICS_API PhysicsBody* alimerPhysicsBodyCreate(PhysicsWorld* world, const PhysicsBodyDesc* desc);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddRef(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyRelease(PhysicsBody* body);
ALIMER_PHYSICS_API bool alimerPhysicsBodyIsValid(PhysicsBody* body);

ALIMER_PHYSICS_API PhysicsWorld* alimerPhysicsBodyGetWorld(PhysicsBody* body);
ALIMER_PHYSICS_API uint32_t alimerPhysicsBodyGetID(PhysicsBody* body);

ALIMER_PHYSICS_API PhysicsBodyType alimerPhysicsBodyGetType(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodySetType(PhysicsBody* body, PhysicsBodyType value);

ALIMER_PHYSICS_API void alimerPhysicsBodyGetTransform(PhysicsBody* body, PhysicsBodyTransform* transform);
ALIMER_PHYSICS_API void alimerPhysicsBodySetTransform(PhysicsBody* body, const PhysicsBodyTransform* transform);
ALIMER_PHYSICS_API void alimerPhysicsBodyGetWorldTransform(PhysicsBody* body, Matrix4x4* transform);

ALIMER_PHYSICS_API bool alimerPhysicsBodyIsActive(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyActivateBody(PhysicsBody* body);
ALIMER_PHYSICS_API void alimerPhysicsBodyDeactivateBody(PhysicsBody* body);

ALIMER_PHYSICS_API void alimerPhysicsBodyGetLinearVelocity(PhysicsBody* body, Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodySetLinearVelocity(PhysicsBody* body, const Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodyGetAngularVelocity(PhysicsBody* body, Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodySetAngularVelocity(PhysicsBody* body, const Vector3* velocity);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddForce(PhysicsBody* body, const Vector3* force);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddForceAtPosition(PhysicsBody* body, const Vector3* force, const Vector3* position);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddTorque(PhysicsBody* body, const Vector3* torque);
ALIMER_PHYSICS_API void alimerPhysicsBodyAddForceAndTorque(PhysicsBody* body, const Vector3* force, const Vector3* torque);

#endif /* ALIMER_PHYSICS_H_ */
