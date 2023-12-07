// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;

namespace Alimer.Engine;

[DataContract(Name = nameof(CameraComponent))]
[DefaultEntitySystem(typeof(CameraSystem))]
public sealed class CameraComponent : EntityComponent
{
    [IgnoreDataMember]
    public Matrix4x4 ViewMatrix { get; set; } = Matrix4x4.Identity;

    [IgnoreDataMember]
    public Matrix4x4 ProjectionMatrix { get; set; } = Matrix4x4.Identity;

    [IgnoreDataMember]
    public Matrix4x4 ViewProjectionMatrix { get; set; } = Matrix4x4.Identity;

    public float AspectRatio { get; set; } = 16.0f / 9.0f;

    public float FieldOfView { get; set; } = 60.0f;

    public float FarPlaneDistance { get; set; } = 100000.0f;

    public float NearPlaneDistance { get; set; } = 0.1f;

    public bool UseCustomAspectRatio { get; set; }

    public void Update(float? screenAspectRatio = null)
    {
        if (Entity is null)
            return;

        float aspectRatio = UseCustomAspectRatio ? AspectRatio : screenAspectRatio ?? AspectRatio;

        Matrix4x4.Decompose(Entity.Transform.WorldMatrix, out _,
            out Quaternion rotation,
            out Vector3 translation);

        Vector3 forwardVector = Vector3.Normalize(Vector3.Transform(-Vector3.UnitZ, rotation));
        Vector3 upVector = Vector3.Normalize(Vector3.Transform(Vector3.UnitY, rotation));
        ViewMatrix = Matrix4x4.CreateLookAt(translation, translation + forwardVector, upVector);

        ProjectionMatrix = Matrix4x4.CreatePerspectiveFieldOfView(
            FieldOfView * (float)(Math.PI / 180.0f),
            aspectRatio,
            NearPlaneDistance,
            FarPlaneDistance);

        ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
    }
}
