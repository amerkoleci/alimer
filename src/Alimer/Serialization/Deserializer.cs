// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;
using Alimer.Serialization.Json;

namespace Alimer.Serialization;

public abstract class ObjectDeserializer : IDisposable
{
    public abstract void Dispose();

    public abstract bool ReadBool(string propertyName, bool defaultValue = false);
    public abstract int ReadInt32(string propertyName, int defaultValue = 0);
    public abstract string? ReadString(string propertyName, string? defaultValue = default);
    public abstract Guid ReadGuid(string propertyName, Guid defaultValue = default);

    public TEnum ReadEnum<TEnum>(string propertyName, TEnum defaultValue = default)
        where TEnum : struct, Enum
    {
        string? enumString = ReadString(propertyName);
        if (string.IsNullOrEmpty(enumString))
            return defaultValue;

        if (Enum.TryParse(enumString, out TEnum result))
            return result;

        return defaultValue;
    }

    public string ReadType(string? defaultType = default) => ReadString(Serializer.TypeKey, defaultType)!;
    public int ReadVersion(int defaultVersion = 0) => ReadInt32(Serializer.VersionKey, defaultVersion);
}

/// <summary>
/// Base class for deserializers that can read data from a source and populate objects implementing the <see cref="ISerializable"/> interface.
/// </summary>
public abstract class Deserializer
{
    public static Deserializer CreateJson(string json) => new JsonEngineDeserializer(json);
    public static Deserializer CreateJson(Stream stream) => new JsonEngineDeserializer(stream);

    public abstract ObjectDeserializer BeginObject();

    public abstract bool BeginObject(string? propertyName = default);

    public abstract int BeginArray(string propertyName);
    public abstract void EndArray();
}
