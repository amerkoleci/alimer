// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using JoltPhysicsSharp;

namespace Alimer.Physics;

public class BoxColliderShape : ColliderShape
{
    public BoxColliderShape(in Vector3 size)
    {
        Size = size;

        Handle = new BoxShape(size / 2.0f);
    }

    public Vector3 Size { get; }

    internal override Shape Handle { get; }
}
