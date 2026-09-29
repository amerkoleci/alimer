// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;
using static Alimer.Physics.Box3DApi;

namespace Alimer.Physics;

public sealed class PhysicsWorld : DisposableObject
{
    private Vector3 _gravity;
    private readonly GCHandle _handle;

    public IReadOnlyCollection<RigidBody> RigidBodies => RigidBodiesDictionary.Values;

    internal Dictionary<b3BodyId, RigidBody> RigidBodiesDictionary { get; } = [];

    public PhysicsWorld()
        : this(new PhysicsWorldSettings())
    {
    }

    public PhysicsWorld(PhysicsWorldSettings settings)
    {
        _gravity = settings.GravityDirection * MathF.Abs(PhysicsWorldSettings.EarthGravity) * settings.GravityScale;

        // TODO: Add Layers/LayerMask
        b3WorldDef config = b3DefaultWorldDef();
        config.gravity = _gravity;
        Debug.Assert(config.internalValue == B3_SECRET_COOKIE);
        //config.internalValue = B3_SECRET_COOKIE;
        ID = b3CreateWorld(in config);
        _handle = GCHandle.Alloc(this);
        b3World_SetUserData(ID, GCHandle.ToIntPtr(_handle));
    }

    /// <inheritdoc />
    protected override void DisposeUnmanagedResources()
    {
        if (_handle.IsAllocated)
            _handle.Free();

        b3DestroyWorld(ID);
        ID = default;
    }


    internal b3WorldId ID { get; private set; }
    public bool IsValid => b3World_IsValid(ID);

    public Vector3 Gravity
    {
        get => _gravity;
        set
        {
            _gravity = value;
            b3World_SetGravity(ID, _gravity);
        }
    }

    public RigidBody CreateRigidBody() => CreateRigidBody(new());

    public RigidBody CreateRigidBody(in RigidBodySettings settings)
    {
        RigidBody body = new(this, in settings);
        RigidBodiesDictionary.Add(body.ID, body);
        return body;
    }

    public void Step(float timeStep, int subStepCount = 4)
    {
        b3World_Step(ID, timeStep, subStepCount);
    }

    internal static PhysicsWorld? FromUserData(b3WorldId worldId)
    {
        if (!b3World_IsValid(worldId))
            return default;

        nint ptr = b3World_GetUserData(worldId);
        if (ptr == 0)
            return default;

        return GCHandle.FromIntPtr(ptr).Target as PhysicsWorld;
    }
}
