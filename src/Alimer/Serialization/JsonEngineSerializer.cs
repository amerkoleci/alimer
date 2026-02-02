// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Alimer.Serialization;
/// <summary>
/// Class that provides JSON serialization methods for types implementing the <see cref="ISerializable"/>.
/// </summary>
public partial class JsonEngineSerializer : ISerializer, IDisposable
{
    private readonly Stream _stream;
    private Utf8JsonWriter? _writer;
    private byte[]? _buffer;
    private int _readerPosition;

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    /// <param name="mode">The read or write mode.</param>
    /// <param name="options">Optional JSON writer options.</param>
    public JsonEngineSerializer(Stream stream, SerializerMode mode, JsonWriterOptions options = default)
    {
        _stream = stream;
        Mode = mode;

        if (mode == SerializerMode.Write)
        {
            _writer = new Utf8JsonWriter(stream, options);
            _writer.WriteStartObject();
        }
        else
        {
            // Read the entire stream into a buffer for Utf8JsonReader
            if (stream.CanSeek)
            {
                int length = (int)stream.Length;
                _buffer = new byte[length];
                stream.ReadExactly(_buffer);
            }
            else
            {
                using MemoryStream ms = new();
                stream.CopyTo(ms);
                _buffer = ms.ToArray();
            }

            _readerPosition = 0;
        }
    }

    /// <inheritdoc />
    public SerializerMode Mode { get; }

    /// <summary>
    /// Gets a value indicating whether the serializer is in read mode.
    /// </summary>
    public bool IsReading => Mode == SerializerMode.Read;

    /// <inheritdoc/>
    public void Dispose()
    {
        _writer?.Dispose();
    }

    /// <summary>
    /// Flushes any buffered data to the underlying stream (write mode only).
    /// </summary>
    public void Flush()
    {
        if (Mode == SerializerMode.Write)
        {
            _writer?.Flush();
        }
    }

    /// <summary>
    /// Serializes a single <strong>byte</strong> value.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public SerializationResult Serialize(string name, ref byte value)
    {
        if (Mode == SerializerMode.Write)
        {
            _writer!.WritePropertyName(name);
            _writer!.WriteNumberValue(value);
        }
        else
        {
            ReadOnlySpan<byte> span = _buffer.AsSpan(_readerPosition);
            Utf8JsonReader reader = new(span);
            reader.Read();
            value = reader.GetByte();
            _readerPosition += (int)reader.BytesConsumed;
        }

        return SerializationResult.Ok;
    }

    /// <summary>
    /// Serializes a single object implementing <see cref="ISerializable"/>.
    /// </summary>
    /// <param name="value">The value to serialize</param>
    /// <remarks>
    /// Note that depending on the serialization <see cref="Mode"/>, this method reads or writes the value.
    /// </remarks>
    public SerializationResult Serialize<T>(ref T value)
        where T : ISerializable
    {
        return value.Serialize(this);
    }

    ref struct JsonWriter
    {

    }
}
