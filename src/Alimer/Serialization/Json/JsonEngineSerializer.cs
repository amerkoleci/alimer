// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.Json.Serialization.Metadata;
using Alimer.Rendering;

namespace Alimer.Serialization.Json;

/// <summary>
/// Class that provides JSON serialization methods.
/// </summary>
internal partial class JsonEngineSerializer : Serializer, IDisposable
{
    private readonly Stream _stream;
    private readonly Utf8JsonWriter _writer;
    private readonly bool _leaveOpen;
    private volatile uint _isDisposed;

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    /// <param name="leaveOpen">Whether to disposeStream.</param>
    /// <param name="options">Optional JSON writer options.</param>
    public JsonEngineSerializer(Stream stream, bool leaveOpen, JsonWriterOptions options = default)
    {
        _stream = stream;
        _leaveOpen = leaveOpen;

        _writer = new Utf8JsonWriter(stream, options);
        _writer.WriteStartObject();
    }

    ~JsonEngineSerializer()
    {
        Dispose(false);
    }

    /// <inheritdoc />
    public override void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    private void Dispose(bool disposing)
    {
        if (Interlocked.Exchange(ref _isDisposed, 1) is not 0)
        {
            return;
        }

        if (disposing)
        {
            _writer.WriteEndObject();
            _writer.Flush();
            _writer.Dispose();

            if (!_leaveOpen)
            {
                _stream.Close();
            }
        }
    }

    /// <summary>
    /// Flushes any buffered data to the underlying stream (write mode only).
    /// </summary>
    public void Flush()
    {
        _writer.Flush();
    }

    ///// <inheritdoc />
    //public override IObjectSerializer Serialize(string name, byte value)
    //{
    //    _writer.WriteNumber(name, value);
    //    return SerializationResult.Ok;
    //}

    /// <inheritdoc />
    public override void Write(string propertyName, bool value)
    {
        _writer.WriteBoolean(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, byte value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, sbyte value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, short value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, ushort value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, int value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, uint value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, long value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, ulong value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, float value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, double value)
    {
        _writer.WriteNumber(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, string value)
    {
        _writer.WriteString(propertyName, value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, Guid value)
    {
        _writer.WriteString(propertyName, value.ToString());
    }

    /// <inheritdoc />
    public override void BeginObject(string? propertyName, string? typeName = default, int? version = default)
    {
        if (string.IsNullOrEmpty(propertyName))
        {
            _writer.WriteStartObject();
        }
        else
        {
            _writer.WriteStartObject(propertyName);
        }

        if (!string.IsNullOrEmpty(typeName))
        {
            _writer.WriteString(TypeKey, typeName);
        }

        if (version.HasValue)
        {
            _writer.WriteNumber(VersionKey, version.Value);
        }
    }

    /// <inheritdoc />
    public override void EndObject()
    {
        //JsonMetadataServices.GetEnumConverter<>.Write()

        _writer.WriteEndObject();
    }

    public override void BeginArray(string propertyName)
    {
        _writer.WriteStartArray(propertyName);
    }

    public override void EndArray()
    {
        _writer.WriteEndArray();
    }
}
