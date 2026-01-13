// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;
using static Alimer.AlimerApi;

namespace Alimer.Physics;

public class RigidBodyComponent : PhysicsComponent
{
    private ColliderShape? _shape;

    /// <summary>
    /// Getrs or sets the body type
    /// </summary>
    public RigidBodyType BodyType
    {
        get;
        set
        {
            if (field == value)
                return;

            field = value;
            if (Handle.IsNotNull)
            {
                alimerPhysicsBodySetType(Handle, value);
            }
        }
    } = RigidBodyType.Dynamic;

    public virtual ColliderShape? ColliderShape
    {
        get => _shape;
        set
        {
            if (_shape == value)
                return;

            _shape = value;

            if (Simulation != null)
            {
            }
        }
    }

    public float Mass
    {
        get;
        set
        {
            field = MathF.Max(value, 0.001f);

            // TODO: Update Jolt mass?
            if (Handle != null)
            {
            }
        }
    } = 1.0f;

    /// <summary>
    /// Gets or sets the linear velocity.
    /// </summary>
    /// <value>
    /// The linear velocity.
    /// </value>
    [IgnoreDataMember]
    [JsonIgnore]
    public Vector3 LinearVelocity
    {
        get
        {
            if (!IsValid)
                return Vector3.Zero;

            alimerPhysicsBodyGetLinearVelocity(Handle, out Vector3 velocity);
            return velocity;
        }
        set
        {
            if (!IsValid)
                return;

            alimerPhysicsBodySetLinearVelocity(Handle, in value);
        }
    }


    [IgnoreDataMember]
    [JsonIgnore]
    public override Matrix4x4 PhysicsWorldTransform
    {
        get
        {
            if (Handle.IsNull)
                return Matrix4x4.Identity;

            alimerPhysicsBodyGetWorldTransform(Handle, out Matrix4x4 transform);
            return transform;
        }
        set
        {
        }
    }

    internal PhysicsBody Handle { get; private set; }
    internal uint BodyID { get; private set; }
    internal bool IsValid => alimerPhysicsBodyIsValid(Handle);

    protected override void OnAttach()
    {
        base.OnAttach();

        if (Handle.IsNotNull)
        {
            alimerPhysicsBodyRelease(Handle);
        }

        if (_shape is null)
            return;

        Matrix4x4.Decompose(Entity!.Transform.WorldMatrix, out _, out Quaternion rotation, out Vector3 translation);

        PhysicsBodyDesc bodyDesc = default;
        alimerPhysicsBodyDescInit(ref bodyDesc);

        bodyDesc.initialTransform = new PhysicsBodyTransform
        {
            position = translation,
            rotation = rotation
        };
        bodyDesc.type = BodyType;

        Handle = alimerPhysicsBodyCreate(Simulation.World, in bodyDesc);
        BodyID = alimerPhysicsBodyGetID(Handle);

        // Add it to the world
        //bodyInterface.AddBody(Handle, (MotionType == MotionType.Static) ? Activation.DontActivate : Activation.Activate);
        Simulation.RigidBodies.Add(Handle, this);
    }

    protected override void OnDetach()
    {
        base.OnDetach();

        Simulation!.RigidBodies.Remove(Handle);
        alimerPhysicsBodyRelease(Handle);
    }
}
