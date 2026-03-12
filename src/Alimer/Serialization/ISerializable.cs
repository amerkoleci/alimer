// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

public interface ISerializable
{
    void Serialize(ObjectSerializer serializer);
    void Deserialize(ObjectDeserializer deserializer);
}
