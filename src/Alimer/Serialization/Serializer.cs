// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using System.Text.Json;
using Alimer.Serialization.Json;

namespace Alimer.Serialization;

public abstract class ObjectSerializer : IDisposable
{
    public abstract void Dispose();

    public abstract void Write(string propertyName, short value);
    public abstract void Write(string propertyName, ushort value);
    public abstract void Write(string propertyName, int value);
    public abstract void Write(string propertyName, uint value);
    public abstract void Write(string propertyName, long value);
    public abstract void Write(string propertyName, ulong value);
    public abstract void Write(string propertyName, float value);
    public abstract void Write(string propertyName, double value);
    public abstract void Write(string propertyName, string value);
    public abstract void Write(string propertyName, Guid value);
    public abstract void Write(string propertyName, scoped ReadOnlySpan<float> values);

}

/// <summary>
/// Provides an abstract base class for serializing objects and values using a specified serialization mode.
/// </summary>
/// <remarks>Derived classes must implement the serialization logic for specific types and formats. The class
/// exposes methods for serializing primitive values and objects that implement the ISerializable interface. The Mode
/// property indicates the current serialization mode, which may affect how data is processed.</remarks>
public abstract partial class Serializer : IDisposable
{
    public const string TypeKey = "__type";
    public const string VersionKey = "__version";

    public static Serializer CreateJson(Stream stream, JsonWriterOptions options = default) => new JsonEngineSerializer(stream, false, options);
    public static Serializer CreateJson(Stream stream, bool leaveOpen, JsonWriterOptions options = default) => new JsonEngineSerializer(stream, leaveOpen, options);

    public abstract void Dispose();

    public abstract void BeginObject(string? propertyName = default);
    public abstract void EndObject();
    public abstract void BeginArray(string? propertyName = default);
    public abstract void EndArray();

    public abstract void WritePropertyName(string propertyName);
    public abstract void WriteBooleanValue(bool value);
    public abstract void WriteNumberValue(byte value);
    public abstract void WriteNumberValue(sbyte value);
    public abstract void WriteNumberValue(int value);
    public abstract void WriteStringValue(string value);
    public abstract void WriteGuidValue(Guid value);
    public abstract void WriteBase64StringValue(scoped ReadOnlySpan<byte> values);
    public abstract void WriteValues(scoped ReadOnlySpan<float> values);

    public void WriteVersion(int value) => Write(VersionKey, value);
    public void WriteType(string value) => Write(TypeKey, value);

    public void Write(string propertyName, bool value)
    {
        WritePropertyName(propertyName);
        WriteBooleanValue(value);
    }

    public void Write(string propertyName, byte value)
    {
        WritePropertyName(propertyName);
        WriteNumberValue(value);
    }

    public void Write(string propertyName, sbyte value)
    {
        WritePropertyName(propertyName);
        WriteNumberValue(value);
    }

    public void Write(string propertyName, int value)
    {
        WritePropertyName(propertyName);
        WriteNumberValue(value);
    }

    public void Write(string propertyName, Guid value)
    {
        WritePropertyName(propertyName);
        WriteGuidValue(value);
    }

    public void Write(string propertyName, string value)
    {
        WritePropertyName(propertyName);
        WriteStringValue(value);
    }

    public void Write<TEnum>(string propertyName, scoped in TEnum value)
        where TEnum : unmanaged, Enum
    {
        WritePropertyName(propertyName);
        WriteStringValue(value.ToString());
    }

    public void Write(string propertyName, Vector2 value)
    {
        ReadOnlySpan<float> values = MemoryMarshal.Cast<Vector2, float>([value]);
        WritePropertyName(propertyName);
        WriteValues(values);
    }

    public void Write(string propertyName, Vector3 value)
    {
        ReadOnlySpan<float> values = MemoryMarshal.Cast<Vector3, float>([value]);
        WritePropertyName(propertyName);
        WriteValues(values);
    }

    public void Write(string propertyName, Vector4 value)
    {
        ReadOnlySpan<float> values = MemoryMarshal.Cast<Vector4, float>([value]);
        WritePropertyName(propertyName);
        WriteValues(values);
    }

    public void Write(string propertyName, Quaternion value)
    {
        ReadOnlySpan<float> values = MemoryMarshal.Cast<Quaternion, float>([value]);
        WritePropertyName(propertyName);
        WriteValues(values);
    }
}
