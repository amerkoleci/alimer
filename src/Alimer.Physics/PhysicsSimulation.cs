// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.CompilerServices;
using BepuPhysics;
using BepuPhysics.Collidables;
using BepuPhysics.CollisionDetection;
using BepuPhysics.Constraints;
using BepuUtilities;
using BepuUtilities.Memory;

namespace Alimer.Physics;

public sealed class PhysicsSimulation : DisposableObject
{
    private readonly ThreadDispatcher _dispatcher;
    private readonly BufferPool _bufferPool;
    private readonly NarrowPhaseCallbacks _narrowPhaseCallbacks;
    private readonly PoseIntegratorCallbacks _poseIntegratorCallbacks;
    private readonly SolveDescription _solveDescription;

    private readonly CollidableProperty<PhysicsMaterial> _materials;

    public PhysicsSimulation()
    {
        int targetThreadCount = Math.Max(1, Environment.ProcessorCount > 4 ? Environment.ProcessorCount - 2 : Environment.ProcessorCount - 1);

        _dispatcher = new(targetThreadCount);
        _bufferPool = new();
        _materials = new(_bufferPool);
        _narrowPhaseCallbacks = new(_materials);
        _poseIntegratorCallbacks = new (new Vector3(0.0f, -9.81f, 0.0f));
        _solveDescription = new SolveDescription(1, 1);

        InternalSimulation = Simulation.Create(_bufferPool, _narrowPhaseCallbacks, _poseIntegratorCallbacks, _solveDescription);
    }

    internal Simulation InternalSimulation { get; }
    public CollidableProperty<PhysicsMaterial> Materials => _materials;

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _materials.Dispose();
            _narrowPhaseCallbacks.Dispose();
            _bufferPool.Clear();
            _dispatcher.Dispose();
            InternalSimulation.Dispose();
        }
    }

    public void Timestep(TimeSpan deltaTime)
    {
        InternalSimulation.Timestep((float)deltaTime.TotalSeconds);
    }

    #region Nested
    private struct NarrowPhaseCallbacks(CollidableProperty<PhysicsMaterial> materials) : INarrowPhaseCallbacks
    {
        public CollidableProperty<PhysicsMaterial> Materials = materials;

        public readonly void Initialize(Simulation simulation)
        {
            Materials.Initialize(simulation);
        }

        public readonly void Dispose()
        {
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly bool AllowContactGeneration(int workerIndex, CollidableReference a, CollidableReference b, ref float speculativeMargin)
        {
            return a.Mobility == CollidableMobility.Dynamic || b.Mobility == CollidableMobility.Dynamic;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly bool AllowContactGeneration(int workerIndex, CollidablePair pair, int childIndexA, int childIndexB)
        {
            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly bool ConfigureContactManifold<TManifold>(int workerIndex, CollidablePair pair, ref TManifold manifold, out PairMaterialProperties pairMaterial) where TManifold : unmanaged, IContactManifold<TManifold>
        {
            PhysicsMaterial a = Materials[pair.A];
            PhysicsMaterial b = Materials[pair.B];
            pairMaterial.FrictionCoefficient = a.FrictionCoefficient * b.FrictionCoefficient;
            pairMaterial.MaximumRecoveryVelocity = MathF.Max(a.MaximumRecoveryVelocity, b.MaximumRecoveryVelocity);
            pairMaterial.SpringSettings = pairMaterial.MaximumRecoveryVelocity == a.MaximumRecoveryVelocity ? a.SpringSettings : b.SpringSettings;
            //Characters.TryReportContacts(pair, ref manifold, workerIndex, ref pairMaterial);
            //Events.HandleManifold(workerIndex, pair, ref manifold);

            pairMaterial.FrictionCoefficient = 1.0f;
            pairMaterial.MaximumRecoveryVelocity = 2.0f;
            pairMaterial.SpringSettings = new SpringSettings(30.0f, 1.0f);

            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly bool ConfigureContactManifold(int workerIndex, CollidablePair pair, int childIndexA, int childIndexB, ref ConvexContactManifold manifold)
        {
            return true;
        }
    }

    private struct PoseIntegratorCallbacks(Vector3 gravity) : IPoseIntegratorCallbacks
    {
        //Note that velocity integration uses "wide" types. These are array-of-struct-of-arrays types that use SIMD accelerated types underneath.
        //Rather than handling a single body at a time, the callback handles up to Vector<float>.Count bodies simultaneously.
        private Vector3Wide _gravityWideDt;

        public readonly AngularIntegrationMode AngularIntegrationMode => AngularIntegrationMode.Nonconserving;
        public bool AllowSubstepsForUnconstrainedBodies { get; }
        public bool IntegrateVelocityForKinematics { get; }
        public Vector3 Gravity { get; set; } = gravity;

        public void Initialize(Simulation simulation)
        {

        }

        public void PrepareForIntegration(float dt)
        {
            _gravityWideDt = Vector3Wide.Broadcast(Gravity * dt);
        }

        /// <summary>
        /// Callback called for each active body within the simulation during body integration.
        /// </summary>
        /// <param dbgName="bodyIndex">Index of the body being visited.</param>
        /// <param dbgName="pose">Body's current pose.</param>
        /// <param dbgName="localInertia">Body's current local inertia.</param>
        /// <param dbgName="workerIndex">Index of the worker thread processing this body.</param>
        /// <param dbgName="velocity">Reference to the body's current velocity to integrate.</param>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public readonly void IntegrateVelocity(Vector<int> bodyIndices, Vector3Wide position, QuaternionWide orientation, BodyInertiaWide localInertia, Vector<int> integrationMask, int workerIndex, Vector<float> dt, ref BodyVelocityWide velocity)
        {
            // This also is a handy spot to implement things like position dependent gravity or per-body damping.
            // We don't have to check for kinematics; IntegrateVelocityForKinematics returns false in this type, so we'll never see them in this callback.
            // Note that these are SIMD operations and "Wide" types. There are Vector<float>.Count lanes of execution being evaluated simultaneously.
            // The types are laid out in array-of-structures-of-arrays (AOSOA) format. That's because this function is frequently called from vectorized contexts within the solver.
            // Transforming to "array of structures" (AOS) format for the callback and then back to AOSOA would involve a lot of overhead, so instead the callback works on the AOSOA representation directly.
            velocity.Linear += _gravityWideDt;
        }
    }
    #endregion
}
