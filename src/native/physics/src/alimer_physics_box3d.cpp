// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_physics_internal.h"
#include "box3d/box3d.h"

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

    static void FromBox3D(const b3Vec3& value, Vector3* result)
    {
        result->x = value.x;
        result->y = value.y;
        result->z = value.z;
    }

    static void FromBox3D(const b3Quat& quat, Quaternion* result)
    {
        result->x = quat.v.x;
        result->y = quat.v.y;
        result->z = quat.v.z;
        result->w = quat.s;
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

    static b3Vec3 ToBox3D(const Vector3* value)
    {
        return { value->x, value->y, value->z };
    }

    static b3Quat ToBox3D(const Quaternion* value)
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

void* physics_default_alloc(size_t size, void* userData)
{
    ALIMER_UNUSED(userData);
    return malloc(size);
}

void physics_default_free(void* ptr, void* userData)
{
    ALIMER_UNUSED(userData);
    free(ptr);
}

PhysicsAllocator g_physics_allocator = { physics_default_alloc, physics_default_free, nullptr };

void alimerPhysicsSetAllocationCallbacks(PhysicsAllocCallback alloc, PhysicsFreeCallback free, void* userData)
{
    g_physics_allocator.alloc = alloc ? alloc : physics_default_alloc;
    g_physics_allocator.free = free ? free : physics_default_free;
    g_physics_allocator.userData = userData;
}

static struct
{
    std::atomic_bool initialized;
} physics_state = {};

struct PhysicsWorld final
{
    std::atomic_uint32_t refCount;
    uint32_t maxBodies;
    b3WorldId id;
};

struct PhysicsMaterial final
{
    std::atomic_uint32_t refCount;
    char* name = nullptr;
    float friction = 0.4f;
    float restitution = 0.3f;
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
    Vector3 size;
    b3HullData* hullData = nullptr;
    b3MeshData* meshData = nullptr;
    PhysicsMaterial* material;
    PhysicsBody* body;
    b3ShapeId id;
};

bool alimerPhysicsInit(void)
{
    //ALIMER_ASSERT(config);

    if (physics_state.initialized.load())
        return true;

    //b3SetAllocator(&Box3DAllocate, &Box3DFree);

    physics_state.initialized.store(true);
    return true;
}

void alimerPhysicsShutdown(void)
{
    if (!physics_state.initialized.load())
        return;

    physics_state.initialized.store(false);
}

PhysicsWorldConfig alimerPhysicsWorldConfigDefault(void)
{
    PhysicsWorldConfig config = {};
    config.maxPhysicsBodies = 65536;
    config.maxPhysicsContactConstraints = 131072;
    config.gravity = { 0.0f, -10.0f, 0.0f };
    return config;
}

static PhysicsWorldConfig PhysicsWorldConfig_Defaults(const PhysicsWorldConfig* config)
{
    PhysicsWorldConfig configDef;
    if (config != NULL)
    {
        configDef = *config;
    }
    else
    {
        configDef = alimerPhysicsWorldConfigDefault();
    }

    configDef.maxPhysicsBodies = PHYSICS_DEF(configDef.maxPhysicsBodies, 65536);
    configDef.maxPhysicsContactConstraints = PHYSICS_DEF(configDef.maxPhysicsContactConstraints, 131072);
    return configDef;
}

PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* config)
{
    PhysicsWorldConfig configDef = PhysicsWorldConfig_Defaults(config);

    const uint32_t maxBodies = Max(configDef.maxPhysicsBodies, 1u);

    PhysicsWorld* world = PhysicsAlloc<PhysicsWorld>();
    world->refCount.store(1);
    world->maxBodies = maxBodies;

    // Init the physics system
    b3WorldDef worldDef = b3DefaultWorldDef();
    worldDef.gravity = ToBox3D(&configDef.gravity);
    worldDef.userData = world;

    // Pre-sizing avoids the world reallocating its body and contact arrays during a spawn burst.
    worldDef.capacity.dynamicBodyCount = (int)maxBodies;
    worldDef.capacity.staticBodyCount = (int)maxBodies;
    worldDef.capacity.dynamicShapeCount = (int)maxBodies;
    worldDef.capacity.staticShapeCount = (int)maxBodies;
    worldDef.capacity.contactCount = (int)configDef.maxPhysicsContactConstraints;

    world->id = b3CreateWorld(&worldDef);

    if (!b3World_IsValid(world->id))
    {
        //alimerLogError(LogCategory_Physics, "Box3D: CreateWorld failed");
        PhysicsFree(world);
        return nullptr;
    }

    return world;
}

void alimerPhysicsWorldDestroy(PhysicsWorld* world)
{
    uint32_t newCount = --world->refCount;
    if (newCount == 0)
    {
        if (b3World_IsValid(world->id))
        {
            b3DestroyWorld(world->id);
        }

        PhysicsFree(world);
    }
}

uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world)
{
    return world->maxBodies;
}

uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world)
{
    return b3World_GetAwakeBodyCount(world->id);
}

void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vector3* gravity)
{
    FromBox3D(b3World_GetGravity(world->id), gravity);
}

void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vector3* gravity)
{
    b3World_SetGravity(world->id, ToBox3D(gravity));
}

void alimerPhysicsWorldUpdate(PhysicsWorld* world, float timeStep, int collisionSteps)
{
    b3World_Step(world->id, timeStep, collisionSteps);
}

/* Material */
static PhysicsMaterialDesc PhysicsMaterialDesc_Defaults(const PhysicsMaterialDesc* desc)
{
    PhysicsMaterialDesc def = {};
    if (desc != nullptr)
        def = *desc;

    return def;
}


PhysicsMaterial* alimerPhysicsMaterialCreate(const PhysicsMaterialDesc* desc)
{
    PhysicsMaterialDesc descDef = PhysicsMaterialDesc_Defaults(desc);

    PhysicsMaterial* material = PhysicsAlloc<PhysicsMaterial>();
    material->refCount.store(1);
    if (descDef.name)
        material->name = physics_strdup(descDef.name);
    material->friction = descDef.friction;
    material->restitution = descDef.restitution;
    return material;
}

void alimerPhysicsMaterialAddRef(PhysicsMaterial* material)
{
    ++material->refCount;
}

void alimerPhysicsMaterialRelease(PhysicsMaterial* material)
{
    uint32_t newCount = --material->refCount;
    if (newCount == 0)
    {
        if (material->name)
            physics_free(material->name);
        PhysicsFree(material);
    }
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
        if (shape->hullData)
        {
            b3DestroyHull(shape->hullData);
            shape->hullData = nullptr;
        }

        if (shape->meshData)
        {
            b3DestroyMesh(shape->meshData);
            shape->meshData = nullptr;
        }

        if (shape->material)
        {
            alimerPhysicsMaterialRelease(shape->material);
        }

        PhysicsFree(shape);
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
    return b3Shape_GetDensity(shape->id);
}

PhysicsShape* alimerPhysicsShapeCreateBox(const Vector3* size, PhysicsMaterial* material)
{
    ALIMER_ASSERT(size);
    ALIMER_ASSERT(size->x > 0.f && size->y > 0.f && size->z > 0.f);

    PhysicsShape* shape = PhysicsAlloc<PhysicsShape>();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Box;
    shape->size.x = size->x;
    shape->size.y = size->y;
    shape->size.z = size->z;
    shape->material = material;
    if (material)
    {
        alimerPhysicsMaterialAddRef(material);
    }
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateSphere(float radius, PhysicsMaterial* material)
{
    ALIMER_ASSERT(radius > 0.f);

    PhysicsShape* shape = PhysicsAlloc<PhysicsShape>();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Sphere;
    shape->size.x = radius;
    shape->size.y = radius;
    shape->size.z = radius;
    shape->material = material;
    if (material)
    {
        alimerPhysicsMaterialAddRef(material);
    }
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateCapsule(float height, float radius, PhysicsMaterial* material)
{
    ALIMER_ASSERT(height > 0.f);
    ALIMER_ASSERT(radius > 0.f);

    PhysicsShape* shape = PhysicsAlloc<PhysicsShape>();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Capsule;
    shape->size.x = radius;
    shape->size.y = height;
    shape->size.z = 0.f;
    shape->material = material;
    if (material)
    {
        alimerPhysicsMaterialAddRef(material);
    }
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateCylinder(float height, float radius, PhysicsMaterial* material)
{
    ALIMER_ASSERT(height > 0.f);
    ALIMER_ASSERT(radius > 0.f);

    const float shapeScale = 1.0f;

    PhysicsShape* shape = PhysicsAlloc<PhysicsShape>();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Cylinder;
    shape->size.x = radius;
    shape->size.y = height;
    shape->size.z = 0.f;
    const float scaledRadius = radius * shapeScale;
    const float scaledHeight = height * shapeScale;
    shape->hullData = b3CreateCylinder(scaledHeight, scaledRadius, -(scaledHeight * 0.5f), 16);
    shape->material = material;
    if (material)
    {
        alimerPhysicsMaterialAddRef(material);
    }
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateConvexHull(const Vector3* points, uint32_t pointsCount, PhysicsMaterial* material)
{
    b3Vec3* boxPoints = (b3Vec3*)physics_malloc(sizeof(b3Vec3) * pointsCount);
    for (uint32_t i = 0; i < pointsCount; ++i)
    {
        boxPoints[i] = ToBox3D(&points[i]);
    }

    b3HullData* hull = b3CreateHull(boxPoints, (int)pointsCount, B3_MAX_HULL_VERTICES);
    if (hull == nullptr)
    {
        return nullptr;
    }
    physics_free(boxPoints);

    PhysicsShape* shape = PhysicsAlloc<PhysicsShape>();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_ConvexHull;
    shape->hullData = hull;
    shape->material = material;
    if (material)
    {
        alimerPhysicsMaterialAddRef(material);
    }

    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateMesh(const Vector3* vertices, uint32_t verticesCount, const uint32_t* indices, uint32_t indicesCount)
{
    b3Vec3* boxVertices = (b3Vec3*)physics_malloc(sizeof(b3Vec3) * verticesCount);
    int32_t* boxIndices = (int32_t*)physics_malloc(sizeof(int32_t) * indicesCount);

    for (uint32_t i = 0; i < verticesCount; ++i)
    {
        boxVertices[i] = ToBox3D(&vertices[i]);
    }

    for (uint32_t i = 0; i < indicesCount; ++i)
    {
        boxIndices[i] = (int32_t)indices[i];
    }

    b3MeshDef Def = b3MeshDef{};
    Def.vertices = boxVertices;
    Def.vertexCount = (int)verticesCount;
    Def.indices =boxIndices;
    Def.triangleCount = (int)(indicesCount / 3);
    Def.weldVertices = true;
    Def.identifyEdges = true;
    b3MeshData* meshData = b3CreateMesh(&Def, nullptr, 0);
    if (meshData == nullptr)
    {
        return nullptr;
    }
    physics_free(boxVertices);
    physics_free(boxIndices);

    PhysicsShape* shape = PhysicsAlloc<PhysicsShape>();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Mesh;
    shape->meshData = meshData;
    return shape;
}

/* Body */
PhysicsBodyDesc alimerPhysicsBodyDescDefault(void)
{
    PhysicsBodyDesc desc = {};
    desc.type = PhysicsBodyType_Dynamic;
    desc.position = { 0.0f, 0.0f, 0.0f };
    desc.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    desc.linearVelocity = { 0.0f, 0.0f, 0.0f };
    desc.angularVelocity = { 0.0f, 0.0f, 0.0f };
    desc.mass = 1.0f;
    desc.linearDamping = 0.05f;
    desc.angularDamping = 0.05f;
    desc.gravityScale = 1.0f;
    desc.isSensor = false;
    desc.allowSleeping = true;
    desc.useContinuousCollision = false;
    desc.shapeCount = 0;
    desc.shapes = nullptr;
    return desc;
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

    PhysicsBody* body = PhysicsAlloc<PhysicsBody>();
    body->refCount.store(1);
    body->world = world;

    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = ToBox3D(desc->type);
    bodyDef.position = ToBox3D(&desc->position);
    bodyDef.rotation = ToBox3D(&desc->rotation);
    bodyDef.linearVelocity = ToBox3D(&desc->linearVelocity);
    bodyDef.angularVelocity = ToBox3D(&desc->angularVelocity);
    bodyDef.linearDamping = desc->linearDamping;
    bodyDef.angularDamping = desc->angularDamping;
    bodyDef.gravityScale = desc->gravityScale;
    bodyDef.enableSleep = desc->allowSleeping;
    bodyDef.isBullet = desc->useContinuousCollision;
    bodyDef.userData = body;

    body->id = b3CreateBody(world->id, &bodyDef);

    for (uint32_t i = 0; i < desc->shapeCount; i++)
    {
        const float shapeScale = 1.0f;

        PhysicsShape* shape = desc->shapes[i];
        b3ShapeDef shapeDef = b3DefaultShapeDef();

        // TODO: Box3D requires density on b3ShapeDef, but alimer doesn't expose it
        // per-shape/material today. Hardcoding 1.0f keeps dynamic bodies simulating
        // correctly (Box3D warns a dynamic body needs >=1 shape with non-zero density)
        // but this should become a real parameter (e.g. on PhysicsMaterial).
        shapeDef.density = 1.0f;
        shapeDef.userData = shape;

        if (shape->material)
        {
            shapeDef.baseMaterial.friction = shape->material->friction;
            shapeDef.baseMaterial.restitution = shape->material->restitution;
        }

        switch (shape->type)
        {
            case PhysicsShapeType_Box:
            {
                b3BoxHull hull = b3MakeBoxHull(shape->size.x * 0.5f * shapeScale, shape->size.y * 0.5f * shapeScale, shape->size.z * 0.5f * shapeScale);
                shape->id = b3CreateHullShape(body->id, &shapeDef, &hull.base);
                break;
            }

            case PhysicsShapeType_Sphere:
            {
                const b3Sphere sphere = { b3Vec3_zero, shape->size.x * shapeScale };
                shape->id = b3CreateSphereShape(body->id, &shapeDef, &sphere);
                break;
            }

            case PhysicsShapeType_Capsule:
            {
                const float radius = shape->size.x;
                const float halfHeight = shape->size.y * 0.5f;

                const float scaledRadius = radius * shapeScale;
                const float scaledHalfHeight = halfHeight * shapeScale;
                b3Capsule capsule = { { 0.0f, 0.0f, -scaledHalfHeight }, { 0.0f, 0.0f, scaledHalfHeight }, scaledRadius };

                shape->id = b3CreateCapsuleShape(body->id, &shapeDef, &capsule);
                break;
            }

            case PhysicsShapeType_Cylinder:
            {
                shape->id = b3CreateHullShape(body->id, &shapeDef, shape->hullData);
                break;
            }

            case PhysicsShapeType_ConvexHull:
            {
                shape->id = b3CreateHullShape(body->id, &shapeDef, shape->hullData);
                break;
            }

            case PhysicsShapeType_Mesh:
            {
                shape->id = b3CreateMeshShape(body->id, &shapeDef, shape->meshData, {shapeScale, shapeScale, shapeScale});
                break;
            }

            default:
                //alimerLogError(LogCategory_Physics, "Failed to attach shape %u to body", i);
                b3DestroyBody(body->id);
                PhysicsFree(body);
                return nullptr;
        }

        // Assign body to the newly created shape, so it can be retrieved later via alimerPhysicsShapeGetBody().
        shape->body = body;
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
        if (b3Body_IsValid(body->id))
        {
            b3DestroyBody(body->id);
            body->id = {};
        }

        body->world = nullptr;
        PhysicsFree(body);
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

void alimerPhysicsBodyGetPosition(PhysicsBody* body, Vector3* position)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetPosition(body->id), position);
}

void alimerPhysicsBodyGetRotation(PhysicsBody* body, Quaternion* rotation)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetRotation(body->id), rotation);
}

void alimerPhysicsBodySetTransform(PhysicsBody* body, const Vector3* position, const Quaternion* rotation)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetTransform(body->id, ToBox3D(position), ToBox3D(rotation));
}

float alimerPhysicsBodyGetMass(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return b3Body_GetMass(body->id);
}

float alimerPhysicsBodyGetInverseMass(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return b3Body_GetInverseMass(body->id);
}

void alimerPhysicsBodyGetCenterOfMassPosition(PhysicsBody* body, Vector3* position)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetWorldCenter(body->id), position);
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

float alimerPhysicsBodyGetLinearDamping(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return b3Body_GetLinearDamping(body->id);
}

void alimerPhysicsBodySetLinearDamping(PhysicsBody* body, float value)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetLinearDamping(body->id, value);
}

float alimerPhysicsBodyGetAngularDamping(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return b3Body_GetAngularDamping(body->id);
}

void alimerPhysicsBodySetAngularDamping(PhysicsBody* body, float value)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetAngularDamping(body->id, value);
}

float alimerPhysicsBodyGetGravityScale(PhysicsBody* body)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    return b3Body_GetGravityScale(body->id);
}

void alimerPhysicsBodySetGravityScale(PhysicsBody* body, float value)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetGravityScale(body->id, value);
}

void alimerPhysicsBodyGetLinearVelocity(PhysicsBody* body, Vector3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetLinearVelocity(body->id), velocity);
}

void alimerPhysicsBodySetLinearVelocity(PhysicsBody* body, const Vector3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetLinearVelocity(body->id, ToBox3D(velocity));
}

void alimerPhysicsBodyGetAngularVelocity(PhysicsBody* body, Vector3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    FromBox3D(b3Body_GetAngularVelocity(body->id), velocity);
}

void alimerPhysicsBodySetAngularVelocity(PhysicsBody* body, const Vector3* velocity)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_SetAngularVelocity(body->id, ToBox3D(velocity));
}

void alimerPhysicsBodyAddForce(PhysicsBody* body, const Vector3* force)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyForceToCenter(body->id, ToBox3D(force), true);
}

void alimerPhysicsBodyAddForceAtPosition(PhysicsBody* body, const Vector3* force, const Vector3* position)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyForce(body->id, ToBox3D(force), ToBox3D(position), true);
}

void alimerPhysicsBodyAddTorque(PhysicsBody* body, const Vector3* torque)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyTorque(body->id, ToBox3D(torque), true);
}

void alimerPhysicsBodyAddImpulse(PhysicsBody* body, const Vector3* impulse)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyLinearImpulseToCenter(body->id, ToBox3D(impulse), true);
}

void alimerPhysicsBodyAddImpulseAtPosition(PhysicsBody* body, const Vector3* impulse, const Vector3* position)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyLinearImpulse(body->id, ToBox3D(impulse), ToBox3D(position), true);
}

void alimerPhysicsBodyAddAngularImpulse(PhysicsBody* body, const Vector3* angularImpulse)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    b3Body_ApplyAngularImpulse(body->id, ToBox3D(angularImpulse), true);
}

bool alimerPhysicsBodyApplyBuoyancyImpulse(PhysicsBody* body, const Vector3* surfacePosition, const Vector3* surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3* fluidVelocity, const Vector3* gravity, float deltaTime)
{
    ALIMER_ASSERT(b3Body_IsValid(body->id));

    // TODO:
    return false;
}
