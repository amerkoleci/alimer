// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Engine;
using JoltPhysicsSharp;

namespace Alimer.Physics;

public class PhysicsSystem : EntitySystem<PhysicsComponent>
{
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
        BodyInterface bodyInterface = Simulation.BodyInterface;

        foreach (var rigidBody in Simulation.RigidBodies)
        {
            //rigidBody.Value.PhysicsWorldTransform = rigidBody.Value.Entity!.Transform.WorldMatrix;
        }

        Simulation.Step((float)time.Elapsed.TotalSeconds);

        foreach (var rigidBody in Simulation.RigidBodies)
        {
            if (bodyInterface.IsActive(rigidBody.Key))
            {
                rigidBody.Value.UpdateTransformComponent();
            }
        }
    }

#pragma warning disable CA2255
    [ModuleInitializer]
    public static void Initialize()
    {
        if (Foundation.Init(false) == false)
        {
            throw new InvalidOperationException("[JoltPhysics] Failed to initialize Foundation");
        }

        Foundation.SetTraceHandler((message) =>
        {
            Log.Trace(message);
        });

#if DEBUG
        Foundation.SetAssertFailureHandler((inExpression, inMessage, inFile, inLine) =>
        {
            string message = inMessage ?? inExpression;

            string outMessage = $"[JoltPhysics] Assertion failure at {inFile}:{inLine}: {message}";

            System.Diagnostics.Debug.WriteLine(outMessage);

            Log.Debug(outMessage);

            throw new Exception(outMessage);
        });
#endif

        EntityManager.RegisterSystemFactory<PhysicsSystem>();
    }
#pragma warning restore CA2255
}
