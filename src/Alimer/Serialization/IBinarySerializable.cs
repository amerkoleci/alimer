// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

/// <summary>
/// Implement this interface to serialize datas with <see cref="BinarySerializer"/>.
/// </summary>
public interface IBinarySerializable
{
    /// <summary>
    /// Reads or writes datas from/to the given binary serializer.
    /// </summary>
    /// <param name="serializer">The binary serializer.</param>
    void Serialize(BinarySerializer serializer);
}
