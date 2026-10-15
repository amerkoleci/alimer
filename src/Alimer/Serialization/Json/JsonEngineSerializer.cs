// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text;
using System.Text.Json;

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

    /// <inheritdoc />
    public override void WritePropertyName(string propertyName)
    {
        _writer.WritePropertyName(propertyName);
    }

    /// <inheritdoc />
    public override void WriteBooleanValue(bool value)
    {
        _writer.WriteBooleanValue(value);
    }

    /// <inheritdoc />
    public override void WriteNumberValue(byte value)
    {
        _writer.WriteNumberValue(value);
    }

    /// <inheritdoc />
    public override void WriteNumberValue(sbyte value)
    {
        _writer.WriteNumberValue(value);
    }

    /// <inheritdoc />
    public override void WriteNumberValue(int value)
    {
        _writer.WriteNumberValue(value);
    }

    /// <inheritdoc />
    public override void WriteStringValue(string value)
    {
        _writer.WriteStringValue(value);
    }

    /// <inheritdoc />
    public override void WriteGuidValue(Guid value)
    {
        _writer.WriteStringValue(value);
    }

    /// <inheritdoc />
    public override void WriteBase64StringValue(scoped ReadOnlySpan<byte> values)
    {
        _writer.WriteBase64StringValue(values);
    }

    /// <inheritdoc />
    public override void WriteValues(scoped ReadOnlySpan<float> values)
    {
        StringBuilder result = new();
        result.Append("[ ");
        result.Append($"{values[0]}");
        for (int i = 1; i < values.Length; i++)
            result.Append($", {values[i]}");
        result.Append(" ]");
        _writer.WriteRawValue(result.ToString());
    }

    /// <inheritdoc />
    public override void BeginObject(string? propertyName = default)
    {
        _writer.WriteStartObject();
    }

    /// <inheritdoc />
    public override void EndObject()
    {
        _writer.WriteEndObject();
    }

    /// <inheritdoc />
    public override void BeginArray(string? propertyName = default)
    {
        if (string.IsNullOrEmpty(propertyName))
        {
            _writer.WriteStartArray();
        }
        else
        {
            _writer.WriteStartArray(propertyName);
        }
    }

    /// <inheritdoc />
    public override void EndArray()
    {
        _writer.WriteEndArray();
    }
}

class JsonObjectSerializer : ObjectSerializer
{
    private Utf8JsonWriter _writer;
    private readonly bool _isArray;

    public JsonObjectSerializer(Utf8JsonWriter writer, bool isArray, string? propertyName = default)
    {
        _writer = writer;
        _isArray = isArray;

        if (isArray)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                _writer.WriteStartArray();
            }
            else
            {
                _writer.WriteStartArray(propertyName);
            }
        }
        else
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                _writer.WriteStartObject();
            }
            else
            {
                _writer.WriteStartObject(propertyName);
            }
        }
    }

    public override void Dispose()
    {
        if (_isArray)
        {
            _writer.WriteEndArray();
        }
        else
        {
            _writer.WriteEndObject();
        }
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
    public override void Write(string propertyName, scoped ReadOnlySpan<float> values)
    {
        var result = new StringBuilder();
        result.Append("[ ");
        result.Append($"{values[0]}");
        for (int i = 1; i < values.Length; i++)
            result.Append($", {values[i]}");
        result.Append(" ]");
        _writer.WritePropertyName(propertyName);
        _writer.WriteRawValue(result.ToString());
    }
}
