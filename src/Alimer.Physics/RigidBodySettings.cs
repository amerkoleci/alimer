// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using static Alimer.Physics.Box3DApi;

namespace Alimer.Physics;

/// <summary>
/// Creation settings for <see cref="RigidBody"/>
/// </summary>
public record struct RigidBodySettings
{
    public RigidBodyType Type;
    public Vector3 Position;
    public Quaternion Rotation;
    public Vector3 LinearVelocity;
    public Vector3 AngularVelocity;

    public RigidBodySettings()
    {
        b3BodyDef defaultBodyDef = b3DefaultBodyDef();
        Type = defaultBodyDef.type.ToRigidBodyType();
        Position = defaultBodyDef.position;
        Rotation = defaultBodyDef.rotation;
        LinearVelocity = defaultBodyDef.linearVelocity;
        AngularVelocity = defaultBodyDef.angularVelocity;
    }

    internal readonly b3BodyDef ToBox3D() => new()
    {
        type = Type.ToBox3D(),
        position = Position,
        rotation = Rotation,
        linearVelocity = LinearVelocity,
        angularVelocity = AngularVelocity,
        internalValue = B3_SECRET_COOKIE
    };
}
