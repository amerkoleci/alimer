// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Engine;
using JoltPhysicsSharp;

namespace Alimer.Physics;

public class PhysicsSystem : EntitySystem<PhysicsComponent>, IDisposable
{
    private const uint MaxBodies = 1024;
    private const uint NumBodyMutexes = 0;
    private const uint MaxBodyPairs = 1024;
    private const uint MaxContactConstraints = 1024;

    private readonly TempAllocator _tempAllocator;
    private readonly JobSystemThreadPool _jobSystem;
    private readonly BPLayerInterfaceImpl _broadPhaseLayer;
    private readonly ObjectVsBroadPhaseLayerFilterImpl _objectVsBroadphaseLayerFilter;
    private readonly ObjectLayerPairFilterImpl _objectVsObjectLayerFilter;
    private readonly JoltPhysicsSharp.PhysicsSystem _joltPhysicsSystem;

    static PhysicsSystem()
    {
        Log.ErrorIf(!Foundation.Init(), "Jolt Physics: Failed to initialized");
    }

    public PhysicsSystem()
        : base(typeof(TransformComponent))
    {
        _tempAllocator = new(10 * 1024 * 1024);
        _jobSystem = new(Foundation.MaxPhysicsJobs, Foundation.MaxPhysicsBarriers);
        _broadPhaseLayer = new();
        _objectVsBroadphaseLayerFilter = new();
        _objectVsObjectLayerFilter = new();

        _joltPhysicsSystem = new();
        _joltPhysicsSystem.Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints,
            _broadPhaseLayer,
            _objectVsBroadphaseLayerFilter,
            _objectVsObjectLayerFilter);
    }

    public void Dispose()
    {
        _joltPhysicsSystem.Dispose();
        _objectVsObjectLayerFilter.Dispose();
        _objectVsBroadphaseLayerFilter.Dispose();
        _broadPhaseLayer.Dispose();
        _jobSystem.Dispose();
        _tempAllocator.Dispose();

        Foundation.Shutdown();
    }

    public override void Update(AppTime time)
    {
        base.Update(time);
    }

    #region Nested
    static class Layers
    {
        public const byte NonMoving = 0;
        public const byte Moving = 1;
        public const int NumLayers = 2;
    }

    static class BroadPhaseLayers
    {
        public const byte NonMoving = 0;
        public const byte Moving = 1;
        public const int NumLayers = 2;
    }

    class BPLayerInterfaceImpl : BroadPhaseLayerInterface
    {
        private readonly BroadPhaseLayer[] _objectToBroadPhase = new BroadPhaseLayer[Layers.NumLayers];

        public BPLayerInterfaceImpl()
        {
            // Create a mapping table from object to broad phase layer
            _objectToBroadPhase[Layers.NonMoving] = BroadPhaseLayers.NonMoving;
            _objectToBroadPhase[Layers.Moving] = BroadPhaseLayers.Moving;
        }

        protected override int GetNumBroadPhaseLayers()
        {
            return BroadPhaseLayers.NumLayers;
        }

        protected override BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer layer)
        {
            Debug.Assert(layer < Layers.NumLayers);

            return _objectToBroadPhase[layer];
        }

        protected override string GetBroadPhaseLayerName(BroadPhaseLayer layer)
        {
            switch ((byte)layer)
            {
                case BroadPhaseLayers.NonMoving: return "NON_MOVING";
                case BroadPhaseLayers.Moving: return "MOVING";
                default:
                    Debug.Assert(false);
                    return "INVALID";
            }
        }
    }

    class ObjectVsBroadPhaseLayerFilterImpl : ObjectVsBroadPhaseLayerFilter
    {
        protected override bool ShouldCollide(ObjectLayer layer1, BroadPhaseLayer layer2)
        {
            switch (layer1)
            {
                case Layers.NonMoving:
                    return layer2 == BroadPhaseLayers.Moving;
                case Layers.Moving:
                    return true;
                default:
                    Debug.Assert(false);
                    return false;
            }
        }
    }

    class ObjectLayerPairFilterImpl : ObjectLayerPairFilter
    {
        protected override bool ShouldCollide(ObjectLayer object1, ObjectLayer object2)
        {
            switch (object1)
            {
                case Layers.NonMoving:
                    return object2 == Layers.Moving;
                case Layers.Moving:
                    return true;
                default:
                    Debug.Assert(false);
                    return false;
            }
        }
    }
    #endregion
}
