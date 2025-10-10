// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_PHYSICS)
#include "alimer_internal.h"
#include "alimer_physics.h"
#include <Jolt/Core/Core.h>

JPH_SUPPRESS_WARNING_PUSH
JPH_SUPPRESS_WARNINGS
#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/CylinderShape.h"
#include "Jolt/Physics/Collision/Shape/MutableCompoundShape.h"
#include "Jolt/Physics/Collision/PhysicsMaterialSimple.h"

// STL includes
#include <cstdarg>

JPH_SUPPRESS_WARNING_POP

static void TraceImpl(const char* fmt, ...)
{
    // Format the message
    va_list list;
    va_start(list, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, list);
    va_end(list);

    alimerLogTrace(LogCategory_Physics, "%s", buffer);
}

#ifdef JPH_ENABLE_ASSERTS
//static JPH_AssertFailureFunc s_AssertFailureFunc = nullptr;

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
{
    // Print to the TTY
    alimerLogError(LogCategory_Physics, "%s:%s: (%s) %s", inFile, inLine, inExpression, (inMessage != nullptr ? inMessage : ""));

    // Breakpoint
    return true;
};

#endif // JPH_ENABLE_ASSERTS

namespace
{
    static_assert(sizeof(JPH::Mat44) == sizeof(Matrix4x4));

    constexpr JPH::EMotionType ToJolt(PhysicsBodyType value)
    {
        switch (value)
        {
            case PhysicsBodyType_Kinematic:
                return JPH::EMotionType::Kinematic;
            case PhysicsBodyType_Dynamic:
                return JPH::EMotionType::Dynamic;
            case PhysicsBodyType_Static:
            default:
                return JPH::EMotionType::Static;
        }
    }

    static void FromJolt(const JPH::Vec3& value, Vector3* result)
    {
        result->x = value.GetX();
        result->y = value.GetY();
        result->z = value.GetZ();
    }

    static void FromJolt(const JPH::Quat& quat, Quaternion* result)
    {
        result->x = quat.GetX();
        result->y = quat.GetY();
        result->z = quat.GetZ();
        result->w = quat.GetW();
    }

    [[maybe_unused]] static void FromJolt(const JPH::Mat44& value, Matrix4x4* result)
    {
        JPH::Mat44 temp = value.Transposed();
        memcpy(result, &temp, sizeof(Matrix4x4));
    }

    static JPH::Vec3 ToJolt(const Vector3* value)
    {
        return JPH::Vec3(value->x, value->y, value->z);
    }

    static JPH::Quat ToJolt(const Quaternion* value)
    {
        return JPH::Quat(value->x, value->y, value->z, value->w);
    }

    [[maybe_unused]] static JPH::Mat44 ToJolt(const Matrix4x4* value)
    {
        JPH::Mat44 result;
        memcpy(&result, value, sizeof(Matrix4x4));

        return result.Transposed();
    }
}

// Based on: https://github.com/jrouwe/JoltPhysics/blob/master/HelloWorld/HelloWorld.cpp
namespace Layers
{
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}

namespace BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr uint32_t NUM_LAYERS(2);
}

/// Class that determines if two object layers can collide
class JoltObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                //JPH_ASSERT(false);
                return false;
        }
    }
};

class JoltBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
{
private:
    JPH::BroadPhaseLayer objectToBroadPhase[Layers::NUM_LAYERS];

public:
    JoltBroadPhaseLayerInterface()
    {
        // Create a mapping table from object to broad phase layer
        objectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        objectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }

    uint32_t GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return objectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch ((uint8_t)inLayer)
        {
            case (uint8_t)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
            case (uint8_t)BroadPhaseLayers::MOVING:		return "MOVING";
            default:
                JPH_ASSERT(false);
                return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
};

/// Class that determines if an object layer can collide with a broadphase layer
class JoltObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    JoltObjectVsBroadPhaseLayerFilter() = default;

    bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }

        //return AlimerJoltCollisionFiltering::s_CollisionFilterConfig.IsCollisionEnabled(m_uiCollisionLayer, static_cast<ezUInt32>(inLayer) & 0xFF);
        return true;
    }
};

class JoltBodyActivationListener final : public JPH::BodyActivationListener
{
public:
    void OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData) override
    {
        //ALIMER_PROFILE_SCOPE();

        /* Body Activated */
    }

    void OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData) override
    {
        //ALIMER_PROFILE_SCOPE();

        /* Body Deactivated */
    }
};

class JoltContactListener final : public JPH::ContactListener
{
private:
    static void	GetFrictionAndRestitution(const JPH::Body& inBody, const JPH::SubShapeID& inSubShapeID, float& outFriction, float& outRestitution)
    {
        //ALIMER_PROFILE_SCOPE();

        // Get the material that corresponds to the sub shape ID
        const JPH::PhysicsMaterial* material = inBody.GetShape()->GetMaterial(inSubShapeID);
        if (material == JPH::PhysicsMaterial::sDefault)
        {
            outFriction = inBody.GetFriction();
            outRestitution = inBody.GetRestitution();
        }
        else
        {
            //const auto* phyMaterial = static_cast<const PhysicsMaterial3D*>(material);
            //outFriction = phyMaterial->Friction;
            //outRestitution = phyMaterial->Restitution;
        }
    }

    static void	OverrideContactSettings(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        //ALIMER_PROFILE_SCOPE();

        // Get the custom friction and restitution for both bodies
        float friction1, friction2, restitution1, restitution2;
        GetFrictionAndRestitution(inBody1, inManifold.mSubShapeID1, friction1, restitution1);
        GetFrictionAndRestitution(inBody2, inManifold.mSubShapeID2, friction2, restitution2);

        // Use the default formulas for combining friction and restitution
        ioSettings.mCombinedFriction = JPH::sqrt(friction1 * friction2);
        ioSettings.mCombinedRestitution = JPH::max(restitution1, restitution2);
    }

public:
    JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
    {
        //ALIMER_PROFILE_SCOPE();

        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
        // TODO: Triggers

        OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
    }

    void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
        //ALIMER_PROFILE_SCOPE();

        OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
    }

    void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
    {
        //ALIMER_PROFILE_SCOPE();

        /* On Collision Exit */
    }
};

class AlimerPhysicsMaterial : public JPH::PhysicsMaterialSimple
{
public:
    float friction;
    float restitution;

    AlimerPhysicsMaterial() = default;

    AlimerPhysicsMaterial(const std::string_view& name, JPH::ColorArg color, float friction_, float restitution_)
        : JPH::PhysicsMaterialSimple(name, color)
        , friction(friction_)
        , restitution(restitution_)
    {
    }
};

static struct
{
    bool initialized;
    JPH::TempAllocatorImplWithMallocFallback* tempAllocator;
    JPH::JobSystemThreadPool* jobSystem;
} physics_state = {};

struct PhysicsWorld final
{
    std::atomic_uint32_t                refCount;
    JoltObjectLayerPairFilter           objectLayerFilter;
    JoltBroadPhaseLayerInterface	    broadPhaseLayerInterface;
    JoltObjectVsBroadPhaseLayerFilter   objectVsBroadPhaseLayerFilter;
    JPH::PhysicsSystem                  system;
    JoltBodyActivationListener          bodyActivationListener;
    JoltContactListener                 contactListener;
    JPH::ShapeRefC                      emptyShape;
};

struct PhysicsMaterial final
{
    std::atomic_uint32_t refCount;
    JPH::Ref<AlimerPhysicsMaterial> handle;
};

struct PhysicsBody final
{
    std::atomic_uint32_t refCount;
    PhysicsWorld* world;
    JPH::Body* handle;
    JPH::BodyID id;
};

struct PhysicsShape final
{
    std::atomic_uint32_t refCount;
    PhysicsShapeType type;
    JPH::ShapeRefC handle;
    PhysicsBody* body;
    void* userdata = nullptr;
};

bool alimerPhysicsInit(const PhysicsConfig* config)
{
    ALIMER_ASSERT(config);

    if (physics_state.initialized)
        return true;

    JPH::RegisterDefaultAllocator();

    // TODO
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

        // Create a factory
        JPH::Factory::sInstance = new JPH::Factory();

    // Register all Jolt physics types
    JPH::RegisterTypes();

    const uint32_t tempAllocatorSize = config->tempAllocatorInitSize > 0 ? config->tempAllocatorInitSize : 8 * 1024 * 1024;
    const uint32_t maxPhysicsJobs = config->maxPhysicsJobs > 0 ? config->maxPhysicsJobs : JPH::cMaxPhysicsJobs;
    const uint32_t maxPhysicsBarriers = config->maxPhysicsBarriers > 0 ? config->maxPhysicsBarriers : JPH::cMaxPhysicsBarriers;

    // Init temp allocator
    physics_state.tempAllocator = new JPH::TempAllocatorImplWithMallocFallback(tempAllocatorSize);

    // Init Job system.
    physics_state.jobSystem = new JPH::JobSystemThreadPool(maxPhysicsJobs, maxPhysicsBarriers);

    physics_state.initialized = true;
    return true;
}

void alimerPhysicsShutdown(void)
{
    if (!physics_state.initialized)
        return;

    delete physics_state.jobSystem; physics_state.jobSystem = nullptr;
    delete physics_state.tempAllocator; physics_state.tempAllocator = nullptr;

    // Unregisters all types with the factory and cleans up the default material
    JPH::UnregisterTypes();

    // Destroy the factory
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    physics_state.initialized = false;
    memset(&physics_state, 0, sizeof(physics_state));
}

PhysicsWorld* alimerPhysicsWorldCreate(const PhysicsWorldConfig* config)
{
    ALIMER_ASSERT(config);

    PhysicsWorld* world = new PhysicsWorld();
    world->refCount.store(1);

    // Init the physics system
    const uint32_t maxBodies = config->maxBodies ? config->maxBodies : 65536;
    const uint32_t maxBodyPairs = config->maxBodyPairs ? config->maxBodyPairs : 65536;
    const uint32_t maxContactConstraints = maxBodies;

    world->system.Init(maxBodies, 0, maxBodyPairs, maxContactConstraints,
        world->broadPhaseLayerInterface,
        world->objectVsBroadPhaseLayerFilter,
        world->objectLayerFilter
    );
    world->system.SetBodyActivationListener(&world->bodyActivationListener);
    world->system.SetContactListener(&world->contactListener);

    JPH::EmptyShapeSettings settings(JPH::Vec3::sZero());
    settings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = settings.Create();
    if (!shapeResult.IsValid())
    {
        alimerLogError(LogCategory_Physics, "Jolt: CreateBox failed with %s", shapeResult.GetError().c_str());
        return nullptr;
    }

    world->emptyShape = shapeResult.Get();
    return world;
}

void alimerPhysicsWorldDestroy(PhysicsWorld* world)
{
    delete world;
}

uint32_t alimerPhysicsWorldGetBodyCount(PhysicsWorld* world)
{
    return world->system.GetNumBodies();
}

uint32_t alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld* world)
{
    return world->system.GetNumActiveBodies(JPH::EBodyType::RigidBody);
}

void alimerPhysicsWorldGetGravity(PhysicsWorld* world, Vector3* gravity)
{
    FromJolt(world->system.GetGravity(), gravity);
}

void alimerPhysicsWorldSetGravity(PhysicsWorld* world, const Vector3* gravity)
{
    world->system.SetGravity(ToJolt(gravity));
}

bool alimerPhysicsWorldUpdate(PhysicsWorld* world, float deltaTime, int collisionSteps)
{
    JPH::EPhysicsUpdateError error = world->system.Update(
        deltaTime,
        collisionSteps,
        physics_state.tempAllocator,
        physics_state.jobSystem
    );
    return error == JPH::EPhysicsUpdateError::None;
}

void alimerPhysicsWorldOptimizeBroadPhase(PhysicsWorld* world)
{
    world->system.OptimizeBroadPhase();
}

/* Material */
PhysicsMaterial* alimerPhysicsMaterialCreate(const char* name, float friction, float restitution)
{
    PhysicsMaterial* material = new PhysicsMaterial();
    material->refCount.store(1);
    material->handle = new AlimerPhysicsMaterial(name, JPH::ColorArg(255, 0, 0), friction, restitution);
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
    return shape && shape->handle != nullptr;
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
    return shape->userdata;
}

void alimerPhysicsShapeSetUserData(PhysicsShape* shape, void* userdata)
{
    // freeUserData?
    shape->userdata = userdata;
}

float alimerPhysicsShapeGetVolume(PhysicsShape* shape)
{
    return shape->handle->GetVolume();
}

float alimerPhysicsShapeGetDensity(PhysicsShape* shape)
{
    if (shape->type == PhysicsShapeType_Mesh || shape->type == PhysicsShapeType_Terrain)
    {
        return 0.f;
    }

    return JPH::StaticCast<JPH::ConvexShape>(shape->handle)->GetDensity();
}

float alimerPhysicsShapeGetMass(PhysicsShape* shape)
{
    if (shape->type == PhysicsShapeType_Mesh || shape->type == PhysicsShapeType_Terrain)
    {
        return 0.0f;
    }

    JPH::MassProperties properties = shape->handle->GetMassProperties();
    return properties.mMass;
}

PhysicsShape* alimerPhysicsShapeCreateBox(const Vector3* size, PhysicsMaterial* material)
{
    JPH_ASSERT(size);
    JPH_ASSERT(size->x > 0.f && size->y > 0.f && size->z > 0.f);

    const JPH::Vec3 halfExtent = { size->x / 2.f, size->y / 2.f, size->z / 2.f };
    float shortestSide = std::min(size->x, std::min(size->y, size->z));
    float convexRadius = std::min(shortestSide * .1f, .05f);
    JPH::BoxShapeSettings settings(halfExtent, convexRadius, material != nullptr ? material->handle : nullptr);
    settings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = settings.Create();
    if (!shapeResult.IsValid())
    {
        alimerLogError(LogCategory_Physics, "Physics: CreateBox failed with %s", shapeResult.GetError().c_str());
        return nullptr;
    }

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Box;
    shape->handle = shapeResult.Get();
    //shape->handle->SetUserData(reinterpret_cast<uint64_t>(shape));
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateSphere(float radius, PhysicsMaterial* material)
{
    JPH_ASSERT(radius > 0.f);

    JPH::SphereShapeSettings settings(radius, material != nullptr ? material->handle : nullptr);
    settings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = settings.Create();
    if (!shapeResult.IsValid())
    {
        alimerLogError(LogCategory_Physics, "Physics: CreateSphere failed with %s", shapeResult.GetError().c_str());
        return nullptr;
    }

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Sphere;
    shape->handle = shapeResult.Get();
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateCapsule(float height, float radius, PhysicsMaterial* material)
{
    JPH::CapsuleShapeSettings settings(
        std::max(0.01f, height) * 0.5f,
        std::max(0.01f, radius), material != nullptr ? material->handle : nullptr
    );
    settings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = settings.Create();
    if (!shapeResult.IsValid())
    {
        alimerLogError(LogCategory_Physics, "Physics: CreateCapsule failed with %s", shapeResult.GetError().c_str());
        return nullptr;
    }

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Capsule;
    shape->handle = shapeResult.Get();
    return shape;
}

PhysicsShape* alimerPhysicsShapeCreateCylinder(float height, float radius, PhysicsMaterial* material)
{
    JPH::CylinderShapeSettings settings(
        std::max(0.01f, height) * 0.5f,
        std::max(0.01f, radius),
        JPH::cDefaultConvexRadius,
        material != nullptr ? material->handle : nullptr
    );
    settings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = settings.Create();
    if (!shapeResult.IsValid())
    {
        //alimerLogError(LogCategory_Physics, "Jolt: CreateCylinder failed with %s", shapeResult.GetError().c_str());
        return nullptr;
    }

    PhysicsShape* shape = new PhysicsShape();
    shape->refCount.store(1);
    shape->type = PhysicsShapeType_Cylinder;
    shape->handle = shapeResult.Get();
    return shape;
}

/* Body */
void alimerPhysicsBodyDescInit(PhysicsBodyDesc* desc)
{
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
                alimerLogError(LogCategory_Physics, "PhysicsShape is already attached to another body");
                return nullptr;
            }
        }
    }

    const uint32_t count = world->system.GetNumBodies();
    const uint32_t limit = world->system.GetMaxBodies();
    if (count >= limit)
    {
        alimerLogError(LogCategory_Physics, "Too many bodies, limit %s reached!", limit);
        return nullptr;
    }

    JPH::BodyInterface& bodyInterface = world->system.GetBodyInterface();

    JPH::RVec3 position = ToJolt(&desc->initialTransform.position);
    JPH::Quat rotation = ToJolt(&desc->initialTransform.rotation);

    JPH::EMotionType motionType = ToJolt(desc->type); // shape->handle->MustBeStatic()
    JPH::ObjectLayer objectLayer = (desc->type == PhysicsBodyType_Static) ? Layers::NON_MOVING : Layers::MOVING;

    JPH::MutableCompoundShapeSettings compoundShapeSettings;
    JPH::BodyCreationSettings bodySettings;
    bodySettings.mPosition = position;
    bodySettings.mRotation = rotation;
    bodySettings.mObjectLayer = objectLayer;
    bodySettings.mMotionType = motionType;

    bool useCompoundShape = desc->shapeCount > 1;
    if (useCompoundShape)
    {
        for (uint32_t i = 0; i < desc->shapeCount; i++)
        {
            compoundShapeSettings.AddShape(JPH::Vec3::sZero(), JPH::Quat::sIdentity(), desc->shapes[i]->handle);
        }

        bodySettings.SetShapeSettings(&compoundShapeSettings);
    }
    else
    {
        bodySettings.SetShape((desc->shapeCount == 0) ? world->emptyShape : desc->shapes[0]->handle);
    }

    bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::All;
    // Allow dynamic bodies to become kinematic during, e.g., user interaction
    bodySettings.mAllowDynamicOrKinematic = (desc->type == PhysicsBodyType_Dynamic);
    bodySettings.mIsSensor = desc->isSensor;
    bodySettings.mLinearDamping = desc->linearDamping;
    bodySettings.mAngularDamping = desc->angularDamping;
    bodySettings.mMotionQuality = desc->continuous ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
    bodySettings.mGravityFactor = desc->gravityScale;
    if (desc->type != PhysicsBodyType_Static && (desc->mass != 0.0f))
    {
        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        bodySettings.mMassPropertiesOverride.mMass = desc->mass;
    }

    PhysicsBody* body = new PhysicsBody();
    body->refCount.store(1);
    body->world = world;
    body->handle = bodyInterface.CreateBody(bodySettings);
    body->id = body->handle->GetID();
    body->handle->SetUserData(reinterpret_cast<uint64_t>(body));

    // Add it to the world
    bodyInterface.AddBody(body->id, JPH::EActivation::Activate);

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
        JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterface();
        bool added = bodyInterface.IsAdded(body->id);
        if (added) {
            bodyInterface.RemoveBody(body->id);
        }

        bodyInterface.DestroyBody(body->id);
        body->handle = nullptr;
        body->id = {};
        body->world = nullptr;

        delete body;
    }
}

bool alimerPhysicsBodyIsValid(PhysicsBody* body)
{
    return body && body->handle != nullptr;
}

bool alimerPhysicsBodyIsActive(PhysicsBody* body)
{
    JPH_ASSERT(!body->id.IsInvalid());

    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterfaceNoLock();
    return bodyInterface.IsActive(body->id);
}

PhysicsWorld* alimerPhysicsBodyGetWorld(PhysicsBody* body)
{
    return body->world;
}

uint32_t alimerPhysicsBodyGetID(PhysicsBody* body)
{
    return body->id.GetIndexAndSequenceNumber();
}

void alimerPhysicsBodyGetTransform(PhysicsBody* body, PhysicsBodyTransform* transform)
{
    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterface();
    JPH::RVec3 position{};
    JPH::Quat rotation{};
    bodyInterface.GetPositionAndRotation(body->id, position, rotation);

    FromJolt(position, &transform->position);
    FromJolt(rotation, &transform->rotation);
}

void alimerPhysicsBodySetTransform(PhysicsBody* body, const PhysicsBodyTransform* transform)
{
    // Update the transform iff it has changed significantly
    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterface();
    JPH::RVec3 position = ToJolt(&transform->position);
    JPH::Quat rotation = ToJolt(&transform->rotation);

    bodyInterface.SetPositionAndRotationWhenChanged(body->id, position, rotation, JPH::EActivation::Activate);
}

void alimerPhysicsBodyGetWorldTransform(PhysicsBody* body, Matrix4x4* transform)
{
    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterface();

    JPH::RMat44 joltTransform = bodyInterface.GetWorldTransform(body->id);
    FromJolt(joltTransform, transform);
}

void alimerPhysicsBodyGetLinearVelocity(PhysicsBody* body, Vector3* velocity)
{
    JPH_ASSERT(!body->id.IsInvalid());

    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterfaceNoLock();
    FromJolt(bodyInterface.GetLinearVelocity(body->id), velocity);
}

void alimerPhysicsBodySetLinearVelocity(PhysicsBody* body, const Vector3* velocity)
{
    JPH_ASSERT(!body->id.IsInvalid());

    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterface();
    bodyInterface.SetLinearVelocity(body->id, ToJolt(velocity));
}

void alimerPhysicsBodyGetAngularVelocity(PhysicsBody* body, Vector3* velocity)
{
    JPH_ASSERT(!body->id.IsInvalid());

    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterfaceNoLock();
    FromJolt(bodyInterface.GetAngularVelocity(body->id), velocity);
}

void alimerPhysicsBodySetAngularVelocity(PhysicsBody* body, const Vector3* velocity)
{
    JPH_ASSERT(!body->id.IsInvalid());

    JPH::BodyInterface& bodyInterface = body->world->system.GetBodyInterface();
    bodyInterface.SetAngularVelocity(body->id, ToJolt(velocity));
}

#endif /* defined(ALIMER_PHYSICS) */
