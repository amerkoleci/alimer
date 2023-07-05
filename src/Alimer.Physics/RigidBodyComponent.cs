// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using JoltPhysicsSharp;

namespace Alimer.Physics;

public class RigidBodyComponent : PhysicsComponent
{
    private Body? _joltBody;
}
