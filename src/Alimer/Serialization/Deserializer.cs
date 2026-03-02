// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;
using Alimer.Serialization.Json;

namespace Alimer.Serialization;

/// <summary>
/// Base class for deserializers that can read data from a source and populate objects implementing the <see cref="ISerializable"/> interface.
/// </summary>
public abstract class Deserializer : IDisposable
{
    public static Deserializer CreateJson(Stream stream, JsonReaderOptions options = default) => new JsonEngineDeserializer(stream, options);

    public abstract void Dispose();
    public abstract byte Deserialize(string name);

    public abstract SerializationResult BeginObject(string name, string? typeName = default);
    public abstract SerializationResult EndObject();
}
