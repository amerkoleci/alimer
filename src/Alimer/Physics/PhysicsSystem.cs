// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Engine;
using static Alimer.AlimerApi;

namespace Alimer.Physics;

public class PhysicsSystem : EntitySystem<PhysicsComponent>
{
    public PhysicsSystem()
        : base(typeof(TransformComponent))
    {
        PhysicsConfig config = default;
        if (alimerPhysicsInit(in config) == false)
        {
            throw new InvalidOperationException("[JoltPhysics] Failed to initialize Foundation");
        }
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

    protected override void OnEntityComponentAdded(PhysicsComponent component)
    {
        component.Simulation = Simulation;
        component.Attach();
    }

    protected override void OnEntityComponentRemoved(PhysicsComponent component)
    {
        component.Detach();
        component.Simulation = null;
    }

    public override void Update(GameTime time)
    {
        //BodyInterface bodyInterface = Simulation.BodyInterface;

        foreach (var rigidBody in Simulation.RigidBodies)
        {
            //rigidBody.Value.PhysicsWorldTransform = rigidBody.Value.Entity!.Transform.WorldMatrix;
        }

        Simulation.Step((float)time.Elapsed.TotalSeconds);

        foreach (var rigidBody in Simulation.RigidBodies)
        {
            if (alimerPhysicsBodyIsActive(rigidBody.Key))
            {
                rigidBody.Value.UpdateTransformComponent();
            }
        }
    }

#pragma warning disable CA2255
    [ModuleInitializer]
    public static void Initialize()
    {
        EntityManager.RegisterSystemFactory<PhysicsSystem>();
    }
#pragma warning restore CA2255
}
