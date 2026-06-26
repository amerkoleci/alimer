// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Physics/Jolt/JoltPhysicsBackend.h"
#include "Alimer/Core/Log.h"

// Jolt includes
#include "Jolt/Core/Core.h"

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
#include "Jolt/Physics/Collision/Shape/PlaneShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/CylinderShape.h"
#include "Jolt/Physics/Collision/Shape/HeightFieldShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/MutableCompoundShape.h"

#ifdef JPH_DEBUG_RENDERER
//#include "Alimer/Renderer/DebugRenderer.h"
#include "Jolt/Renderer/DebugRendererSimple.h"
#endif // JPH_DEBUG_RENDERER
JPH_SUPPRESS_WARNING_POP

#include <thread>
#include <cstdarg>

using namespace Alimer;

namespace
{
    static void JoltTraceFunc(const char* fmt, ...)
    {
        // Format the message
        va_list list;
        va_start(list, fmt);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, list);
        va_end(list);

        // Print to the TTY
        LOGV("Jolt: {}", buffer);
    }

#if defined(JPH_ENABLE_ASSERTS) && ALIMER_ENABLE_ASSERT

    // Callback for asserts, connect this to your own assert handler if you have one
    static bool JoltAssertFailed(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
    {
        // Print to the TTY
        Assert::GetHandler()(inExpression, inMessage, inFile, inLine);
        //cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << endl;

        // Breakpoint
        return true;
    }
#endif /* defined(JPH_ENABLE_ASSERTS) && ALIMER_ENABLE_ASSERT */

    inline JPH::Vec3 ToJoltVec3(const Vector3& v)
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    inline JPH::RVec3 ToJoltRVec3(const Vector3& v)
    {
        return JPH::RVec3(v.x, v.y, v.z);
    }

    inline JPH::Vec4 ToJolt(const Vector4& vec)
    {
        return JPH::Vec4(vec.x, vec.y, vec.z, vec.w);
    }

    inline JPH::Quat ToJolt(const Quaternion& quaternion)
    {
        return JPH::Quat(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
    }

    inline Vector3 FromJolt(const JPH::Vec3& value)
    {
        return Vector3(value.GetX(), value.GetY(), value.GetZ());
    }

    inline Vector3 FromJolt(const JPH::Float3& value)
    {
        return Vector3(value.x, value.y, value.z);
    }

#ifdef JPH_DOUBLE_PRECISION
    inline Vector3 FromJolt(const JPH::RVec3& value)
    {
        return Vector3(static_cast<float>(value.GetX()), static_cast<float>(value.GetY()), static_cast<float>(value.GetZ()));
    }
#endif

    inline Quaternion FromJolt(const JPH::Quat& value)
    {
        return Quaternion(value.GetX(), value.GetY(), value.GetZ(), value.GetW());
    }

    inline BoundingBox FromJolt(const JPH::AABox& aabb)
    {
        return BoundingBox(FromJolt(aabb.mMin), FromJolt(aabb.mMax));
    }

    // -----------------------------------------------------------------------
    // Broad-phase layer interface
    // -----------------------------------------------------------------------
    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint32_t NUM_LAYERS(2);
    };

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            _objectToBroadPhase[PhysicsLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            _objectToBroadPhase[PhysicsLayers::MOVING] = BroadPhaseLayers::MOVING;
            _objectToBroadPhase[PhysicsLayers::CHARACTER] = BroadPhaseLayers::MOVING;
            _objectToBroadPhase[PhysicsLayers::CHARACTER_GHOST] = BroadPhaseLayers::MOVING;
            _objectToBroadPhase[PhysicsLayers::TRIGGER] = BroadPhaseLayers::MOVING;
        }

        JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < PhysicsLayers::NUM_LAYERS);
            return _objectToBroadPhase[inLayer];
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

    private:
        JPH::BroadPhaseLayer _objectToBroadPhase[PhysicsLayers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
                case PhysicsLayers::NON_MOVING:      return inLayer2 == BroadPhaseLayers::MOVING;
                case PhysicsLayers::MOVING:          return true;
                case PhysicsLayers::CHARACTER:       return true;
                case PhysicsLayers::CHARACTER_GHOST: return true;
                case PhysicsLayers::TRIGGER:         return inLayer2 == BroadPhaseLayers::MOVING;
                default:                             return false;
            }
        }
    };

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
                case PhysicsLayers::TRIGGER:
                    return inObject2 == PhysicsLayers::CHARACTER_GHOST ||
                        inObject2 == PhysicsLayers::CHARACTER ||
                        inObject2 == PhysicsLayers::MOVING;
                case PhysicsLayers::NON_MOVING:
                    return inObject2 == PhysicsLayers::MOVING ||
                        inObject2 == PhysicsLayers::CHARACTER_GHOST;
                case PhysicsLayers::MOVING:
                    return inObject2 == PhysicsLayers::NON_MOVING ||
                        inObject2 == PhysicsLayers::MOVING ||
                        inObject2 == PhysicsLayers::TRIGGER;
                case PhysicsLayers::CHARACTER_GHOST:
                    return inObject2 == PhysicsLayers::TRIGGER;
                case PhysicsLayers::CHARACTER:
                    return inObject2 != PhysicsLayers::CHARACTER_GHOST &&
                        inObject2 != PhysicsLayers::CHARACTER;
                default:
                    return false;
            }
        }
    };

#ifdef JPH_DEBUG_RENDERER
    class JoltDebugRenderer final : public JPH::DebugRendererSimple
    {
    public:
        JoltDebugRenderer()
        {
            Initialize();
        }

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
        {
            // TODO
        }

        // DebugRendererSimple already provides DrawTriangle as 3 line calls,
        // and DrawText3D is a no-op below.
        void DrawText3D(JPH::RVec3Arg, const std::string_view&, JPH::ColorArg, float) override {}
    };
#endif
}

// Static filter objects (one per process)
static BPLayerInterfaceImpl              s_BPLayerInterface;
static ObjectVsBroadPhaseLayerFilterImpl s_ObjVsBP;
static ObjectLayerPairFilterImpl         s_ObjPairFilter;


class JoltCollisionShape final : public CollisionShape
{
public:
    JPH::ShapeRefC handle;
    CollisionShapeType type = CollisionShapeType::Count;

    CollisionShapeType GetType() const override
    {
        return type;
    }

    float GetVolume() const override
    {
        return handle->GetVolume();
    }

    float GetDensity() const override
    {
        if (type == CollisionShapeType::Mesh || type == CollisionShapeType::Terrain)
        {
            return 0.f;
        }

        return JPH::StaticCast<JPH::ConvexShape>(handle)->GetDensity();
    }

    float GetMass() const override
    {
        if (type == CollisionShapeType::Mesh || type == CollisionShapeType::Terrain)
        {
            return 0.0f;
        }

        JPH::MassProperties properties = handle->GetMassProperties();
        return properties.mMass;
    }

    BoundingBox GetLocalBounds() const override
    {
        return FromJolt(handle->GetLocalBounds());
    }

    Vector3 GetCenterOfMass() const override
    {
        return FromJolt(handle->GetCenterOfMass());
    }
};

class JoltRigidBody final : public RigidBody
{
public:
};

/* JoltPhysicsWorld */
struct JoltPhysicsWorld::Impl
{
    JPH::TempAllocatorMalloc* tempAllocator = nullptr;
    JPH::PhysicsSystem system;
    JPH::BodyInterface* bodyInterface = nullptr;
    JPH::BodyInterface* bodyInterfaceNoLock = nullptr;
    bool debugDrawEnabled = false;
};

JoltPhysicsWorld::JoltPhysicsWorld()
    : _impl(new Impl())
{
    _impl->tempAllocator = new JPH::TempAllocatorMalloc();

    // Init the physics system
    const uint32_t maxBodies = /*config.maxBodies ? config.maxBodies :*/ 65536;
    const uint32_t numBodyMutexes = 0;
    const uint32_t maxBodyPairs = /*config.maxBodyPairs ? config.maxBodyPairs :*/ 65536;
    const uint32_t maxContactConstraints = maxBodies;

    _impl->system.Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, s_BPLayerInterface, s_ObjVsBP, s_ObjPairFilter);
    _impl->system.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
    _impl->bodyInterface = &_impl->system.GetBodyInterface();
    _impl->bodyInterfaceNoLock = &_impl->system.GetBodyInterfaceNoLock();
}

JoltPhysicsWorld::~JoltPhysicsWorld()
{
    SafeDelete(_impl);
}

JPH::PhysicsSystem& JoltPhysicsWorld::GetPhysicsSystem()
{
    return _impl->system;
}

JPH::BodyInterface& JoltPhysicsWorld::GetBodyInterface()
{
    return *_impl->bodyInterface;
}

JPH::BodyInterface& JoltPhysicsWorld::GetBodyInterfaceNoLock()
{
    return *_impl->bodyInterfaceNoLock;
}

RigidBodyRef JoltPhysicsWorld::CreateRigidBody()
{
    SharedPtr<JoltRigidBody> rigidBody = MakeShared<JoltRigidBody>();
    return rigidBody;
}

/* JoltPhysicsBackend */
JoltPhysicsBackend::JoltPhysicsBackend()
{
    // Register allocation hook
    JPH::RegisterDefaultAllocator();

    // Install callbacks
    JPH::Trace = JoltTraceFunc;
#if defined(JPH_ENABLE_ASSERTS) && ALIMER_ENABLE_ASSERT
    JPH::AssertFailed = JoltAssertFailed;
#endif

    // Create a factory and register all Jolt physics types
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();


#ifdef JPH_DEBUG_RENDERER
    static JoltDebugRenderer s_DebugRenderer;
    JPH::DebugRenderer::sInstance = &s_DebugRenderer;
#endif

    // Create a thread pool for Jolt physics jobs
    const int availableThreads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
    _threadPool = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, availableThreads);
}

JoltPhysicsBackend::~JoltPhysicsBackend()
{
    delete _threadPool;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

PhysicsWorldRef JoltPhysicsBackend::CreatePhysicsWorld()
{
    return MakeShared<JoltPhysicsWorld>();
}

CollisionShape* JoltPhysicsBackend::CreateBoxShape(const Vector3& size) const
{
    JPH::BoxShapeSettings shapeSettings(ToJoltVec3(size * 0.5f));
    shapeSettings.SetEmbedded();
    //shapeSettings.SetDensity(glm::max(0.001f, bc.Density));
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateBox failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Box;
    shapeResult.Get()->SetUserData((JPH::uint64)shape);
    return shape;
}

CollisionShape* JoltPhysicsBackend::CreateSphereShape(float radius) const
{
    JPH::SphereShapeSettings shapeSettings(radius);
    shapeSettings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateSphere failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Sphere;
    shapeResult.Get()->SetUserData((JPH::uint64)shape);
    return shape;
}

CollisionShape* JoltPhysicsBackend::CreateCapsuleShape(float height, float radius) const
{
    JPH::CapsuleShapeSettings shapeSettings(
        std::max(0.01f, height) * 0.5f,
        std::max(0.01f, radius),
        /*material != nullptr ? material->handle : */ nullptr
    );
    shapeSettings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateCapsule failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Capsule;
    shapeResult.Get()->SetUserData((JPH::uint64)shape);
    return shape;
}

void JoltPhysicsBackend::DestroyShape(CollisionShape* shape)
{
    if (shape)
    {
        JoltCollisionShape* joltShape = static_cast<JoltCollisionShape*>(shape);
        delete joltShape;
    }
}

JPH::JobSystem& JoltPhysicsBackend::GetThreadPool()
{
    return *_threadPool;
}
