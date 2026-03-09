// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;
using System.Text.Json.Serialization;
using Alimer.Serialization.Json;

namespace Alimer.Serialization;

/// <summary>
/// Provides an abstract base class for serializing objects and values using a specified serialization mode.
/// </summary>
/// <remarks>Derived classes must implement the serialization logic for specific types and formats. The class
/// exposes methods for serializing primitive values and objects that implement the ISerializable interface. The Mode
/// property indicates the current serialization mode, which may affect how data is processed.</remarks>
public abstract partial class Serializer : IDisposable
{
    public static Serializer CreateJson(Stream stream, JsonWriterOptions options = default) => new JsonEngineSerializer(stream, options);

    /// <summary>
    /// Serializes a single <strong>byte</strong> value.
    /// </summary>
    /// <param name="name">The property name to serialize.</param>
    /// <param name="value">The value to serialize</param>
    /// <returns>
    /// <see cref="SerializationResult"/> describing the result of the serialization operation.
    /// </returns>
    public abstract SerializationResult Serialize(string name, byte value);

	public abstract SerializationResult BeginObject(string name, string? typeName = default);
    public abstract SerializationResult EndObject();

    public SerializationResult Serialize<T>(T value)
        where T : ISerializable
    {
        if (value is null)
            return SerializationResult.NullValue;

        return value.Serialize(this);
    }

    public abstract void Dispose();
}
