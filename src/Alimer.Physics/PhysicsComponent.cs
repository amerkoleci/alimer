// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;
using Alimer.Engine;

namespace Alimer.Physics;

[Meta]
[DefaultEntitySystem(typeof(PhysicsSystem))]
public abstract partial class PhysicsComponent : Component
{
    [IgnoreDataMember]
    public PhysicsSimulation? Simulation { get; internal set; }
    [IgnoreDataMember]
    public abstract Matrix4x4 PhysicsWorldTransform { get; set; }

    public void UpdateTransformComponent()
    {
        Matrix4x4.Decompose(Entity!.Transform.WorldMatrix, out Vector3 scale, out _, out _);

        Entity.Transform.WorldMatrix = Matrix4x4.CreateScale(scale) * PhysicsWorldTransform;
        Entity.Transform.UpdateLocalFromWorldMatrix();
    }

    internal void Attach()
    {
        OnAttach();
    }

    internal void Detach()
    {
        OnDetach();
    }

    protected virtual void OnAttach()
    {
        Entity!.Transform.UpdateWorldMatrix();
    }

    protected virtual void OnDetach()
    {
    }
}
