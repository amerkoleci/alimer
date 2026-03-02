// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Alimer.Serialization.Json;

internal partial class JsonEngineDeserializer : Deserializer, IDisposable
{
    private readonly Stream _stream;
    private readonly JsonReaderOptions _options;
    private readonly byte[] _buffer;
    private int _readerPosition = 0;

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    /// <param name="mode">The read or write mode.</param>
    /// <param name="options">Optional JSON writer options.</param>
    public JsonEngineDeserializer(Stream stream, JsonReaderOptions options = default)
    {
        _stream = stream;
        _options = options;

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
        ReadOnlySpan<byte> span = _buffer.AsSpan(_readerPosition);
        Utf8JsonReader reader = new(span, options);
        reader.Read();
        if (reader.TokenType != JsonTokenType.StartObject)
        {
            throw new JsonException("Expected start of JSON object.");
        }
        _readerPosition += (int)reader.BytesConsumed;
    }

    /// <inheritdoc />
    public override void Dispose()
    {
    }

    public override SerializationResult BeginObject(string name, string? typeName = null)
    {
        ReadOnlySpan<byte> span = _buffer.AsSpan(_readerPosition);
        Utf8JsonReader reader = new(span, _options);
        reader.Read();
        if (reader.TokenType != JsonTokenType.StartObject)
        {
            throw new JsonException("Expected start of JSON object.");
        }

        _readerPosition += (int)reader.BytesConsumed;
        return SerializationResult.Ok;
    }

    public override SerializationResult EndObject()
    {
        ReadOnlySpan<byte> span = _buffer.AsSpan(_readerPosition);
        Utf8JsonReader reader = new(span, _options);
        reader.Read();
        if (reader.TokenType != JsonTokenType.EndObject)
        {
            throw new JsonException("Expected end of JSON object.");
        }
        _readerPosition += (int)reader.BytesConsumed;
        return SerializationResult.Ok;
    }

    public override byte Deserialize(string name) => throw new NotImplementedException();

}
