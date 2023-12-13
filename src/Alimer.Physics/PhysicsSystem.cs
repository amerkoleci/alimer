// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Engine;

namespace Alimer.Physics;

public class PhysicsSystem : EntitySystem<PhysicsComponent>
{
#pragma warning disable CA2255
    [ModuleInitializer]
#pragma warning restore CA2255
    internal static void Register()
    {
        EntityManager.RegisterSystemFactory<PhysicsSystem>();
    }

    public PhysicsSystem()
    : base(typeof(TransformComponent))
    {
    }

    public PhysicsSimulation Simulation { get; } = new();

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Simulation.Dispose();
        }

        base.Dispose(disposing);
    }

    public override void Update(AppTime time)
    {
        //foreach (var rigidBody in Simulation.RigidBodies)
        //{
        //    rigidBody.Value.PhysicsWorldTransform = rigidBody.Value.Entity!.Transform.WorldMatrix;
        //}

        Simulation.Timestep(time.Elapsed);

        //foreach (var rigidBody in Simulation.RigidBodies)
        //{
        //    rigidBody.Value.UpdateTransformComponent();
        //}
    }
}
