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
    public override void Write(string propertyName, int value)
    {
        //_writer.WriteNumber(propertyName, value);
        _writer.WriteNumberValue(value);
    }

    /// <inheritdoc />
    public override void Write(string propertyName, Guid value)
    {
        //_writer.WriteString(propertyName, value.ToString());
        _writer.WriteStringValue(value.ToString());
    }

    /// <inheritdoc />
    public override IDisposable BeginObject()
    {
        return new ScopedWriteObject(_writer);
    }

    /// <inheritdoc />
    public override IDisposable BeginArray()
    {
        return new ScopedWriteArray(_writer);
    }

    readonly struct ScopedWriteObject : IDisposable
    {
        private readonly Utf8JsonWriter _writer;

        public ScopedWriteObject(Utf8JsonWriter writer)
        {
            _writer = writer;
            _writer.WriteStartObject();
        }

        public void Dispose() => _writer.WriteEndObject();
    }

    readonly struct ScopedWriteArray : IDisposable
    {
        private readonly Utf8JsonWriter _writer;

        public ScopedWriteArray(Utf8JsonWriter writer)
        {
            _writer = writer;
            _writer.WriteStartArray();
        }

        public void Dispose() => _writer.WriteEndArray();
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

    /// <inheritdoc />
    public override ObjectSerializer BeginArray(string? propertyName = default)
    {
        return new JsonObjectSerializer(_writer, true, propertyName);
    }

    /// <inheritdoc />
    public override ObjectSerializer BeginObject(string? propertyName = default)
    {
        return new JsonObjectSerializer(_writer, false, propertyName);
    }
}
