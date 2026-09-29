// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using Alimer.Numerics;

using Plane = Alimer.Numerics.Plane;
#if !BOX3D_DOUBLE_PRECISION
using b3Pos = System.Numerics.Vector3;
#endif

using unsafe b3FrictionCallback = delegate* unmanaged<float /*frictionA*/, ulong /*userMaterialIdA*/, float /*frictionB*/, ulong /*userMaterialIdB*/, float>;
using unsafe b3RestitutionCallback = delegate* unmanaged<float /*restitutionA*/, ulong /*userMaterialIdA*/, float /*restitutionB*/, ulong /*userMaterialIdB*/, float>;
using unsafe b3TaskCallback = delegate* unmanaged<void* /*taskContext*/, void>;
using unsafe b3EnqueueTaskCallback = delegate* unmanaged</*b3TaskCallback* task*/delegate* unmanaged<void* /*taskContext*/, void>, void* /*taskContext*/, void* /*userContext*/, sbyte* /*taskName*/, void*>;
using unsafe b3FinishTaskCallback = delegate* unmanaged<void* /*userTask*/, void* /*userContext*/, void>;

using unsafe b3CreateDebugShapeCallback = delegate* unmanaged<Alimer.Physics.Box3DApi.b3DebugShape* /*userTask*/, void* /*userContext*/, void>;
using unsafe b3DestroyDebugShapeCallback = delegate* unmanaged<void* /*userShape*/, void* /*userContext*/, void>;

namespace Alimer.Physics;

unsafe partial class Box3DApi
{
    public const string LibraryName = "box3d";
    // Use to validate definitions. Do not take my cookie.
    public const int B3_SECRET_COOKIE = 1152023;


    [CustomMarshaller(typeof(string), MarshalMode.ManagedToUnmanagedOut, typeof(UTF8OwnedMarshaler))]
    public static class UTF8OwnedMarshaler
    {
        /// <summary>
        /// Converts an unmanaged string to a managed version.
        /// </summary>
        /// <returns>A managed string.</returns>
        public static string? ConvertToManaged(byte* unmanaged)
        {
            if (unmanaged == null)
                return null;

            return UTF8EncodingRelaxed.Default.GetString(MemoryMarshal.CreateReadOnlySpanFromNullTerminated(unmanaged));
        }

        internal sealed class UTF8EncodingRelaxed : UTF8Encoding
        {
            public static new readonly UTF8EncodingRelaxed Default = new();

            private UTF8EncodingRelaxed() : base(false, false)
            {
            }
        }
    }

    #region Structs
    [StructLayout(LayoutKind.Sequential)]
    public struct b3Vec2
    {
        public float x;
        public float y;

        public static implicit operator b3Vec2(in Vector2 vector) => Unsafe.BitCast<Vector2, b3Vec2>(vector);
        public static implicit operator Vector2(in b3Vec2 vector) => Unsafe.BitCast<b3Vec2, Vector2>(vector);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Vec3
    {
        public float x;
        public float y;
        public float z;

        public static implicit operator b3Vec3(in Vector3 vector) => Unsafe.BitCast<Vector3, b3Vec3>(vector);
        public static implicit operator Vector3(in b3Vec3 vector) => Unsafe.BitCast<b3Vec3, Vector3>(vector);
    }

#if BOX3D_DOUBLE_PRECISION
    [StructLayout(LayoutKind.Sequential)]
    public struct b3Pos
    {
        public double x;
        public double y;
        public double z;

        public static implicit operator b3Pos(in Vector3 vector) => new() { x = vector.X, y = vector.Y, z = vector.Z };
        public static implicit operator Vector3(in b3Pos vector) => new((float)vector.x, (float)vector.y, (float)vector.z);
    }
#endif

    [StructLayout(LayoutKind.Sequential)]
    public struct b3CosSin
    {
        public float cosine;
        public float sine;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Quat
    {
        public b3Vec3 v;
        public float s;

        public static implicit operator b3Quat(in Quaternion quaternion) => Unsafe.BitCast<Quaternion, b3Quat>(quaternion);
        public static implicit operator Quaternion(in b3Quat quaternion) => Unsafe.BitCast<b3Quat, Quaternion>(quaternion);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Transform
    {
        public b3Vec3 p;
        public b3Quat q;

        public static implicit operator PhysicsTransform(in b3Transform transform) => Unsafe.BitCast<b3Transform, PhysicsTransform>(transform);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Matrix3
    {
        public b3Vec3 cx;
        public b3Vec3 cy;
        public b3Vec3 cz;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3AABB
    {
        public b3Vec3 lowerBound;
        public b3Vec3 upperBound;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Plane
    {
        public b3Vec3 normal;
        public float offset;

        public static implicit operator b3Plane(in Plane plane) => Unsafe.BitCast<Plane, b3Plane>(plane);
        public static implicit operator Plane(in b3Plane plane) => Unsafe.BitCast<b3Plane, Plane>(plane);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3MassData
    {
        public float mass;
        public b3Vec3 center;
        public b3Matrix3 inertia;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Sphere
    {
        public b3Vec3 center;
        public float radius;

        public static implicit operator b3Sphere(in BoundingSphere sphere) => Unsafe.BitCast<BoundingSphere, b3Sphere>(sphere);
        public static implicit operator BoundingSphere(in b3Sphere sphere) => Unsafe.BitCast<b3Sphere, BoundingSphere>(sphere);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Capsule
    {
        public b3Vec3 center1;
        public b3Vec3 center2;
        public float radius;
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct b3ExplosionDef
    {
        /// Mask bits to filter shapes
        public ulong maskBits;

        /// The center of the explosion in world space
        public b3Pos position;

        /// The radius of the explosion
        public float radius;

        /// The falloff distance beyond the radius. Impulse is reduced to zero at this distance.
        public float falloff;

        /// Impulse per unit area. This applies an impulse according to the shape area that
        /// is facing the explosion. Explosions only apply to spheres, capsules, and hulls. This
        /// may be negative for implosions.
        public float impulsePerArea;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3WorldId
    {
        public ushort index1;
        public ushort generation;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3BodyId
    {
        public int index1;
        public ushort world0;
        public ushort generation;

        public readonly bool IsNull => index1 == 0;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3ShapeId
    {
        public int index1;
        public ushort world0;
        public ushort generation;

        public readonly bool IsNull => index1 == 0;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3JointId
    {
        public int index1;
        public ushort world0;
        public ushort generation;

        public readonly bool IsNull => index1 == 0;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3ContactId
    {
        public int index1;
        public ushort world0;
        public short padding;
        public uint generation;

        public readonly bool IsNull => index1 == 0;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Capacity
    {
        public int staticShapeCount;
        public int dynamicShapeCount;
        public int staticBodyCount;
        public int dynamicBodyCount;
        public int contactCount;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3WorldDef
    {
        public b3Vec3 gravity;
        public float restitutionThreshold;
        public float hitEventThreshold;
        public float contactHertz;
        public float contactDampingRatio;
        public float contactSpeed;
        public float maximumLinearSpeed;
        public b3FrictionCallback frictionCallback;
        public b3RestitutionCallback restitutionCallback;
        public NativeBool enableSleep;
        public NativeBool enableContinuous;
        public uint workerCount;
        public b3EnqueueTaskCallback enqueueTask;
        public b3FinishTaskCallback finishTask;
        public nint userTaskContext;
        public nint userData;
        public b3CreateDebugShapeCallback createDebugShape;
        public b3DestroyDebugShapeCallback destroyDebugShape;
        public nint userDebugShapeContext;
        public b3Capacity capacity;
        public int internalValue;
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct b3DebugShape
    {
        [FieldOffset(0)]
        public b3ShapeId shapeId;

        [FieldOffset(8)]
        public b3ShapeType type;

        [FieldOffset(16)]
        public b3Capsule* capsule;			  ///< Capsule shape.
		[FieldOffset(16)]
        public b3CompoundData* compound;		  ///< Compound shape.
		[FieldOffset(16)]
        public b3HeightFieldData* heightField; ///< Height-field shape.
		[FieldOffset(16)]
        public b3HullData* hull;				  ///< Convex hull shape.
		[FieldOffset(16)]
        public b3Mesh* mesh;					  ///< Mesh shape with scale.
		[FieldOffset(16)]
        public b3Sphere* sphere;				  ///< Sphere shape.
	}
    #endregion

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3SetLengthUnitsPerMeter(float lengthUnits);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3GetLengthUnitsPerMeter();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3SetStallThreshold(float seconds);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3GetStallThreshold();

    #region World
    [LibraryImport(LibraryName)]

    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3WorldDef b3DefaultWorldDef();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3WorldId b3CreateWorld(in b3WorldDef def);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3DestroyWorld(b3WorldId worldId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int b3GetWorldCount();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int b3GetMaxWorldCount();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3World_IsValid(b3WorldId id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3World_Step(b3WorldId worldId, float timeStep, int subStepCount);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3World_SetGravity(b3WorldId worldId, b3Vec3 gravity);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3World_GetGravity(b3WorldId worldId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3World_SetUserData(b3WorldId worldId, nint userData);


    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint b3World_GetUserData(b3WorldId worldId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3World_Explode(b3WorldId worldId, in b3ExplosionDef explosionDef);
    #endregion

    #region Body
    public enum b3BodyType
    {
        b3_staticBody = 0,
        b3_kinematicBody = 1,
        b3_dynamicBody = 2,
        b3_bodyTypeCount = 3,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3MotionLocks
    {
        public NativeBool linearX;
        public NativeBool linearY;
        public NativeBool linearZ;
        public NativeBool angularX;
        public NativeBool angularY;
        public NativeBool angularZ;
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct b3BodyDef
    {
        public b3BodyType type;
        public b3Pos position;
        public b3Quat rotation;
        public b3Vec3 linearVelocity;
        public b3Vec3 angularVelocity;
        public float linearDamping;
        public float angularDamping;
        public float gravityScale;
        public float sleepThreshold;
        public byte* name;
        public IntPtr userData;
        public b3MotionLocks motionLocks;
        public NativeBool enableSleep;
        public NativeBool isAwake;
        public NativeBool isBullet;
        public NativeBool isEnabled;
        public NativeBool allowFastRotation;
        public NativeBool enableContactRecycling;
        public int internalValue;
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3BodyDef b3DefaultBodyDef();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3BodyId b3CreateBody(b3WorldId worldId, in b3BodyDef def);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3DestroyBody(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsValid(b3BodyId id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3BodyType b3Body_GetType(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetType(b3BodyId bodyId, b3BodyType type);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetName(b3BodyId bodyId, string name);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(UTF8OwnedMarshaler))]
    public static partial string? b3Body_GetName(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetUserData(b3BodyId bodyId, IntPtr userData);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial IntPtr b3Body_GetUserData(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetPosition(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Quat b3Body_GetRotation(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Transform b3Body_GetTransform(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetTransform(b3BodyId bodyId, b3Vec3 position, b3Quat rotation);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetLocalPoint(b3BodyId bodyId, b3Vec3 worldPoint);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetWorldPoint(b3BodyId bodyId, b3Vec3 localPoint);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetLocalVector(b3BodyId bodyId, b3Vec3 worldVector);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetWorldVector(b3BodyId bodyId, b3Vec3 localVector);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetLinearVelocity(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetAngularVelocity(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetLinearVelocity(b3BodyId bodyId, b3Vec3 linearVelocity);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetAngularVelocity(b3BodyId bodyId, b3Vec3 angularVelocity);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetTargetTransform(b3BodyId bodyId, b3Transform target, float timeStep, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetLocalPointVelocity(b3BodyId bodyId, b3Vec3 localPoint);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetWorldPointVelocity(b3BodyId bodyId, b3Vec3 worldPoint);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyForce(b3BodyId bodyId, b3Vec3 force, b3Vec3 point, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyForceToCenter(b3BodyId bodyId, b3Vec3 force, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyTorque(b3BodyId bodyId, b3Vec3 torque, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyLinearImpulse(b3BodyId bodyId, b3Vec3 impulse, b3Vec3 point, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyLinearImpulseToCenter(b3BodyId bodyId, b3Vec3 impulse, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyAngularImpulse(b3BodyId bodyId, b3Vec3 impulse, NativeBool wake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3Body_GetMass(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Matrix3 b3Body_GetLocalRotationalInertia(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3Body_GetInverseMass(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Matrix3 b3Body_GetWorldInverseRotationalInertia(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetLocalCenter(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Vec3 b3Body_GetWorldCenter(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetMassData(b3BodyId bodyId, b3MassData massData);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3MassData b3Body_GetMassData(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_ApplyMassFromShapes(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetLinearDamping(b3BodyId bodyId, float linearDamping);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3Body_GetLinearDamping(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetAngularDamping(b3BodyId bodyId, float angularDamping);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3Body_GetAngularDamping(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetGravityScale(b3BodyId bodyId, float gravityScale);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3Body_GetGravityScale(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsAwake(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetAwake(b3BodyId bodyId, NativeBool awake);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_EnableSleep(b3BodyId bodyId, NativeBool enableSleep);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsSleepEnabled(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetSleepThreshold(b3BodyId bodyId, float sleepThreshold);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float b3Body_GetSleepThreshold(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsEnabled(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_Disable(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_Enable(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetMotionLocks(b3BodyId bodyId, b3MotionLocks locks);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3MotionLocks b3Body_GetMotionLocks(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_SetBullet(b3BodyId bodyId, NativeBool flag);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsBullet(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_AllowFastRotation(b3BodyId bodyId, NativeBool flag);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsFastRotationAllowed(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_EnableContactRecycling(b3BodyId bodyId, NativeBool flag);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Body_IsContactRecyclingEnabled(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3Body_EnableHitEvents(b3BodyId bodyId, NativeBool flag);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3WorldId b3Body_GetWorld(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int b3Body_GetShapeCount(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int b3Body_GetShapes(b3BodyId bodyId, b3ShapeId* shapeArray, int capacity);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int b3Body_GetJointCount(b3BodyId bodyId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int b3Body_GetJoints(b3BodyId bodyId, b3JointId* jointArray, int capacity);
    #endregion

    #region Filter
    [StructLayout(LayoutKind.Sequential)]
    public struct b3Filter
    {
        /// <summary>
        /// The collision category bits. Normally you would just set one bit. The category bits should
        /// represent your application object types. For example:
        /// enum MyCategories
        /// {
        ///    Static  = 0x00000001,
        ///    Dynamic = 0x00000002,
        ///    Debris  = 0x00000004,
        ///    Player  = 0x00000008,
        ///    // etc
        /// };
        /// </summary>
        public ulong categoryBits;
        /// <summary>
        /// The collision mask bits. This states the categories that this
        /// shape would accept for collision.
        /// For example, you may want your player to only collide with static objects
        /// and other players.
        /// @code{.c}
        /// maskBits = Static | Player;
        /// @endcode
        /// </summary>
        public ulong maskBits;

        /// <summary>
        /// Collision groups allow a certain group of objects to never collide (negative)
        /// or always collide (positive). A group index of zero has no effect. Non-zero group filtering
        /// always wins against the mask bits.
        /// For example, you may want ragdolls to collide with other ragdolls but you don't want
        /// ragdoll self-collision. In this case you would give each ragdoll a unique negative group index
        /// and apply that group index to all shapes on the ragdoll.
        /// </summary>
        public int groupIndex;
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3Filter b3DefaultFilter();
    #endregion

    #region SurfaceMaterial
    [StructLayout(LayoutKind.Sequential)]
    public struct b3SurfaceMaterial
    {
        public float friction;
        public float restitution;
        public float rollingResistance;
        public b3Vec3 tangentVelocity;
        public ulong userMaterialId;
        public uint customColor;
        public uint padding;
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3SurfaceMaterial b3DefaultSurfaceMaterial();

    #endregion

    #region Shape
    public enum b3ShapeType
    {
        b3_capsuleShape = 0,
        b3_compoundShape = 1,
        b3_heightShape = 2,
        b3_hullShape = 3,
        b3_meshShape = 4,
        b3_sphereShape = 5,
        b3_shapeTypeCount = 6,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3ShapeDef
    {
        public byte* name;
        public IntPtr userData;
        public b3SurfaceMaterial* materials;
        public int materialCount;
        public b3SurfaceMaterial baseMaterial;
        public float density;
        public float explosionScale;
        public b3Filter filter;
        public NativeBool enableCustomFiltering;
        public NativeBool isSensor;
        public NativeBool enableSensorEvents;
        public NativeBool enableContactEvents;
        public NativeBool enableHitEvents;
        public NativeBool enablePreSolveEvents;
        public NativeBool invokeContactCreation;
        public NativeBool updateBodyMass;
        public NativeBool enableSpeculativeContact;
        public int internalValue;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3MeshData
    {
        public ulong version;
        public ulong hash;
        public int byteCount;
        public b3AABB bounds;
        public float surfaceArea;
        public int treeHeight;
        public int degenerateCount;
        public int nodeOffset;
        public int nodeCount;
        public int vertexOffset;
        public int vertexCount;
        public int triangleOffset;
        public int triangleCount;
        public int materialOffset;
        public int materialCount;
        public int flagsOffset;
        public int padding;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3Mesh
    {
        public b3MeshData* data;
        public b3Vec3 scale;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3HeightFieldData
    {
        public ulong version;
        public ulong hash;
        public int byteCount;
        public b3AABB aabb;
        public float minHeight;
        public float maxHeight;
        public float heightScale;
        public b3Vec3 scale;
        public int columnCount;
        public int rowCount;
        public int heightsOffset;
        public int materialOffset;
        public int flagsOffset;
        public byte clockwise;
        public fixed byte padding[7];
    }

    [Flags]
    public enum b3TreeNodeFlags : uint
    {
        b3_allocatedNode = 0x0001,
        b3_enlargedNode = 0x0002,
        b3_leafNode = 0x0004,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3TreeNodeChildren
    {
        public int child1;
        public int child2;
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct b3TreeNode
    {
        [FieldOffset(0)]
        public b3AABB aabb;
        [FieldOffset(24)]
        public ulong categoryBits;
        [FieldOffset(32)]
        public b3TreeNodeChildren __children;
        [FieldOffset(32)]
        public ulong __userData;
        [FieldOffset(40)]
        public int __parent;
        [FieldOffset(40)]
        public int __next;
        [FieldOffset(44)]
        public ushort height;
        [FieldOffset(46)]
        public ushort flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3DynamicTree
    {
        public ulong version;
        public b3TreeNode* nodes;
        public int root;
        public int nodeCount;
        public int nodeCapacity;
        public int proxyCount;
        public int freeList;
        public int* leafIndices;
        public b3AABB* leafBoxes;
        public b3Vec3* leafCenters;
        public int* binIndices;
        public int rebuildCapacity;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3CompoundData
    {
        public ulong version;
        public int byteCount;
        public int nodeOffset;
        public b3DynamicTree tree;
        public int materialOffset;
        public int materialCount;
        public int capsuleOffset;
        public int capsuleCount;
        public int hullOffset;
        public int hullCount;
        public int sharedHullCount;
        public int meshOffset;
        public int meshCount;
        public int sharedMeshCount;
        public int sphereOffset;
        public int sphereCount;
    }

    [LibraryImport(LibraryName)]

    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeDef b3DefaultShapeDef();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeId b3CreateSphereShape(b3BodyId bodyId, in b3ShapeDef def, in b3Sphere sphere);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeId b3CreateCapsuleShape(b3BodyId bodyId, in b3ShapeDef def, in b3Capsule capsule);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeId b3CreateHullShape(b3BodyId bodyId, in b3ShapeDef def, b3HullData* hull);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeId b3CreateTransformedHullShape(b3BodyId bodyId, in b3ShapeDef def, in b3HullData hull, b3Transform transform, b3Vec3 scale);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeId b3CreateMeshShape(b3BodyId bodyId, in b3ShapeDef def, b3MeshData* mesh, b3Vec3 scale);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeId b3CreateHeightFieldShape(b3BodyId bodyId, in b3ShapeDef def, in b3HeightFieldData heightField);

    //LibraryImport(LibraryName)]
    //UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    //ublic static partial b3ShapeId b3CreateBakedCompoundShape(b3BodyId bodyId, in b3ShapeDef def, in b3CompoundData compound);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void b3DestroyShape(b3ShapeId shapeId, NativeBool updateBodyMass);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial NativeBool b3Shape_IsValid(b3ShapeId id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3ShapeType b3Shape_GetType(b3ShapeId shapeId);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3BodyId b3Shape_GetBody(b3ShapeId shapeId);
    #endregion

    #region BoxHull
    [StructLayout(LayoutKind.Sequential)]
    public struct b3HullVertex
    {
        public byte edge;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3HullHalfEdge
    {
        public byte next;
        public byte twin;
        public byte origin;
        public byte face;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3HullFace
    {
        public byte edge;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3HullData
    {
        public ulong version;
        public ulong hash;
        public b3AABB aabb;
        public float surfaceArea;
        public float volume;
        public float innerRadius;
        public b3Vec3 center;
        public b3Matrix3 centralInertia;
        public int vertexCount;
        public int vertexOffset;
        public int pointOffset;
        public int edgeCount;
        public int edgeOffset;
        public int faceCount;
        public int planeOffset;
        public int faceOffset;
        public int soaVertexOffset;
        public int soaNormalOffset;
        public int byteCount;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct b3BoxHull
    {
        public b3HullData @base;
        public b3HullVertex boxVertices0;
        public b3HullVertex boxVertices1;
        public b3HullVertex boxVertices2;
        public b3HullVertex boxVertices3;
        public b3HullVertex boxVertices4;
        public b3HullVertex boxVertices5;
        public b3HullVertex boxVertices6;
        public b3HullVertex boxVertices7;
        public b3Vec3 boxPoints0;
        public b3Vec3 boxPoints1;
        public b3Vec3 boxPoints2;
        public b3Vec3 boxPoints3;
        public b3Vec3 boxPoints4;
        public b3Vec3 boxPoints5;
        public b3Vec3 boxPoints6;
        public b3Vec3 boxPoints7;
        public b3HullHalfEdge boxEdges0;
        public b3HullHalfEdge boxEdges1;
        public b3HullHalfEdge boxEdges2;
        public b3HullHalfEdge boxEdges3;
        public b3HullHalfEdge boxEdges4;
        public b3HullHalfEdge boxEdges5;
        public b3HullHalfEdge boxEdges6;
        public b3HullHalfEdge boxEdges7;
        public b3HullHalfEdge boxEdges8;
        public b3HullHalfEdge boxEdges9;
        public b3HullHalfEdge boxEdges10;
        public b3HullHalfEdge boxEdges11;
        public b3HullHalfEdge boxEdges12;
        public b3HullHalfEdge boxEdges13;
        public b3HullHalfEdge boxEdges14;
        public b3HullHalfEdge boxEdges15;
        public b3HullHalfEdge boxEdges16;
        public b3HullHalfEdge boxEdges17;
        public b3HullHalfEdge boxEdges18;
        public b3HullHalfEdge boxEdges19;
        public b3HullHalfEdge boxEdges20;
        public b3HullHalfEdge boxEdges21;
        public b3HullHalfEdge boxEdges22;
        public b3HullHalfEdge boxEdges23;
        public b3Plane boxPlanes0;
        public b3Plane boxPlanes1;
        public b3Plane boxPlanes2;
        public b3Plane boxPlanes3;
        public b3Plane boxPlanes4;
        public b3Plane boxPlanes5;
        public b3HullFace boxFaces0;
        public b3HullFace boxFaces1;
        public b3HullFace boxFaces2;
        public b3HullFace boxFaces3;
        public b3HullFace boxFaces4;
        public b3HullFace boxFaces5;
        public fixed byte padding[2];
        public fixed float vx[8];
        public fixed float vy[8];
        public fixed float vz[8];
        public fixed float nx[8];
        public fixed float ny[8];
        public fixed float nz[8];
    }

    public static b3BoxHull b3MakeCubeHull(float halfWidth)
    {
        return b3MakeBoxHull(halfWidth, halfWidth, halfWidth);
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial b3BoxHull b3MakeBoxHull(float hx, float hy, float hz);
    #endregion
}
