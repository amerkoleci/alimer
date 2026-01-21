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

    public struct PhysicsConfig
    {
        public uint tempAllocatorInitSize;
        public uint maxPhysicsJobs;
        public uint maxPhysicsBarriers;
    }

    public struct PhysicsWorldConfig
    {
        public uint maxBodies;
        public uint maxBodyPairs;
    }
    #endregion

    #region Handles
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct PhysicsWorld(nint handle) : IEquatable<PhysicsWorld>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static PhysicsWorld Null => new(0);
        public static implicit operator PhysicsWorld(nint handle) => new(handle);
        public static implicit operator nint(PhysicsWorld handle) => handle.Handle;

        public static bool operator ==(PhysicsWorld left, PhysicsWorld right) => left.Handle == right.Handle;
        public static bool operator !=(PhysicsWorld left, PhysicsWorld right) => left.Handle != right.Handle;
        public static bool operator ==(PhysicsWorld left, nint right) => left.Handle == right;
        public static bool operator !=(PhysicsWorld left, nint right) => left.Handle != right;
        public bool Equals(PhysicsWorld other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is PhysicsWorld handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(PhysicsWorld)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct PhysicsMaterial(nint handle) : IEquatable<PhysicsMaterial>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static PhysicsMaterial Null => new(0);
        public static implicit operator PhysicsMaterial(nint handle) => new(handle);
        public static implicit operator nint(PhysicsMaterial handle) => handle.Handle;

        public static bool operator ==(PhysicsMaterial left, PhysicsMaterial right) => left.Handle == right.Handle;
        public static bool operator !=(PhysicsMaterial left, PhysicsMaterial right) => left.Handle != right.Handle;
        public static bool operator ==(PhysicsMaterial left, nint right) => left.Handle == right;
        public static bool operator !=(PhysicsMaterial left, nint right) => left.Handle != right;
        public bool Equals(PhysicsMaterial other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is PhysicsMaterial handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(PhysicsMaterial)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct PhysicsShape(nint handle) : IEquatable<PhysicsShape>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static PhysicsShape Null => new(0);
        public static implicit operator PhysicsShape(nint handle) => new(handle);
        public static implicit operator nint(PhysicsShape handle) => handle.Handle;

        public static bool operator ==(PhysicsShape left, PhysicsShape right) => left.Handle == right.Handle;
        public static bool operator !=(PhysicsShape left, PhysicsShape right) => left.Handle != right.Handle;
        public static bool operator ==(PhysicsShape left, nint right) => left.Handle == right;
        public static bool operator !=(PhysicsShape left, nint right) => left.Handle != right;
        public bool Equals(PhysicsShape other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is PhysicsShape handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(PhysicsShape)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct PhysicsBody(nint handle) : IEquatable<PhysicsBody>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static PhysicsBody Null => new(0);
        public static implicit operator PhysicsBody(nint handle) => new(handle);
        public static implicit operator nint(PhysicsBody handle) => handle.Handle;

        public static bool operator ==(PhysicsBody left, PhysicsBody right) => left.Handle == right.Handle;
        public static bool operator !=(PhysicsBody left, PhysicsBody right) => left.Handle != right.Handle;
        public static bool operator ==(PhysicsBody left, nint right) => left.Handle == right;
        public static bool operator !=(PhysicsBody left, nint right) => left.Handle != right;
        public bool Equals(PhysicsBody other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is PhysicsBody handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(PhysicsBody)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPhysicsInit(in PhysicsConfig config);

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

    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPhysicsWorldUpdate(PhysicsWorld world, float deltaTime, int collisionSteps);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsWorldOptimizeBroadPhase(PhysicsWorld world);
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
    public static partial PhysicsShape alimerPhysicsCreateBoxShape(in Vector3 size, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsCreateSphereShape(float radius, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsCreateCapsuleShape(float height, float radius, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsCreateCylinderShape(float height, float radius, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsCreateConvexHullShape(Vector3* points, uint pointsCount, PhysicsMaterial material);
    [LibraryImport(LibraryName)]
    public static partial PhysicsShape alimerPhysicsCreateMeshShape(Vector3* vertices, uint verticesCount, uint* indices, uint indicesCount);

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
    public static partial void* alimerPhysicsShapeGetUserData(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsShapeSetUserData(PhysicsShape shape, void* userdata);
    [LibraryImport(LibraryName)]
    public static partial float alimerPhysicsShapeGetVolume(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial float alimerPhysicsShapeGetDensity(PhysicsShape shape);
    [LibraryImport(LibraryName)]
    public static partial float alimerPhysicsShapeGetMass(PhysicsShape shape);
    #endregion

    #region PhysicsBody
    /* Body */
    [LibraryImport(LibraryName)]
    public static partial void alimerPhysicsBodyDescInit(ref PhysicsBodyDesc desc);
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
