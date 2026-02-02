// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

public interface ISerializer
{
    /// <summary>
    /// Gets the serialization mode.
    /// </summary>
    SerializerMode Mode { get; }

    /// <summary>
    /// Serializes a single <strong>byte</strong> value.
    /// </summary>
    /// <param name="name">The property name to serialize.</param>
    /// <param name="value">The value to serialize</param>
    /// <returns>
    /// <see cref="SerializationResult"/> describing the result of the serialization operation.
    /// </returns>
    public SerializationResult Serialize(string name, ref byte value);

    SerializationResult Serialize<T>(ref T value)
        where T : ISerializable;
}
