// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Serialization;

namespace Alimer.Engine;

/// <summary>
/// Defines a component that can be attached to an <see cref="Alimer.Engine.Entity"/>.
/// </summary>
[Meta]
public abstract partial class Component : ISerializable
{
    /// <summary>
	/// The version of the component.
	/// </summary>
    public virtual int ComponentVersion => 0;

    public Entity? Entity { get; internal set; }

    public virtual bool IsEnabled { get; set; } = true;

    public void Serialize(Serializer serializer)
    {
        string componentTypeName = GetType().Name;
        //using ObjectSerializer objectSerializer = serializer.BeginObject();
        //serializer.WriteType(componentTypeName);
        //serializer.WriteVersion(ComponentVersion);
        //serializer.Write(Keys.IsEnabled, IsEnabled);
        //OnSerialize(serializer);
    }

    public void Deserialize(ObjectDeserializer deserializer)
    {
        //IsEnabled = deserializer.ReadBool(Keys.IsEnabled, IsEnabled);
    }

    protected virtual void OnSerialize(ObjectSerializer serializer)
    {

    }

    internal static class Keys
    {
        public const string Id = "Id";
        public const string IsEnabled = nameof(Component.IsEnabled);
    }
}
