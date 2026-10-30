// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Serialization;

namespace Alimer.Engine;

/// <summary>
/// Defines a component that can be attached to an <see cref="Engine.Entity"/>.
/// </summary>
[Meta]
public abstract partial class Component : ISerializable, ITypeMetadataProvider
{
    /// <summary>
	/// The version of the component.
	/// </summary>
    public virtual int ComponentVersion => 0;

    //[IgnoreDataMember]
    public Entity? Entity { get; internal set; }

    public virtual bool IsEnabled { get; set; } = true;

    public static ITypeMetadata TypeMetadata => MetadataRegistry.GetMetadata<Component, IObjectTypeMetadata>();

    public void Serialize(Serializer serializer)
    {
        string componentTypeName = GetType().Name;
        serializer.WriteType(componentTypeName);
        serializer.WriteVersion(ComponentVersion);
        serializer.Write(Keys.IsEnabled, IsEnabled);
        OnSerialize(serializer);
    }

    public void Deserialize(Deserializer deserializer)
    {
        int version = deserializer.ReadVersion();
        IsEnabled = deserializer.ReadBool(Keys.IsEnabled, IsEnabled);
    }

    protected virtual void OnSerialize(Serializer serializer)
    {

    }

    internal static class Keys
    {
        public const string Id = "Id";
        public const string IsEnabled = nameof(Component.IsEnabled);
    }
}
