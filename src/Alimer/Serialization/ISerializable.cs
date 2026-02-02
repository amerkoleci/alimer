// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

/// <summary>
/// Implement this interface to serialize datas with <see cref="ISerializer"/>.
/// </summary>
public interface ISerializable
{
    /// <summary>
    /// Reads or writes datas from/to the given textual serializer.
    /// </summary>
    /// <param name="serializer">The textual serializer.</param>
    /// <returns></returns>
    SerializationResult Serialize(ISerializer serializer);
}
