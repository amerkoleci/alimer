// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

//#include "alimer_internal.h"
#include "alimer_physics.h"

#include "box3d/box3d.h"
#include <atomic>

#ifdef ALIMER_ENABLE_ASSERTS
#   include <assert.h>
#   define ALIMER_ASSERT(c) assert(c)
#else
#   define ALIMER_ASSERT(...) ((void)0)
#endif

#define ALIMER_UNUSED(x) (void)(x)

namespace
{
    static constexpr PhysicsBodyType FromBox3D(b3BodyType value)
    {
        switch (value)
        {
            case b3_kinematicBody:
                return PhysicsBodyType_Kinematic;
            case b3_dynamicBody:
                return PhysicsBodyType_Dynamic;

            case b3_staticBody:
            default:
                return PhysicsBodyType_Static;
        }
    }

    static void FromBox3D(const b3Vec3& value, Vec3* result)
    {
        result->x = value.x;
        result->y = value.y;
        result->z = value.z;
    }

    static void FromBox3D(const b3Quat& quat, Quat* result)
    {
        result->x = quat.v.x;
        result->y = quat.v.y;
        result->z = quat.v.z;
        result->w = quat.s;
    }

    [[maybe_unused]] static void FromBox3D(const b3Transform& value, Matrix4x4* result)
    {
        //JPH::Mat44 temp = value.Transposed();
        //memcpy(result, &temp, sizeof(Matrix4x4));
    }

    constexpr b3BodyType ToBox3D(PhysicsBodyType value)
    {
        switch (value)
        {
            case PhysicsBodyType_Kinematic:
                return b3_kinematicBody;
            case PhysicsBodyType_Dynamic:
                return b3_dynamicBody;
            case PhysicsBodyType_Static:
            default:
                return b3_staticBody;
        }
    }

    static b3Vec3 ToBox3D(const Vec3* value)
    {
        return { value->x, value->y, value->z };
    }

    static b3Quat ToBox3D(const Quat* value)
    {
        return { value->x, value->y, value->z, value->w };
    }

    //[[maybe_unused]] static JPH::Mat44 ToBox3D(const Matrix4x4* value)
    //{
    //    JPH::Mat44 result;
    //    memcpy(&result, value, sizeof(Matrix4x4));
    //
    //    return result.Transposed();
    //}
}

static struct
{
    bool initialized;
} physics_state = {};

struct PhysicsWorld final
{
    std::atomic_uint32_t refCount;
    b3WorldId id;
};

struct PhysicsMaterial final
{
    std::atomic_uint32_t refCount;
    //JPH::Ref<AlimerPhysicsMaterial> handle;
};

struct PhysicsBody final
{
    std::atomic_uint32_t refCount;
    PhysicsWorld* world;
    b3BodyId id;
};

struct PhysicsShape final
{
    std::atomic_uint32_t refCount;
    PhysicsShapeType type;
    Vec3 size;
    PhysicsBody* body;
    b3ShapeId id;
};

bool alimerPhysicsInit(const PhysicsConfig* config)
{
    //ALIMER_ASSERT(config);

    if (physics_state.initialized)
        return true;

    physics_state.initialized = true;
    return true;
}

void alimerPhysicsShutdown(void)
{
    if (!physics_state.initialized)
        return;

    physics_state.initialized = false;
    memset(&physics_state, 0, sizeof(physics_state));
}

static PhysicsWorldConfig PhysicsWorldConfig_Defaults(const PhysicsWorldConfig* pConfig)
{
    PhysicsWorldConfig config;
    if (pConfig != NULL)
    {
        config = *pConfig;
    }
    else
    {
        memset(&config, 0, sizeof(config));
    }

    return config;
}

PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* pConfig)
{
    PhysicsWorldConfig config = PhysicsWorldConfig_Defaults(pConfig);

    PhysicsWorld* world = new PhysicsWorld();
    world->refCount.store(1);

    // Init the physics system
    b3WorldDef worldDef = b3DefaultWorldDef();
    //worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };
    worldDef.userData = world;

    world->id = b3CreateWorld(&worldDef);

    if (!b3World_IsValid(world->id))
    {
        //alimerLogError(LogCategory_Physics, "Jolt: CreateBox failed with %s", shapeResult.GetError().c_str());
        return nullptr;
    }

    return world;
}

void alimerPhysicsWorldDestroy(PhysicsWorld* world)
{
    uint32_t newCount = --world->refCount;
    if (newCount == 0)
    {
        b3DestroyWorld(world->id);
        delete world;
    }
}

uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world)
{
    return 0;
}

uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world)
{
    return 0;
}

void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vec3* gravity)
{
    FromBox3D(b3World_GetGravity(world->id), gravity);
}

void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vec3* gravity)
{
    b3World_SetGravity(world->id, ToBox3D(gravity));
}

bool alimerPhysicsWorldUpdate(PhysicsWorld* world, float deltaTime, int collisionSteps)
{
    b3World_Step(world->id, deltaTime, collisionSteps);
    return true;
}

void alimerPhysicsWorldOptimizeBroadPhase(PhysicsWorld* world)
{
    ALIMER_UNUSED(world);
}

/* Material */
PhysicsMaterial* alimerPhysicsMaterialCreate(const char* name, float friction, float restitution)
{
    PhysicsMaterial* material = new PhysicsMaterial();
    material->refCount.store(1);
    //material->handle = new AlimerPhysicsMaterial(name, JPH::ColorArg(255, 0, 0), friction, restitution);
    return material;
}

uint32_t alimerPhysicsMaterialAddRef(PhysicsMaterial* material)
{
    return ++material->refCount;
}

uint32_t alimerPhysicsMaterialRelease(PhysicsMaterial* material)
{
    uint32_t newCount = --material->refCount;
    if (newCount == 0)
    {
        delete material;
    }

    return newCount;
}

void alimerPhysicsShapeAddRef(PhysicsShape* shape)
{
    ++shape->refCount;
}

void alimerPhysicsShapeRelease(PhysicsShape* shape)
{
    uint32_t result = --shape->refCount;
    if (result == 0)
    {
        delete shape;
    }
}

bool alimerPhysicsShapeIsValid(PhysicsShape* shape)
{
    return shape && b3Shape_IsValid(shape->id);
}

PhysicsShapeType alimerPhysicsShapeGetType(PhysicsShape* shape)
{
    return shape->type;
}

PhysicsBody* alimerPhysicsShapeGetBody(PhysicsShape* shape)
{
    return shape->body;
}

void* alimerPhysicsShapeGetUserData(PhysicsShape* shape)
{
    return b3Shape_GetUserData(shape->id);
}

void alimerPhysicsShapeSetUserData(PhysicsShape* shape, void* userdata)
{
    b3Shape_SetUserData(shape->id, userdata);
}

float alimerPhysicsShapeGetVolume(PhysicsShape* shape)
{
    return 0.f;
}

float alimerPhysicsShapeGetDensity(PhysicsShape* shape)
{
    if (shape->type == PhysicsShapeType_Mesh || shape->type == PhysicsShapeType_Terrain)
    {
        return 0.f;
    }

    return b3Shape_GetDensity(shape->id);
}

float alimerPhysicsShapeGetMass(PhysicsShape* shape)
{
    if (shape->type == PhysicsShapeType_Mesh || shape->type == PhysicsShapeType_Terrain)
    {
        return 0.0f;
    }

    return 0.f;
}

PhysicsShape* alimerPhysicsCreateBoxShape(const Vec3* size, PhysicsMaterial* material)
{
    ALIMER_ASSERT(size);
    ALIMER_ASSERT(size->x > 0.f && size->y > 0.f && size->z > 0.f);

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Box;
    shape->size.x = size->x;
    shape->size.y = size->y;
    shape->size.z = size->z;
    return shape;
}

PhysicsShape* alimerPhysicsCreateSphereShape(float radius, PhysicsMaterial* material)
{
    ALIMER_ASSERT(radius > 0.f);

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Sphere;
    shape->size.x = radius;
    shape->size.y = radius;
    shape->size.z = radius;
    return shape;
}

PhysicsShape* alimerPhysicsCreateCapsuleShape(float height, float radius, PhysicsMaterial* material)
{
    ALIMER_ASSERT(height > 0.f);
    ALIMER_ASSERT(radius > 0.f);

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Capsule;
    shape->size.x = radius;
    shape->size.y = height;
    shape->size.z = 0.f;
    return shape;
}

PhysicsShape* alimerPhysicsCreateCylinderShape(float height, float radius, PhysicsMaterial* material)
{
    ALIMER_ASSERT(height > 0.f);
    ALIMER_ASSERT(radius > 0.f);

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Cylinder;
    shape->size.x = radius;
    shape->size.y = height;
    shape->size.z = 0.f;
    return shape;
}

PhysicsShape* alimerPhysicsCreateConvexHullShape(const Vec3* points, uint32_t pointsCount, PhysicsMaterial* material)
{
    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_ConvexHull;
    // TODO: save points and pointsCount
    return shape;
}

PhysicsShape* alimerPhysicsCreateMeshShape(const Vec3* vertices, uint32_t verticesCount, const uint32_t* indices, uint32_t indicesCount)
{
    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Mesh;
    // TODO: save vertices, verticesCount, indices, and indicesCount
    return shape;
}

PhysicsShape* alimerPhysicsCreateTerrainShape(const float* samples, const Vec3* offset, const Vec3* scale, uint32_t sampleCount)
{
    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Terrain;
    // TODO: save samples, offset, scale, and sampleCount
    return shape;
}

/* Body */
void alimerPhysicsBodyDescInit(PhysicsBodyDesc* desc)
{
    memset(desc, 0, sizeof(PhysicsBodyDesc));

    desc->type = PhysicsBodyType_Dynamic;
    desc->mass = 1.0f;
    desc->linearDamping = 0.05f;
    desc->angularDamping = 0.05f;
    desc->gravityScale = 1.0f;
    desc->isSensor = false;
    desc->allowSleeping = true;
    desc->continuous = false;
    desc->shapeCount = 0;
    desc->shapes = nullptr;
}

PhysicsBody* alimerPhysicsBodyCreate(PhysicsWorld* world, const PhysicsBodyDesc* desc)
{
    if (!desc) {
        return nullptr;
    }

    if (desc->shapeCount > 0)
    {
        for (uint32_t i = 0; i < desc->shapeCount; i++)
        {
            if (desc->shapes[i]->body)
            {
                //alimerLogError(LogCategory_Physics, "PhysicsShape is already attached to another body");
                return nullptr;
            }
        }
    }

    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = ToBox3D(desc->type);
    bodyDef.position = ToBox3D(&desc->initialTransform.position);
    bodyDef.rotation = ToBox3D(&desc->initialTransform.rotation);
    bodyDef.linearDamping = desc->linearDamping;

    for (uint32_t i = 0; i < desc->shapeCount; i++)
    {
    }

    PhysicsBody* body = new PhysicsBody();
    body->refCount.store(1);
    body->world = world;
    body->id = b3CreateBody(world->id, &bodyDef);
    b3Body_SetUserData(body->id, body);

    // Assign the body to the shapes
    for (uint32_t i = 0; i < desc->shapeCount; i++)
    {
        desc->shapes[i]->body = body;
    }

    return body;
}

void alimerPhysicsBodyAddRef(PhysicsBody* body)
{
    ++body->refCount;
}

void alimerPhysicsBodyRelease(PhysicsBody* body)
{
    uint32_t result = --body->refCount;
    if (result == 0)
    {
        b3DestroyBody(body->id);
        body->id = {};
        body->world = nullptr;

        delete body;
    }
}

bool alimerPhysicsBodyIsValid(PhysicsBody* body)
{
    return body && b3Body_IsValid(body->id);
}

PhysicsWorld* alimerPhysicsBodyGetWorld(PhysicsBody* body)
{
    return body->world;
}

uint32_t alimerPhysicsBodyGetID(PhysicsBody* body)
{
    return body->id.index1;
}

PhysicsBodyType alimerPhysicsBodyGetType(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return FromBox3D(b3Body_GetType(body->id));
}

void alimerPhysicsBodySetType(PhysicsBody* body, PhysicsBodyType value)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetType(body->id, ToBox3D(value));
}

void alimerPhysicsBodyGetTransform(PhysicsBody* body, PhysicsBodyTransform* transform)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Vec3 position = b3Body_GetPosition(body->id);
    b3Quat rotation = b3Body_GetRotation(body->id);

    FromBox3D(position, &transform->position);
    FromBox3D(rotation, &transform->rotation);
}

void alimerPhysicsBodySetTransform(PhysicsBody* body, const PhysicsBodyTransform* transform)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetTransform(body->id, ToBox3D(&transform->position), ToBox3D(&transform->rotation));
}

void alimerPhysicsBodyGetWorldTransform(PhysicsBody* body, Matrix4x4* transform)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3WorldTransform worldTransform = b3Body_GetTransform(body->id);
    FromBox3D(worldTransform, transform);
}

bool alimerPhysicsBodyIsActive(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return b3Body_IsAwake(body->id);
}

void alimerPhysicsBodyActivateBody(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetAwake(body->id, true);
}

void alimerPhysicsBodyDeactivateBody(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetAwake(body->id, false);
}

void alimerPhysicsBodyGetLinearVelocity(PhysicsBody* body, Vec3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetLinearVelocity(body->id), velocity);
}

void alimerPhysicsBodySetLinearVelocity(PhysicsBody* body, const Vec3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetLinearVelocity(body->id, ToBox3D(velocity));
}

void alimerPhysicsBodyGetAngularVelocity(PhysicsBody* body, Vec3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetAngularVelocity(body->id), velocity);
}

void alimerPhysicsBodySetAngularVelocity(PhysicsBody* body, const Vec3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetAngularVelocity(body->id, ToBox3D(velocity));
}

void alimerPhysicsBodyAddForce(PhysicsBody* body, const Vec3* force)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyForceToCenter(body->id, ToBox3D(force), true);
}

void alimerPhysicsBodyAddForceAtPosition(PhysicsBody* body, const Vec3* force, const Vec3* position)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyForce(body->id, ToBox3D(force), ToBox3D(position), true);
}

void alimerPhysicsBodyAddTorque(PhysicsBody* body, const Vec3* torque)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyTorque(body->id, ToBox3D(torque), true);
}

void alimerPhysicsBodyAddForceAndTorque(PhysicsBody* body, const Vec3* force, const Vec3* torque)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyForceToCenter(body->id, ToBox3D(force), true);
    b3Body_ApplyTorque(body->id, ToBox3D(torque), true);
}

void alimerPhysicsBodyAddImpulse(PhysicsBody* body, const Vec3* impulse)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyLinearImpulseToCenter(body->id, ToBox3D(impulse), true);
}

void alimerPhysicsBodyAddImpulseAtPosition(PhysicsBody* body, const Vec3* impulse, const Vec3* position)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyLinearImpulse(body->id, ToBox3D(impulse), ToBox3D(position), true);
}

void alimerPhysicsBodyAddAngularImpulse(PhysicsBody* body, const Vec3* angularImpulse)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyAngularImpulse(body->id, ToBox3D(angularImpulse), true);
}

bool alimerPhysicsBodyApplyBuoyancyImpulse(PhysicsBody* body, const Vec3* surfacePosition, const Vec3* surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vec3* fluidVelocity, const Vec3* gravity, float deltaTime)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    // TODO:
    return false;
}
