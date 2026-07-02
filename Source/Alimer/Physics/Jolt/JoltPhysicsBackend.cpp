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
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/CylinderShape.h"
#include "Jolt/Physics/Collision/Shape/HeightFieldShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/PlaneShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
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

    static_assert(sizeof(JPH::Mat44) == sizeof(Matrix4x4));

    constexpr JPH::EMotionType ToJolt(RigidBodyType value)
    {
        switch (value)
        {
            case RigidBodyType::Kinematic:
                return JPH::EMotionType::Kinematic;
            case RigidBodyType::Dynamic:
                return JPH::EMotionType::Dynamic;
            case RigidBodyType::Static:
            default:
                return JPH::EMotionType::Static;
        }
    }

    inline JPH::Vec3 ToJolt(const Vector3& v)
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

    static constexpr RigidBodyType FromJolt(JPH::EMotionType value)
    {
        switch (value)
        {
            case JPH::EMotionType::Kinematic:
                return RigidBodyType::Kinematic;
            case JPH::EMotionType::Dynamic:
                return RigidBodyType::Dynamic;

            case JPH::EMotionType::Static:
            default:
                return RigidBodyType::Static;
        }
    }

    inline Vector3 FromJolt(const JPH::Vec3& value)
    {
        return Vector3(value.GetX(), value.GetY(), value.GetZ());
    }

    inline Vector3 FromJolt(const JPH::Float3& value)
    {
        return Vector3(value.x, value.y, value.z);
    }

    inline Matrix4x4 FromJolt(const JPH::Mat44& value)
    {
        // Jolt is column-major, so we need to transpose the matrix
        JPH::Mat44 temp = value.Transposed();
        Matrix4x4 result;
        memcpy(&result, &temp, sizeof(Matrix4x4));
        return result;
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
    JoltRigidBody(JPH::BodyID bodyID, JPH::PhysicsSystem* system);
    ~JoltRigidBody() override;

    uint32_t GetID() const override;

    RigidBodyType GetType() const override;
    void SetType(RigidBodyType type) override;

    bool IsActive() const override;
    void Activate() override;
    void Deactivate()  override;

    Vector3 GetPosition() const override;
    Quaternion GetRotation() const override;
    RigidBodyTransform GetTransform() const override;
    void SetTransform(const RigidBodyTransform& transform) override;
    Matrix4x4 GetWorldTransform() const override;

    Vector3 GetCenterOfMass() const override;

    Vector3 GetLinearVelocity() const override;
    void SetLinearVelocity(const Vector3& velocity) override;
    Vector3 GetAngularVelocity() const override;
    void SetAngularVelocity(const Vector3& velocity) override;

    void AddForce(const Vector3& force) override;
    void AddForce(const Vector3& force, const Vector3& position) override;
    void AddTorque(const Vector3& torque) override;
    void AddForceAndTorque(const Vector3& force, const Vector3& torque) override;

    void AddImpulse(const Vector3& impulse) override;
    void AddImpulse(const Vector3& impulse, const Vector3& position) override;
    void AddAngularImpulse(const Vector3& angularImpulse) override;

    bool ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3& fluidVelocity, float deltaTime) override;

private:
    JPH::BodyID _bodyID;
    JPH::PhysicsSystem* _system;
};

/* JoltPhysicsWorld */
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
            //const auto* engineMaterial = static_cast<const AlimerPhysicsMaterial*>(material);
            //outFriction = engineMaterial->friction;
            //outRestitution = engineMaterial->restitution;
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
        ioSettings.mCombinedRestitution = std::max(restitution1, restitution2);
    }

public:
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

class JoltPhysicsWorld final : public PhysicsWorld
{
public:
    JoltPhysicsWorld();
    ~JoltPhysicsWorld() override;

    Vector3 GetGravity() const override;
    void SetGravity(const Vector3& gravity) override;

    uint32_t GetBodyCount() const override;
    uint32_t GetActiveBodyCount() const override;

    RigidBodyRef CreateRigidBody(const RigidBodyDesc& desc) override;

    void OptimizeBroadPhase() override;

    [[nodiscard]] JPH::PhysicsSystem& GetPhysicsSystem();
    [[nodiscard]] JPH::BodyInterface& GetBodyInterface();
    [[nodiscard]] JPH::BodyInterface& GetBodyInterfaceNoLock();

private:
    JPH::TempAllocatorMalloc* _tempAllocator = nullptr;
    JPH::PhysicsSystem  _system;
    JPH::BodyInterface* _bodyInterface = nullptr;
    JPH::BodyInterface* _bodyInterfaceNoLock = nullptr;
    JoltBodyActivationListener  _bodyActivationListener;
    JoltContactListener _contactListener;
    JPH::ShapeRefC _emptyShape;
    bool _debugDrawEnabled = false;
};

/* JoltRigidBody */
JoltRigidBody::JoltRigidBody(JPH::BodyID bodyID, JPH::PhysicsSystem* system)
    : _bodyID(bodyID)
    , _system(system)
{
}

JoltRigidBody::~JoltRigidBody()
{
    if (_bodyID.IsInvalid())
        return;

    JPH::BodyInterface& bodyInterface = _system->GetBodyInterface();
    bool added = bodyInterface.IsAdded(_bodyID);
    if (added)
    {
        bodyInterface.RemoveBody(_bodyID);
    }

    bodyInterface.DestroyBody(_bodyID);
    _bodyID = JPH::BodyID{ JPH::BodyID::cInvalidBodyID };
}

uint32_t JoltRigidBody::GetID() const
{
    return _bodyID.GetIndexAndSequenceNumber();
}


RigidBodyType JoltRigidBody::GetType() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    return FromJolt(_system->GetBodyInterface().GetMotionType(_bodyID));
}

void JoltRigidBody::SetType(RigidBodyType type)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().SetMotionType(_bodyID, ToJolt(type), JPH::EActivation::Activate);
}

bool JoltRigidBody::IsActive() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    return _system->GetBodyInterface().IsActive(_bodyID);
}

void JoltRigidBody::Activate()
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().ActivateBody(_bodyID);
}

void JoltRigidBody::Deactivate()
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().DeactivateBody(_bodyID);
}

Vector3 JoltRigidBody::GetPosition() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    return FromJolt(_system->GetBodyInterface().GetPosition(_bodyID));
}

Quaternion JoltRigidBody::GetRotation() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    return FromJolt(_system->GetBodyInterface().GetRotation(_bodyID));
}

RigidBodyTransform JoltRigidBody::GetTransform() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    JPH::RVec3 position{};
    JPH::Quat rotation{};
    _system->GetBodyInterface().GetPositionAndRotation(_bodyID, position, rotation);

    RigidBodyTransform transform{};
    transform.position = FromJolt(position);
    transform.rotation = FromJolt(rotation);
    return transform;
}

void JoltRigidBody::SetTransform(const RigidBodyTransform& transform)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    // Update the transform iff it has changed significantly
    JPH::RVec3 position = ToJolt(transform.position);
    JPH::Quat rotation = ToJolt(transform.rotation);

    _system->GetBodyInterface().SetPositionAndRotationWhenChanged(_bodyID, position, rotation, JPH::EActivation::Activate);
}

Matrix4x4 JoltRigidBody::GetWorldTransform() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    JPH::RMat44 joltTransform = _system->GetBodyInterface().GetWorldTransform(_bodyID);
    Matrix4x4 transform = FromJolt(joltTransform);
    return transform;
}

Vector3 JoltRigidBody::GetCenterOfMass() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    return FromJolt(_system->GetBodyInterface().GetCenterOfMassPosition(_bodyID));
}

Vector3 JoltRigidBody::GetLinearVelocity() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    JPH::Vec3 velocity = _system->GetBodyInterface().GetLinearVelocity(_bodyID);
    return FromJolt(velocity);
}

void JoltRigidBody::SetLinearVelocity(const Vector3& velocity)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().SetLinearVelocity(_bodyID, ToJolt(velocity));
}

Vector3 JoltRigidBody::GetAngularVelocity() const
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    JPH::Vec3 velocity = _system->GetBodyInterface().GetAngularVelocity(_bodyID);
    return FromJolt(velocity);
}

void JoltRigidBody::SetAngularVelocity(const Vector3& velocity)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().SetAngularVelocity(_bodyID, ToJolt(velocity));
}

void JoltRigidBody::AddForce(const Vector3& force)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddForce(_bodyID, ToJolt(force));
}

void JoltRigidBody::AddForce(const Vector3& force, const Vector3& position)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddForce(_bodyID, ToJolt(force), ToJolt(position));
}

void JoltRigidBody::AddTorque(const Vector3& torque)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddTorque(_bodyID, ToJolt(torque));
}

void JoltRigidBody::AddForceAndTorque(const Vector3& force, const Vector3& torque)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddForceAndTorque(_bodyID, ToJolt(force), ToJolt(torque));
}

void JoltRigidBody::AddImpulse(const Vector3& impulse)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddImpulse(_bodyID, ToJolt(impulse));
}

void JoltRigidBody::AddImpulse(const Vector3& impulse, const Vector3& position)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddImpulse(_bodyID, ToJolt(impulse), ToJolt(position));
}
void JoltRigidBody::AddAngularImpulse(const Vector3& angularImpulse)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    _system->GetBodyInterface().AddAngularImpulse(_bodyID, ToJolt(angularImpulse));
}

bool JoltRigidBody::ApplyBuoyancyImpulse(const Vector3& surfacePosition, const Vector3& surfaceNormal, float buoyancy, float linearDrag, float angularDrag, const Vector3& fluidVelocity, float deltaTime)
{
    JPH_ASSERT(!_bodyID.IsInvalid());

    return _system->GetBodyInterface().ApplyBuoyancyImpulse(
        _bodyID,
        ToJolt(surfacePosition),
        ToJolt(surfaceNormal),
        buoyancy,
        linearDrag,
        angularDrag,
        ToJolt(fluidVelocity),
        _system->GetGravity(),
        deltaTime
    );

}

JoltPhysicsWorld::JoltPhysicsWorld()
{
    _tempAllocator = new JPH::TempAllocatorMalloc();

    // Init the physics system
    const uint32_t maxBodies = /*config.maxBodies ? config.maxBodies :*/ 65536;
    const uint32_t numBodyMutexes = 0;
    const uint32_t maxBodyPairs = /*config.maxBodyPairs ? config.maxBodyPairs :*/ 65536;
    const uint32_t maxContactConstraints = maxBodies;

    _system.Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, s_BPLayerInterface, s_ObjVsBP, s_ObjPairFilter);
    _system.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
    _system.SetBodyActivationListener(&_bodyActivationListener);
    _system.SetContactListener(&_contactListener);

    JPH::EmptyShapeSettings settings(JPH::Vec3::sZero());
    JPH::ShapeSettings::ShapeResult shapeResult = settings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateBox failed with {}", shapeResult.GetError().c_str());
        return;
    }

    _emptyShape = shapeResult.Get();
    _bodyInterface = &_system.GetBodyInterface();
    _bodyInterfaceNoLock = &_system.GetBodyInterfaceNoLock();
}

JoltPhysicsWorld::~JoltPhysicsWorld()
{}

JPH::PhysicsSystem& JoltPhysicsWorld::GetPhysicsSystem()
{
    return _system;
}

JPH::BodyInterface& JoltPhysicsWorld::GetBodyInterface()
{
    return *_bodyInterface;
}

JPH::BodyInterface& JoltPhysicsWorld::GetBodyInterfaceNoLock()
{
    return *_bodyInterfaceNoLock;
}

Vector3 JoltPhysicsWorld::GetGravity() const
{
    return FromJolt(_system.GetGravity());
}

void JoltPhysicsWorld::SetGravity(const Vector3& gravity)
{
    _system.SetGravity(ToJolt(gravity));
}

uint32_t JoltPhysicsWorld::GetBodyCount() const
{
    return _system.GetNumBodies();
}

uint32_t JoltPhysicsWorld::GetActiveBodyCount() const
{
    return _system.GetNumActiveBodies(JPH::EBodyType::RigidBody);
}

RigidBodyRef JoltPhysicsWorld::CreateRigidBody(const RigidBodyDesc& desc)
{
    const uint32_t count = _system.GetNumBodies();
    const uint32_t limit = _system.GetMaxBodies();
    if (count >= limit)
    {
        LOGE("Too many bodies, limit {} reached!", limit);
        return nullptr;
    }

    JPH::BodyCreationSettings bodySettings{};
    bodySettings.mPosition = ToJoltRVec3(desc.initialTransform.position);
    bodySettings.mRotation = ToJolt(desc.initialTransform.rotation);

    switch (desc.type)
    {
        case RigidBodyType::Static:
            bodySettings.mMotionType = JPH::EMotionType::Static;
            break;
        case RigidBodyType::Kinematic:
            bodySettings.mMotionType = JPH::EMotionType::Kinematic;
            break;
        case RigidBodyType::Dynamic:
            bodySettings.mMotionType = JPH::EMotionType::Dynamic;
            break;
    }
    bodySettings.mObjectLayer = desc.isTrigger ? PhysicsLayers::TRIGGER : (desc.type == RigidBodyType::Static ? PhysicsLayers::NON_MOVING : PhysicsLayers::MOVING);

    JPH::MutableCompoundShapeSettings compoundShapeSettings;
    bool useCompoundShape = desc.shapes.size() > 1;
    if (useCompoundShape)
    {
        for (size_t i = 0; i < desc.shapes.size(); i++)
        {
            JoltCollisionShape* shape = static_cast<JoltCollisionShape*>(desc.shapes[i]);
            compoundShapeSettings.AddShape(JPH::Vec3::sZero(), JPH::Quat::sIdentity(), shape->handle);
        }

        bodySettings.SetShapeSettings(&compoundShapeSettings);
    }
    else
    {
        bodySettings.SetShape((desc.shapes.size() == 0) ? _emptyShape : static_cast<JoltCollisionShape*>(desc.shapes[0])->handle);
    }

    bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::All;
    // Allow dynamic bodies to become kinematic during, e.g., user interaction
    bodySettings.mAllowDynamicOrKinematic = (desc.type == RigidBodyType::Dynamic);
    bodySettings.mIsSensor = desc.isTrigger;
    //bodySettings.mFriction = desc.friction;
    //bodySettings.mRestitution = desc.restitution;
    bodySettings.mLinearDamping = desc.linearDamping;
    bodySettings.mAngularDamping = desc.angularDamping;
    bodySettings.mGravityFactor = desc.gravityScale;
    bodySettings.mAllowSleeping = desc.allowSleeping;
    bodySettings.mMotionQuality = desc.continuous ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
    if (desc.type == RigidBodyType::Dynamic && (desc.mass != 0.0f))
    {
        bodySettings.mMassPropertiesOverride.mMass = desc.mass;
        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    }
    JPH::Body* body = _bodyInterface->CreateBody(bodySettings);
    _bodyInterface->AddBody(body->GetID(), JPH::EActivation::Activate);

    //_physicsSystem.OptimizeBroadPhase();
    SharedPtr<JoltRigidBody> rigidBody = MakeShared<JoltRigidBody>(body->GetID(), &_system);
    _bodyInterface->SetUserData(body->GetID(), reinterpret_cast<uint64_t>(rigidBody.Get()));

    return rigidBody;
}

void JoltPhysicsWorld::OptimizeBroadPhase()
{
    _system.OptimizeBroadPhase();
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
    JPH::BoxShapeSettings shapeSettings(ToJolt(size * 0.5f));
    shapeSettings.SetEmbedded();
    //shapeSettings.SetDensity(glm::max(0.001f, bc.Density));
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateBoxShape failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Box;
    shapeResult.Get()->SetUserData(reinterpret_cast<JPH::uint64>(shape));
    return shape;
}

CollisionShape* JoltPhysicsBackend::CreateSphereShape(float radius) const
{
    JPH::SphereShapeSettings shapeSettings(radius);
    shapeSettings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateSphereShape failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Sphere;
    shapeResult.Get()->SetUserData(reinterpret_cast<JPH::uint64>(shape));
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
        LOGE("Jolt: CreateCapsuleShape failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Capsule;
    shapeResult.Get()->SetUserData((JPH::uint64)shape);
    return shape;
}

CollisionShape* JoltPhysicsBackend::CreateCylinderShape(float height, float radius) const
{
    JPH::CylinderShapeSettings shapeSettings(
        std::max(0.01f, height) * 0.5f,
        std::max(0.01f, radius),
        JPH::cDefaultConvexRadius,
        /*material != nullptr ? material->handle : */ nullptr
    );
    shapeSettings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateCylinderShape failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Cylinder;
    shapeResult.Get()->SetUserData((JPH::uint64)shape);
    return shape;
}

CollisionShape* JoltPhysicsBackend::CreatePlaneShape(float halfExtent) const
{
    JPH::PlaneShapeSettings  shapeSettings(
        JPH::Plane::sFromPointAndNormal(JPH::Vec3::sZero(), JPH::Vec3::sAxisY()),
        /*material != nullptr ? material->handle : */ nullptr,
        Alimer::Max(halfExtent, 1.0f)
    );
    shapeSettings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreatePlaneShape failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::Plane;
    shapeResult.Get()->SetUserData((JPH::uint64)shape);
    return shape;

}

CollisionShape* JoltPhysicsBackend::CreateConvexHullShape(const Vector3* points, uint32_t pointsCount) const
{
    JPH::Array<JPH::Vec3> joltPoints;
    joltPoints.reserve(pointsCount);

    for (uint32_t i = 0; i < pointsCount; i++)
    {
        joltPoints.push_back(ToJolt(points[i]));
    }

    JPH::ConvexHullShapeSettings shapeSettings(
        joltPoints,
        JPH::cDefaultConvexRadius,
        /*material != nullptr ? material->handle : */ nullptr
    );
    shapeSettings.SetEmbedded();
    JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
    if (!shapeResult.IsValid())
    {
        LOGE("Jolt: CreateConvexHull failed with {}", shapeResult.GetError());
        return nullptr;
    }

    JoltCollisionShape* shape = new JoltCollisionShape();
    shape->handle = shapeResult.Get();
    shape->type = CollisionShapeType::ConvexHull;
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
