// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
namespace Alimer.Physics;

public abstract class ColliderShape
{
    internal abstract PhysicsShape Handle { get; }
}
