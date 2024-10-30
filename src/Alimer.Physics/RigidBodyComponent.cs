// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;
using Alimer.Engine;
using JoltPhysicsSharp;

namespace Alimer.Physics;

public class RigidBodyComponent : PhysicsComponent
{
    private MotionType _motionType = MotionType.Dynamic;
    private ColliderShape? _shape;
    private float _mass = 1.0f;

    public MotionType MotionType
    {
        get => _motionType;
        set
        {
            if (_motionType == value)
                return;

            _motionType = value;
            if (Handle.IsNotNull)
            {
                Handle.MotionType = value.ToJolt();
            }
        }
    }

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
        get => _mass;
        set
        {
            if (value < 0.0f)
            {
                throw new InvalidOperationException("The mass of a RigidBody cannot be negative.");
            }

            _mass = value;

            // TODO: Update Jolt mass?
            if (Handle.IsNotNull)
            {
            }
        }
    }

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

            return Simulation!.BodyInterfaceNoLock.GetLinearVelocity(BodyID);
        }
        set
        {
            if (!IsValid)
                return;

            //Handle.SetLinearVelocity(value);
            Simulation!.BodyInterfaceNoLock.SetLinearVelocity(BodyID, value);
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

            return Handle.GetWorldTransform();
        }
        set
        {
            // TODO: Update Jolt WorldTransform?
            if (Handle.IsNotNull)
            {
            }
        }
    }

    internal Body Handle;
    internal BodyID BodyID { get; private set; }
    internal bool IsValid => BodyID.IsValid;

    protected override void OnAttach()
    {
        base.OnAttach();

        if (Handle.IsNotNull)
        {
            Simulation!.BodyInterface.DestroyBody(Handle.ID);
        }

        if (_shape is null)
            return;

        BodyInterface bodyInterface = Simulation!.BodyInterface;

        Matrix4x4.Decompose(Entity!.Transform.WorldMatrix, out _, out Quaternion rotation, out Vector3 translation);

        BodyCreationSettings bodySettings = new(
            _shape.Handle,
            translation,
            rotation,
            MotionType.ToJolt(),
            (MotionType == MotionType.Static) ? PhysicsSimulation.Layers.NonMoving : PhysicsSimulation.Layers.Moving);

        Handle = bodyInterface.CreateBody(bodySettings);
        BodyID = Handle.ID;

        // Add it to the world
        bodyInterface.AddBody(Handle, (MotionType == MotionType.Static) ? Activation.DontActivate : Activation.Activate);
        Simulation.RigidBodies.Add(BodyID, this);
    }

    protected override void OnDetach()
    {
        base.OnDetach();

        Simulation!.RigidBodies.Remove(BodyID);
        Simulation!.BodyInterface.RemoveBody(BodyID);
        Simulation!.BodyInterface.DestroyBody(BodyID);
    }
}
