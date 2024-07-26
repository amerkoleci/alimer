// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using JoltPhysicsSharp;
using JoltPhysicsSystem = JoltPhysicsSharp.PhysicsSystem;

namespace Alimer.Physics;

public sealed class PhysicsSimulation : DisposableObject
{
    private const int MaxBodies = 65536;
    private const int MaxBodyPairs = 65536;
    private const int MaxContactConstraints = 65536;
    private const int NumBodyMutexes = 0;
    private bool _optimizeBroadPhase = true;
    public Dictionary<BodyID, RigidBodyComponent> RigidBodies { get; } = [];

    public PhysicsSimulation()
    {
        // TODO: Add Layers/LayerMask
        // We use only 2 layers: one for non-moving objects and one for moving objects
        ObjectLayerPairFilterTable objectLayerPairFilterTable = new(2);
        objectLayerPairFilterTable.EnableCollision(Layers.NonMoving, Layers.Moving);
        objectLayerPairFilterTable.EnableCollision(Layers.Moving, Layers.Moving);

        // We use a 1-to-1 mapping between object layers and broadphase layers
        BroadPhaseLayerInterfaceTable broadPhaseLayerInterfaceTable = new(2, 2);
        broadPhaseLayerInterfaceTable.MapObjectToBroadPhaseLayer(Layers.NonMoving, BroadPhaseLayers.NonMoving);
        broadPhaseLayerInterfaceTable.MapObjectToBroadPhaseLayer(Layers.Moving, BroadPhaseLayers.Moving);

        ObjectVsBroadPhaseLayerFilterTable objectVsBroadPhaseLayerFilter = new(
            broadPhaseLayerInterfaceTable, 2, objectLayerPairFilterTable, 2
        );

        PhysicsSystemSettings settings = new()
        {
            MaxBodies = MaxBodies,
            MaxBodyPairs = MaxBodyPairs,
            MaxContactConstraints = MaxContactConstraints,
            NumBodyMutexes = NumBodyMutexes,
            ObjectLayerPairFilter = objectLayerPairFilterTable,
            BroadPhaseLayerInterface = broadPhaseLayerInterfaceTable,
            ObjectVsBroadPhaseLayerFilter = objectVsBroadPhaseLayerFilter
        };

        InternalSimulation = new(settings);

        // ContactListener
        InternalSimulation.OnContactValidate += OnContactValidate;
        InternalSimulation.OnContactAdded += OnContactAdded;
        InternalSimulation.OnContactPersisted += OnContactPersisted;
        InternalSimulation.OnContactRemoved += OnContactRemoved;
        // BodyActivationListener
        InternalSimulation.OnBodyActivated += OnBodyActivated;
        InternalSimulation.OnBodyDeactivated += OnBodyDeactivated;
    }

    internal JoltPhysicsSystem InternalSimulation { get; }
    internal BodyInterface BodyInterface => InternalSimulation.BodyInterface;
    internal BodyInterface BodyInterfaceNoLock => InternalSimulation.BodyInterfaceNoLock;

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            InternalSimulation.Dispose();
        }
    }

    public void Step(float deltaTime)
    {
        if (_optimizeBroadPhase)
        {
            InternalSimulation.OptimizeBroadPhase();
            _optimizeBroadPhase = false;
        }

        // When running below 55 Hz, do 2 steps instead of 1
        int numSteps = 1; // deltaTime > 1.0 / 55.0 ? 2 : 1;

        //const int steps = ::clamp(int(dt / TIMESTEP), 1, ACCURACY);

        InternalSimulation.Step(deltaTime, numSteps);
    }

    #region ContactListener
    private ValidateResult OnContactValidate(JoltPhysicsSystem system, in Body body1, in Body body2, Double3 baseOffset, nint collisionResult)
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

    internal static class Layers
    {
        public static readonly ObjectLayer NonMoving = 0;
        public static readonly ObjectLayer Moving = 1;
    }

    static class BroadPhaseLayers
    {
        public static readonly BroadPhaseLayer NonMoving = 0;
        public static readonly BroadPhaseLayer Moving = 1;
    }
}
