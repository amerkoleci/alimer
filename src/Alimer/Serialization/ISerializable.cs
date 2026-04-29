// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

/// <summary>
/// Base interface that supports serialization and deserialization using <see cref="Serializer"/> and <see cref="Deserializer"/>.
/// </summary>
public interface ISerializable
{
    void Serialize(Serializer serializer);
    void Deserialize(ObjectDeserializer deserializer);
}
