// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.Serialization;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

[JsonPolymorphic(TypeDiscriminatorPropertyName = "__type")]
[JsonDerivedType(typeof(TransformComponent), "TransformComponent")]
[JsonDerivedType(typeof(CameraComponent), "CameraComponent")]
public abstract class EntityComponent
{
    [IgnoreDataMember]
    [JsonIgnore]
    public Entity? Entity { get; internal set; }

    [JsonPropertyName("Enabled")]
    [JsonPropertyOrder(-10)]
    public virtual bool IsEnabled { get; set; } = true;
}
