// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;

namespace Alimer.Serialization.Json;

internal partial class JsonEngineDeserializer : Deserializer, IDisposable
{
    private readonly Stream _stream;
    private readonly JsonReaderOptions _options;
    private readonly byte[] _buffer;
    private readonly JsonElement _rootElement;
    private JsonElement _currentElement;
    private JsonElement.ArrayEnumerator _currentArrayEnumerator;

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    public JsonEngineDeserializer(Stream stream)
    {
        _stream = stream;
        _options = new JsonReaderOptions
        {
            AllowTrailingCommas = true,
            CommentHandling = JsonCommentHandling.Skip
        };

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

        ReadOnlySpan<byte> span = _buffer.AsSpan();
        Utf8JsonReader reader = new(span, _options);
        _rootElement = JsonElement.ParseValue(ref reader);
        if (_rootElement.ValueKind != JsonValueKind.Object)
        {
            throw new JsonException("Expected start of JSON object.");
        }

        _currentElement = _rootElement;
    }

    /// <inheritdoc />
    public override void Dispose()
    {
    }

    //public override SerializationResult BeginObject(string name, string? typeName = null)
    //{
    //    ReadOnlySpan<byte> span = _buffer.AsSpan(_readerPosition);
    //    Utf8JsonReader reader = new(span, _options);
    //    reader.Read();
    //    if (reader.TokenType != JsonTokenType.StartObject)
    //    {
    //        throw new JsonException("Expected start of JSON object.");
    //    }

    //    _readerPosition += (int)reader.BytesConsumed;
    //    return SerializationResult.Ok;
    //}

    //public override SerializationResult EndObject()
    //{
    //    ReadOnlySpan<byte> span = _buffer.AsSpan(_readerPosition);
    //    Utf8JsonReader reader = new(span, _options);
    //    reader.Read();
    //    if (reader.TokenType != JsonTokenType.EndObject)
    //    {
    //        throw new JsonException("Expected end of JSON object.");
    //    }
    //    _readerPosition += (int)reader.BytesConsumed;
    //    return SerializationResult.Ok;
    //}

    /// <inheritdoc />
    public override int ReadInt32(string propertyName, int defaultValue)
    {
        if (!_currentElement.TryGetProperty(propertyName, out JsonElement property))
        {
            //throw new JsonException($"Property '{name}' not found.");
            return defaultValue;
        }

        return property.GetInt32();
    }

    public override string? ReadString(string propertyName, string? defaultValue = null)
    {
        if (!_currentElement.TryGetProperty(propertyName, out JsonElement property))
        {
            return defaultValue;
        }

        return property.GetString();
    }

    public override Guid ReadGuid(string propertyName, Guid defaultValue = default)
    {
        if (!_currentElement.TryGetProperty(propertyName, out JsonElement property))
        {
            return defaultValue;
        }

        string? guidString = property.GetString();
        return string.IsNullOrEmpty(guidString) ? defaultValue : Guid.Parse(guidString);
    }

    public override bool BeginObject(string? propertyName = null)
    {
        JsonElement property = _currentElement;
        if (propertyName != null)
        {
            if (!_currentElement.TryGetProperty(propertyName, out property))
            {
                return false;
            }
        }
        else
        {
            if (!_currentArrayEnumerator.MoveNext())
            {
                return false;
            }

            property = _currentArrayEnumerator.Current;
        }

        if (property.ValueKind != JsonValueKind.Object)
        {
            throw new JsonException($"Property '{propertyName}' is not an object.");
        }

        // Store the object element for subsequent reads
        _currentElement = property;
        return true;
    }

    public override void EndObject()
    {
        _currentElement = _rootElement;
    }

    public override int BeginArray(string propertyName)
    {
        if (!_currentElement.TryGetProperty(propertyName, out JsonElement property))
        {
            return -1;
        }

        if (property.ValueKind != JsonValueKind.Array)
        {
            throw new JsonException($"Property '{propertyName}' is not an array.");
        }

        // Store the array element for subsequent reads
        //_currentArray = property.EnumerateArray();
        _currentElement = property;
        _currentArrayEnumerator = property.EnumerateArray();
        return property.GetArrayLength();
    }

    public override void EndArray()
    {
        // TODO: Stack with elements

        _currentElement = _rootElement;
    }
}
