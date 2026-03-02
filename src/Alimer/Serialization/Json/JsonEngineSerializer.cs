// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;

namespace Alimer.Serialization.Json;

/// <summary>
/// Class that provides JSON serialization methods for types implementing the <see cref="ISerializable"/>.
/// </summary>
internal partial class JsonEngineSerializer : Serializer, IDisposable
{
    private readonly Stream _stream;
    private readonly Utf8JsonWriter _writer;

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    /// <param name="mode">The read or write mode.</param>
    /// <param name="options">Optional JSON writer options.</param>
    public JsonEngineSerializer(Stream stream, JsonWriterOptions options = default)
    {
        _stream = stream;
        _writer = new Utf8JsonWriter(stream, options);
        _writer.WriteStartObject();
    }

    /// <inheritdoc />
    public override void Dispose()
    {
        _writer.WriteEndObject();
        _writer.Flush();
        _writer.Dispose();
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Flushes any buffered data to the underlying stream (write mode only).
    /// </summary>
    public void Flush()
    {
        _writer.Flush();
    }

    /// <inheritdoc />
    public override SerializationResult Serialize(string name, byte value)
    {
        _writer.WriteNumber(name, value);
        return SerializationResult.Ok;
    }

    public override SerializationResult BeginObject(string name, string? typeName = null)
    {
        _writer.WritePropertyName(name);
        _writer.WriteStartObject();
        if (typeName != null)
        {
            _writer.WriteString("Type", typeName);
        }
        return SerializationResult.Ok;
    }

    public override SerializationResult EndObject()
    {
        _writer.WriteEndObject();
        return SerializationResult.Ok;
    }
}
