// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text;
using System.Text.Json;

namespace Alimer.Serialization.Json;

internal partial class JsonEngineDeserializer : Deserializer
{
    private readonly byte[] _buffer;
    private readonly JsonReaderOptions _options;
    private readonly JsonElement _rootElement;
    private JsonElement _currentElement;
    private readonly List<JsonElement> _elements = [];
    private JsonElement.ArrayEnumerator _currentArrayEnumerator;

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    public JsonEngineDeserializer(string json)
    {
        _options = new JsonReaderOptions
        {
            AllowTrailingCommas = true,
            CommentHandling = JsonCommentHandling.Skip
        };

        _buffer = Encoding.UTF8.GetBytes(json);
        Utf8JsonReader reader = new(_buffer, _options);
        _rootElement = JsonElement.ParseValue(ref reader);
        if (_rootElement.ValueKind != JsonValueKind.Object)
        {
            throw new JsonException("Expected start of JSON object.");
        }

        _currentElement = _rootElement;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="JsonEngineSerializer" /> class.
    /// </summary>
    /// <param name="stream">The stream to read or write to.</param>
    public JsonEngineDeserializer(Stream stream)
    {
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

    public override ObjectDeserializer BeginObject()
    {
        _elements.Add(_currentElement);
        return new JsonObjectDeserializer(this, _currentElement);
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

        _elements.Add(_currentElement);
        _currentElement = property;
        return true;
    }

    public void EndObject()
    {
        if (_elements.Count == 0)
            throw new InvalidOperationException();

        _currentElement = _elements[_elements.Count - 1];
        _elements.RemoveAt(_elements.Count - 1);
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

        _elements.Add(_currentElement);
        _currentElement = property;
        _currentArrayEnumerator = property.EnumerateArray();
        return property.GetArrayLength();
    }

    public override void EndArray()
    {
        EndObject();
    }

    class JsonObjectDeserializer : ObjectDeserializer
    {
        private readonly JsonEngineDeserializer _deserializer;
        private readonly JsonElement _jsonElement;

        public JsonObjectDeserializer(JsonEngineDeserializer deserializer, JsonElement jsonElement)
        {
            _deserializer = deserializer;
            _jsonElement = jsonElement;
        }

        /// <inheritdoc />
        public override void Dispose()
        {
            _deserializer.EndObject();
        }

        /// <inheritdoc />
        public override bool ReadBool(string propertyName, bool defaultValue = false)
        {
            if (!_jsonElement.TryGetProperty(propertyName, out JsonElement property))
            {
                return defaultValue;
            }

            return property.GetBoolean();
        }

        /// <inheritdoc />
        public override int ReadInt32(string propertyName, int defaultValue)
        {
            if (!_jsonElement.TryGetProperty(propertyName, out JsonElement property))
            {
                return defaultValue;
            }

            return property.GetInt32();
        }

        /// <inheritdoc />
        public override string? ReadString(string propertyName, string? defaultValue = null)
        {
            if (!_jsonElement.TryGetProperty(propertyName, out JsonElement property))
            {
                return defaultValue;
            }

            return property.GetString();
        }

        /// <inheritdoc />
        public override Guid ReadGuid(string propertyName, Guid defaultValue = default)
        {
            if (!_jsonElement.TryGetProperty(propertyName, out JsonElement property))
            {
                return defaultValue;
            }

            string? guidString = property.GetString();
            return string.IsNullOrEmpty(guidString) ? defaultValue : Guid.Parse(guidString);
        }
    }
}
