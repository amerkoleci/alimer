// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
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
            if (Handle != null)
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
            if(Handle != null)
            {
            }
        }
    }

    public override Matrix4x4 PhysicsWorldTransform
    {
        get
        {
            if (Handle is null)
                return Matrix4x4.Identity;

            return Handle.GetWorldTransform();
        }
        set
        {
            // TODO: Update Jolt WorldTransform?
            if (Handle != null)
            {
            }
        }
    }

    internal Body? Handle { get; private set; }
    internal BodyID BodyID { get; private set; }

    protected override void OnAttach()
    {
        base.OnAttach();

        if (Handle != null)
        {
            Simulation!.BodyInterface.DestroyBody(Handle.ID);
        }

        if (_shape is null)
            return;

        Matrix4x4.Decompose(Entity!.Transform.WorldMatrix, out _, out Quaternion rotation, out Vector3 translation);

        BodyCreationSettings bodySettings = new(
            _shape.Handle,
            translation,
            rotation,
            MotionType.ToJolt(),
            (MotionType == MotionType.Static) ? PhysicsSimulation.Layers.NonMoving : PhysicsSimulation.Layers.Moving);

        Handle = Simulation!.BodyInterface.CreateBody(bodySettings);
        BodyID = Handle.ID;

        // Add it to the world
        Simulation!.BodyInterface.AddBody(Handle, (MotionType == MotionType.Static) ? Activation.DontActivate : Activation.Activate);
        Simulation.RigidBodies.Add(BodyID,  this);
    }

    protected override void OnDetach()
    {
        base.OnDetach();

        Simulation!.RigidBodies.Remove(BodyID);
        Simulation!.BodyInterface.RemoveBody(BodyID);
        Simulation!.BodyInterface.DestroyBody(BodyID);
    }
}
