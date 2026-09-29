// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Physics.Box3DApi;

namespace Alimer.Physics;

public class RigidBody
{
    private readonly List<ColliderShape> _shapes = [];

    internal RigidBody(PhysicsWorld world, in RigidBodySettings settings)
    {
        World = world;
        b3BodyDef bodyDef = settings.ToBox3D();
        ID = b3CreateBody(world.ID, bodyDef);
    }

    public PhysicsWorld World { get; }
    internal b3BodyId ID { get; private set; }
    public bool IsValid => b3Body_IsValid(ID);
    public bool IsEnabled => b3Body_IsEnabled(ID);
    public bool IsAwake => b3Body_IsAwake(ID);

    public void AddShape(ColliderShape shape)
    {
        shape.CreateShape(this);
        _shapes.Add(shape);
    }
}

