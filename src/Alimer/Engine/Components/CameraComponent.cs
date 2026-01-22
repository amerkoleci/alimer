// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.ComponentModel.DataAnnotations;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

[DataContract(Name = nameof(CameraComponent))]
[Category("Rendering")]
[DisplayName("Camera")]
[Description("Camera component.")]
//[Icon("camera")]
[DefaultEntitySystem(typeof(CameraSystem))]
public partial class CameraComponent : EntityComponent
{
    public const float DefaultFieldOfView = 60.0f;

    [JsonPropertyOrder(10)]
    [DataMember]
    public float AspectRatio { get; set; } = 16.0f / 9.0f;

    [JsonPropertyOrder(20)]
    [DataMember]
    [DefaultValue(DefaultFieldOfView)]
    [DisplayName("Field of view")]
    //[DataMemberRange(1.0, 179.0, 1.0, 10.0, 0)]
    public float FieldOfView { get; set; } = DefaultFieldOfView;

    [JsonPropertyOrder(30)]
    public float FarPlaneDistance { get; set; } = 100000.0f;

    [JsonPropertyOrder(40)]
    public float NearPlaneDistance { get; set; } = 0.1f;

    [JsonPropertyOrder(50)]
    public bool UseCustomAspectRatio { get; set; }

    [IgnoreDataMember]
    [JsonIgnore]
    public Matrix4x4 ViewMatrix { get; set; } = Matrix4x4.Identity;

    [IgnoreDataMember]
    [JsonIgnore]
    public Matrix4x4 ProjectionMatrix { get; set; } = Matrix4x4.Identity;

    [IgnoreDataMember]
    [JsonIgnore]
    public Matrix4x4 ViewProjectionMatrix { get; set; } = Matrix4x4.Identity;


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
            FieldOfView.ToRadians(),
            aspectRatio,
            NearPlaneDistance,
            FarPlaneDistance);

        ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
    }
}
