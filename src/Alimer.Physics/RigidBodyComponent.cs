// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using BepuPhysics;

namespace Alimer.Physics;

public class RigidBodyComponent : PhysicsComponent
{
    internal BodyHandle Handle { get; private set; }
}
