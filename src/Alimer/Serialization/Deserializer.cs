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
    public static Deserializer CreateJson(Stream stream) => new JsonEngineDeserializer(stream);

    public abstract void Dispose();

    public string ReadType(string? defaultType = default) => ReadString(Serializer.TypeKey, defaultType)!;
    public int ReadVersion(int defaultVersion = 0) => ReadInt32(Serializer.VersionKey, defaultVersion);

    public abstract int ReadInt32(string propertyName, int defaultValue = 0);
    public abstract string? ReadString(string propertyName, string? defaultValue = default);
    public abstract Guid ReadGuid(string propertyName, Guid defaultValue = default);

    public abstract bool BeginObject(string? propertyName = default);
    public abstract void EndObject();

    public abstract int BeginArray(string propertyName);
    public abstract void EndArray();

    //public abstract SerializationResult BeginObject(string name, string? typeName = default);
    //public abstract SerializationResult EndObject();
}
