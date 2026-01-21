// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Alimer.AlimerApi;

namespace Alimer.Physics;

public sealed class PhysicsSimulation : DisposableObject
{
    private const int MaxBodies = 65536;
    private const int MaxBodyPairs = 65536;
    private bool _optimizeBroadPhase = true;

    internal readonly PhysicsWorld World;
    internal Dictionary<PhysicsBody, RigidBodyComponent> RigidBodies { get; } = [];

    public PhysicsSimulation()
    {
        // TODO: Add Layers/LayerMask
        PhysicsWorldConfig config = new()
        {
            maxBodies = MaxBodies,
            maxBodyPairs = MaxBodyPairs
        };
        World = alimerPhysicsWorldCreate(in config);

#if TODO_Events
        // ContactListener
        InternalSimulation.OnContactValidate += OnContactValidate;
        InternalSimulation.OnContactAdded += OnContactAdded;
        InternalSimulation.OnContactPersisted += OnContactPersisted;
        InternalSimulation.OnContactRemoved += OnContactRemoved;
        // BodyActivationListener
        InternalSimulation.OnBodyActivated += OnBodyActivated;
        InternalSimulation.OnBodyDeactivated += OnBodyDeactivated;
#endif
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            alimerPhysicsWorldDestroy(World);
        }
    }

    public void Step(float deltaTime)
    {
        if (_optimizeBroadPhase)
        {
            alimerPhysicsWorldOptimizeBroadPhase(World);
            _optimizeBroadPhase = false;
        }

        // When running below 55 Hz, do 2 steps instead of 1
        int numSteps = 1; // deltaTime > 1.0 / 55.0 ? 2 : 1;

        //const int steps = ::clamp(int(dt / TIMESTEP), 1, ACCURACY);

        //PhysicsUpdateError error = alimerPhysicsWorldUpdate(_world, deltaTime, numSteps);
        //Debug.Assert(error == PhysicsUpdateError.None);
        _ = alimerPhysicsWorldUpdate(World, deltaTime, numSteps);
    }

#if TODO_Events
    #region ContactListener
    private ValidateResult OnContactValidate(JoltPhysicsSystem system, in Body body1, in Body body2, RVector3 baseOffset, in CollideShapeResult collisionResult)
    {
        Console.WriteLine("Contact validate callback");

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return ValidateResult.AcceptAllContactsForThisBodyPair;
    }

    private void OnContactAdded(JoltPhysicsSystem system, in Body body1, in Body body2, in ContactManifold manifold, in ContactSettings settings)
    {
        Console.WriteLine("A contact was added");
    }

    private void OnContactPersisted(JoltPhysicsSystem system, in Body body1, in Body body2, in ContactManifold manifold, in ContactSettings settings)
    {
        Console.WriteLine("A contact was persisted");
    }

    private void OnContactRemoved(JoltPhysicsSystem system, ref SubShapeIDPair subShapePair)
    {
        Console.WriteLine("A contact was removed");
    }
    #endregion

    #region BodyActivationListener
    private void OnBodyActivated(JoltPhysicsSystem system, in BodyID bodyID, ulong bodyUserData)
    {
        Console.WriteLine("A body got activated");
    }

    private void OnBodyDeactivated(JoltPhysicsSystem system, in BodyID bodyID, ulong bodyUserData)
    {
        Console.WriteLine("A body went to sleep");
    }
    #endregion
#endif
}
