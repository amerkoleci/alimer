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

    public Vector3 ReadVector3(Vector3 defaultValue = default)
    {
        Span<float> values = stackalloc float[3];
        if (ReadValues(values))
        {
            return new Vector3(values);
        }

        return defaultValue;
    }

    public bool ReadBool(string propertyName, bool defaultValue = false)
    {
        if (TryReadProperty(propertyName))
        {
            return ReadBool(defaultValue);
        }

        return defaultValue;
    }

    public int ReadInt32(string propertyName, int defaultValue = 0)
    {
        if (TryReadProperty(propertyName))
        {
            return ReadInt32(defaultValue);
        }

        return defaultValue;
    }

    public string? ReadString(string propertyName, string? defaultValue = default)
    {
        if (TryReadProperty(propertyName))
        {
            return ReadString(defaultValue);
        }

        return defaultValue;
    }

    public Guid ReadGuid(string propertyName, Guid defaultValue = default)
    {
        if (TryReadProperty(propertyName))
        {
            return ReadGuid(defaultValue);
        }

        return defaultValue;
    }

    public TEnum ReadEnum<TEnum>(string propertyName, TEnum defaultValue = default)
        where TEnum : unmanaged, Enum
    {
        if (TryReadProperty(propertyName))
        {
            string? enumString = ReadString();
            if (string.IsNullOrEmpty(enumString))
                return defaultValue;

            if (Enum.TryParse(enumString, out TEnum result))
                return result;
        }

        return defaultValue;
    }

    public Vector3 ReadVector3(string propertyName, Vector3 defaultValue = default)
    {
        if (TryReadProperty(propertyName))
        {
            Span<float> values = stackalloc float[3];
            if (ReadValues(values))
            {
                return new Vector3(values);
            }
        }

        return defaultValue;
    }

    public int ReadVersion(int defaultVersion = 0) => ReadInt32(Serializer.VersionKey, defaultVersion);
    public string ReadType(string? defaultType = default) => ReadString(Serializer.TypeKey, defaultType)!;
}
