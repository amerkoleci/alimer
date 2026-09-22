// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Physics;

namespace Alimer;

unsafe partial class AlimerApi
{
    public const string LibraryName = "alimer_physics";

    #region Enums
    public enum PhysicsShapeType
    {
        Box,
        Sphere,
        Capsule,
        Cylinder,
        Convex,
        Mesh,
        Terrain,

        Count,
    }
    #endregion

    #region Structs
    public struct PhysicsBodyTransform
    {
        public Vector3 position;
        public Quaternion rotation;
    }

    public struct PhysicsBodyDesc
    {
        public RigidBodyType type;
        public PhysicsBodyTransform initialTransform;
        public Vector3 linearVelocity;
        public Vector3 angularVelocity;
        public float mass;
        public float linearDamping;
        public float angularDamping;
        public float gravityScale;
        public Bool8 isSensor;
        public Bool8 allowSleeping;
        public Bool8 continuous;
        public uint shapeCount;
        public PhysicsShape* shapes;
    }

    //public struct PhysicsAllocationCallbacks
    //{
    //    public delegate* unmanaged<nuint, nint, void*> allocate;
    //    public delegate* unmanaged<void*, nint, void> free;
    //    public nint userData;
    //}
    //
    //public struct PhysicsConfig
    //{
    //    public PhysicsAllocationCallbacks* allocationCallbacks;
    //}

    public struct PhysicsWorldConfig
    {
        public uint maxBodies;
        public uint maxBodyPairs;
    }
    #endregion

    #region Handles
    public readonly struct PhysicsWorld(nint handle)
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;
    }

    public readonly struct PhysicsMaterial(nint handle)
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;
        public static PhysicsMaterial Null => new(0);
    }

    public readonly struct PhysicsShape(nint handle)
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;
    }

    public readonly struct PhysicsBody(nint handle)
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;
    }
    #endregion

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPhysicsInit();

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsShutdown();

    #region PhysicsWorld
    [LibraryImport(LibraryName)]
    public static partial PhysicsWorld alimerPhysicsWorldCreate(in PhysicsWorldConfig config);

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsWorldDestroy(PhysicsWorld handle);

    [LibraryImport(LibraryName)]
    public static partial int alimerPhysicsWorldGetBodyCount(PhysicsWorld world);
    [LibraryImport(LibraryName)]
    public static partial int alimerPhysicsWorldGetActiveBodyCount(PhysicsWorld world);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsWorldGetGravity(PhysicsWorld world, out Vector3 gravity);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsWorldSetGravity(PhysicsWorld world, in Vector3 gravity);

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsWorldUpdate(PhysicsWorld world, float deltaTime, int collisionSteps);
    #endregion

    #region PhysicsMaterial
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial PhysicsMaterial alimerPhysicsMaterialCreate(string name, float friction, float restitution);
    [LibraryImport(LibraryName)]
    public static partial uint alimerPhysicsMaterialAddRef(PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial uint alimerPhysicsMaterialRelease(PhysicsMaterial material);
    #endregion

    #region PhysicsShape
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsShapeCreateBox(in Vector3 size, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsShapeCreateSphere(float radius, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsShapeCreateCapsule(float height, float radius, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsShapeCreateCylinder(float height, float radius, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsShapeCreateConvexHull(Vector3* points, uint pointsCount, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsShapeCreateMesh(Vector3* vertices, uint verticesCount, uint* indices, uint indicesCount);

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsShapeAddRef(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsShapeRelease(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPhysicsShapeIsValid(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShapeType alimerPhysicsShapeGetType(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial PhysicsBody* alimerPhysicsShapeGetBody(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial float alimerPhysicsShapeGetVolume(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial float alimerPhysicsShapeGetDensity(PhysicsShape shape);
    #endregion

    #region PhysicsBody
    /* Body */
    [LibraryImport(LibraryName)]
    public static partial PhysicsBodyDesc alimerPhysicsBodyDescDefault();
    [LibraryImport(LibraryName)]
    public static partial PhysicsBody alimerPhysicsBodyCreate(PhysicsWorld world, in PhysicsBodyDesc desc);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyAddRef(PhysicsBody body);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyRelease(PhysicsBody body);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPhysicsBodyIsValid(PhysicsBody body);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPhysicsBodyIsActive(PhysicsBody body);

    [LibraryImport(LibraryName)]
    public static partial PhysicsWorld alimerPhysicsBodyGetWorld(PhysicsBody body);

    [LibraryImport(LibraryName)]
    public static partial uint alimerPhysicsBodyGetID(PhysicsBody body);

    [LibraryImport(LibraryName)]
    public static partial RigidBodyType alimerPhysicsBodyGetType(PhysicsBody body);

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodySetType(PhysicsBody body, RigidBodyType value);

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyGetTransform(PhysicsBody body, out PhysicsBodyTransform transform);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodySetTransform(PhysicsBody body, in PhysicsBodyTransform transform);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyGetWorldTransform(PhysicsBody body, out Matrix4x4 transform);

    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyGetLinearVelocity(PhysicsBody body, out Vector3 velocity);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodySetLinearVelocity(PhysicsBody body, in Vector3 velocity);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyGetAngularVelocity(PhysicsBody body, out Vector3 velocity);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodySetAngularVelocity(PhysicsBody body, in Vector3 velocity);
    #endregion
}
