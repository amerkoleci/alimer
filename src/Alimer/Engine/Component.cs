// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Serialization;

namespace Alimer.Engine;

/// <summary>
/// Defines a component that can be attached to an <see cref="Entity"/>.
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
        serializer.Write(Keys.IsEnabled, IsEnabled);

        OnSerialize(serializer);
    }

    public void Deserialize(Deserializer deserializer)
    {
        //IsEnabled = deserializer.ReadInt32(Keys.IsEnabled, IsEnabled ? 1 : 0) != 0;
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
