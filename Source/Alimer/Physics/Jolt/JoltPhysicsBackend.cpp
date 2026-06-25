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
    : _impl(std::make_unique<Impl>())
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
{}

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

JPH::JobSystem& JoltPhysicsBackend::GetThreadPool()
{
    return *_threadPool;
}
