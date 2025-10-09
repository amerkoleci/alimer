// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

[DataContract(Name = nameof(CameraComponent))]
[DefaultEntitySystem(typeof(CameraSystem))]
public partial class CameraComponent : EntityComponent
{
    [IgnoreDataMember]
    [JsonIgnore]
    public Matrix4x4 ViewMatrix { get; set; } = Matrix4x4.Identity;

    [IgnoreDataMember]
    [JsonIgnore]
    public Matrix4x4 ProjectionMatrix { get; set; } = Matrix4x4.Identity;

    [IgnoreDataMember]
    [JsonIgnore]
    public Matrix4x4 ViewProjectionMatrix { get; set; } = Matrix4x4.Identity;

    [JsonPropertyOrder(10)]
    public float AspectRatio { get; set; } = 16.0f / 9.0f;

    [JsonPropertyOrder(20)]
    public float FieldOfView { get; set; } = 60.0f;

    [JsonPropertyOrder(30)]
    public float FarPlaneDistance { get; set; } = 100000.0f;

    [JsonPropertyOrder(40)]
    public float NearPlaneDistance { get; set; } = 0.1f;

    [JsonPropertyOrder(50)]
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
