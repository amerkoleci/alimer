// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;
using Alimer.Serialization.Json;

namespace Alimer.Serialization;


/// <summary>
/// Base class for deserializers that can read data from a source and populate objects implementing the <see cref="ISerializable"/> interface.
/// </summary>
public abstract class Deserializer
{
    public static Deserializer CreateJson(string json) => new JsonEngineDeserializer(json);
    public static Deserializer CreateJson(Stream stream) => new JsonEngineDeserializer(stream);

    public abstract bool BeginObject(string? propertyName = default);
    public abstract void EndObject();

    public abstract int BeginArray(string? propertyName = default);
    public abstract void EndArray();

    public abstract bool HasProperty(string propertyName);
    public abstract void ReadProperty(string propertyName);
    public abstract bool TryReadProperty(string propertyName);

    public abstract bool ReadBool(bool defaultValue = false);
    public abstract int ReadInt32(int defaultValue = 0);
    public abstract string? ReadString(string? defaultValue = default);
    public abstract Guid ReadGuid(Guid defaultValue = default);
    public abstract bool ReadValues(scoped Span<float> values);

    public int ReadInt32(string propertyName, int defaultValue = 0)
    {
        ReadProperty(propertyName);
        return ReadInt32(defaultValue);
    }

    public string? ReadString(string propertyName, string? defaultValue = default)
    {
        ReadProperty(propertyName);
        return ReadString(defaultValue);
    }

    public Guid ReadGuid(string propertyName, Guid defaultValue = default)
    {
        ReadProperty(propertyName);
        return ReadGuid(defaultValue);
    }

    public TEnum ReadEnum<TEnum>(string propertyName, TEnum defaultValue = default)
        where TEnum : unmanaged, Enum
    {
        ReadProperty(propertyName);
        string? enumString = ReadString();
        if (string.IsNullOrEmpty(enumString))
            return defaultValue;

        if (Enum.TryParse(enumString, out TEnum result))
            return result;

        return defaultValue;
    }

    public Vector3 ReadVector3(string propertyName, Vector3 defaultValue = default)
    {
        ReadProperty(propertyName);
        Span<float> values = stackalloc float[3];
        if (ReadValues(values))
        {
            return new Vector3(values);
        }

        return defaultValue;
    }

    public int ReadVersion(int defaultVersion = 0) => ReadInt32(Serializer.VersionKey, defaultVersion);
    public string ReadType(string? defaultType = default) => ReadString(Serializer.TypeKey, defaultType)!;
}
