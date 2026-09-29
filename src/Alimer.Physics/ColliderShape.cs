// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Physics.Box3DApi;
namespace Alimer.Physics;

public abstract class ColliderShape
{
    internal b3ShapeId ID { get; set; }
    public bool IsValid => b3Shape_IsValid(ID);

    public abstract void CreateShape(RigidBody body);
}
