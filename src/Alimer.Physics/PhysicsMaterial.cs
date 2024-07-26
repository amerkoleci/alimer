// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if TODO
using BepuPhysics.Constraints;

namespace Alimer.Physics;

public struct PhysicsMaterial
{
    public SpringSettings SpringSettings;
    public float FrictionCoefficient;
    public float MaximumRecoveryVelocity;

    public PhysicsMaterial()
    {
        SpringSettings = new SpringSettings(30, 1);
        FrictionCoefficient = 1.0f;
        MaximumRecoveryVelocity = 2.0f;
    }

    public PhysicsMaterial(SpringSettings springSettings, float frictionCoefficient = 1, float maximumRecoveryVelocity = 2)
    {
        SpringSettings = springSettings;
        FrictionCoefficient = frictionCoefficient;
        MaximumRecoveryVelocity = maximumRecoveryVelocity;
    }

    public PhysicsMaterial(float frictionCoefficient = 1, float maximumRecoveryVelocity = 2)
    {
        SpringSettings = new SpringSettings(30, 1);
        FrictionCoefficient = frictionCoefficient;
        MaximumRecoveryVelocity = maximumRecoveryVelocity;
    }
}

#endif
