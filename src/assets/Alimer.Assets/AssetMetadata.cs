// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text.Json;
using System.Text.Json.Serialization;
using Alimer.Assets.Graphics;

namespace Alimer.Assets;

[JsonPolymorphic]
//[JsonDerivedType(typeof(ShaderMetadata), nameof(ShaderMetadata))]
[JsonDerivedType(typeof(TextureMetadata), nameof(TextureMetadata))]
//[JsonDerivedType(typeof(FontMetadata), nameof(FontMetadata))]
public class AssetMetadata
{
    [JsonPropertyOrder(-100)]
    public Guid Id { get; set; } = Guid.NewGuid();

    [JsonIgnore]
    public string FileFullPath { get; set; } = string.Empty;

    [JsonIgnore]
    public string FileRelativePath { get; set; } = string.Empty;

    public string ToJson()
    {
        return JsonSerializer.Serialize<AssetMetadata>(this, AssetMetadataJsonContext.Default.AssetMetadata);
    }

    public static AssetMetadata? FromFile(string filePath)
    {
        string jsonString = File.ReadAllText(filePath);
        return FromJson(jsonString);
    }

    public static AssetMetadata? FromJson(string json)
    {
        return JsonSerializer.Deserialize(json, AssetMetadataJsonContext.Default.AssetMetadata);
    }

    //public static readonly JsonSerializerOptions SerializerOptions = new()
    //{
    //    PropertyNameCaseInsensitive = true,
    //    PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
    //    WriteIndented = true,
    //    IgnoreReadOnlyProperties = true,
    //    DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
    //    Converters =
    //    {
    //        new JsonStringEnumConverter<TextureCompressionFormat>(),
    //        new Vector2JsonConverter(),
    //        new Vector3JsonConverter()
    //    },
    //};
}

[JsonSerializable(typeof(AssetMetadata))]
[JsonSourceGenerationOptions(
    WriteIndented = true,
    PropertyNameCaseInsensitive = true,
    PropertyNamingPolicy = JsonKnownNamingPolicy.CamelCase,
    IgnoreReadOnlyProperties = true,
    DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
    Converters = [
        typeof(JsonStringEnumConverter<TextureCompressionFormat>),
        //typeof(Vector2JsonConverter),
        //typeof(Vector3JsonConverter)
    ]
)]
internal partial class AssetMetadataJsonContext : JsonSerializerContext;
