// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using static Alimer.Physics.Box3DApi;

namespace Alimer.Physics;

public class BoxColliderShape : ColliderShape
{
    public BoxColliderShape(in Vector3 size)
    {
        Size = size;
    }

    public Vector3 Size { get; }
    public Vector3 HalfSize => Size * 0.5f;

    public override unsafe void CreateShape(RigidBody body)
    {
        b3BoxHull boxHull = b3MakeBoxHull(HalfSize.X, HalfSize.Y, HalfSize.Z);

        b3ShapeDef shapeDef = b3DefaultShapeDef();
        Debug.Assert(shapeDef.internalValue == B3_SECRET_COOKIE);
        ID = b3CreateHullShape(body.ID, shapeDef, &boxHull.@base);
    }
}
