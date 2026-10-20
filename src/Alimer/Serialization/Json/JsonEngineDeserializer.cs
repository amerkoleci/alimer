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
    private JsonElement.ArrayEnumerator? _currentArrayEnumerator;
    private readonly List<JsonElement> _elements = [];

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
        if (_rootElement.ValueKind != JsonValueKind.Object
            && _rootElement.ValueKind != JsonValueKind.Array)
        {
            throw new JsonException("Expected start of JSON object or array.");
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
        if (_rootElement.ValueKind != JsonValueKind.Object
            && _rootElement.ValueKind != JsonValueKind.Array)
        {
            throw new JsonException("Expected start of JSON object or array.");
        }

        _currentElement = _rootElement;
    }

    private void EnsureCurrentObject()
    {
        // We read a value from other property?
        if (_currentElement.ValueKind != JsonValueKind.Object)
        {
            _currentElement = _elements[_elements.Count - 1];
            _elements.RemoveAt(_elements.Count - 1);
        }
    }

    /// <inheritdoc />
    public override bool BeginObject(string? propertyName = null)
    {
        EnsureCurrentObject();

        JsonElement property;
        if (!string.IsNullOrEmpty(propertyName))
        {
            if (!_currentElement.TryGetProperty(propertyName, out property))
            {
                return false;
            }
        }
        else
        {
            if (_currentArrayEnumerator.HasValue && _currentElement.ValueKind == JsonValueKind.Array)
            {
                if (!_currentArrayEnumerator.Value.MoveNext())
                {
                    return false;
                }

                property = _currentArrayEnumerator.Value.Current;
            }
            else
            {
                property = _currentElement;
            }
        }

        if (property.ValueKind != JsonValueKind.Object)
        {
            //throw new JsonException($"Property '{propertyName}' is not an object.");
            return false;
        }

        _elements.Add(_currentElement);
        _currentElement = property;
        return true;
    }

    /// <inheritdoc />
    public override void EndObject()
    {
        if (_elements.Count == 0)
            throw new InvalidOperationException();

        _currentElement = _elements[_elements.Count - 1];
        _elements.RemoveAt(_elements.Count - 1);
    }

    /// <inheritdoc />
    public override int BeginArray(string? propertyName = default)
    {
        EnsureCurrentObject();

        JsonElement property = _currentElement;
        if (!string.IsNullOrEmpty(propertyName))
        {
            if (!_currentElement.TryGetProperty(propertyName, out property))
            {
                return -1;
            }
        }

        if (property.ValueKind != JsonValueKind.Array)
        {
            //throw new JsonException($"Property '{propertyName}' is not an array.");
            return -1;
        }

        int arrayLength = property.GetArrayLength();
        _elements.Add(_currentElement);
        _currentElement = property;
        _currentArrayEnumerator = property.EnumerateArray();
        

        return arrayLength;
    }

    /// <inheritdoc />
    public override void EndArray()
    {
        EndObject();
        _currentArrayEnumerator = default;
    }

    /// <inheritdoc />
    public override bool HasProperty(string propertyName)
    {
        return _currentElement.TryGetProperty(propertyName, out _);
    }

    /// <inheritdoc />
    public override void ReadProperty(string propertyName)
    {
        EnsureCurrentObject();

        JsonElement property = _currentElement.GetProperty(propertyName);
        _elements.Add(_currentElement);
        _currentElement = property;
    }

    /// <inheritdoc />
    public override bool TryReadProperty(string propertyName)
    {
        EnsureCurrentObject();

        if (_currentElement.TryGetProperty(propertyName, out JsonElement property))
        {
            _elements.Add(_currentElement);
            _currentElement = property;
            return true;
        }

        return false;
    }

    /// <inheritdoc />
    public override bool ReadBool(bool defaultValue = false)
    {
        return _currentElement.ValueKind switch
        {
            JsonValueKind.True => true,
            JsonValueKind.False => false,
            _ => defaultValue
        };
    }

    /// <inheritdoc />
    public override int ReadInt32(int defaultValue = 0)
    {
        if (_currentElement.ValueKind == JsonValueKind.Number && _currentElement.TryGetInt32(out int value))
        {
            return value;
        }

        return defaultValue;
    }

    /// <inheritdoc />
    public override string? ReadString(string? defaultValue = null)
    {
        if (_currentElement.ValueKind == JsonValueKind.String)
        {
            return _currentElement.GetString();
        }

        return defaultValue;
    }

    /// <inheritdoc />
    public override Guid ReadGuid(Guid defaultValue = default)
    {
        if (_currentElement.ValueKind == JsonValueKind.String && _currentElement.TryGetGuid(out Guid value))
        {
            return value;
        }

        return defaultValue;
    }

    /// <inheritdoc />
    public override bool ReadValues(scoped Span<float> values)
    {
        if (_currentElement.ValueKind == JsonValueKind.Array)
        {
            int length = _currentElement.GetArrayLength();
            if (values.Length != length)
            {
                throw new ArgumentException($"The provided span length ({values.Length}) does not match the array length ({length}).");
            }

            int index = 0;
            foreach (JsonElement element in _currentElement.EnumerateArray())
            {
                if (element.ValueKind == JsonValueKind.Number && element.TryGetSingle(out float value))
                {
                    values[index++] = value;
                }
                else
                {
                    throw new JsonException("Expected a number in the array.");
                }
            }

            return true;
        }

        return false;
    }
}
