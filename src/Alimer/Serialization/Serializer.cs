// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;
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
    public const string TypeKey = "__type";
    public const string VersionKey = "__version";

    public static Serializer CreateJson(Stream stream, JsonWriterOptions options = default) => new JsonEngineSerializer(stream, false, options);
    public static Serializer CreateJson(Stream stream, bool leaveOpen, JsonWriterOptions options = default) => new JsonEngineSerializer(stream, leaveOpen, options);

    public abstract void Dispose();

    public abstract void Write(string propertyName, bool value);
    public abstract void Write(string propertyName, byte value);
    public abstract void Write(string propertyName, sbyte value);
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

    public void Write<TEnum>(string propertyName, scoped in TEnum value)
        where TEnum : struct, Enum
    {
        Write(propertyName, value);
    }

    public void WriteType(string value) => Write(TypeKey, value);
    public void WriteVersion(int value) => Write(VersionKey, value);

    public abstract void BeginObject(string? propertyName = default, string? typeName = default, int? version = default);
    public abstract void EndObject();
    public abstract void BeginArray(string propertyName);
    public abstract void EndArray();
}
