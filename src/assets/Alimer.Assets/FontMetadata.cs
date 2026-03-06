// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;

namespace Alimer.Assets;

public sealed class FontMetadata : AssetMetadata
{
    public const string DefaultCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789,.;:-+*/?!%&()";

    public string? FontFamily { get; set; }
    public int FontSize { get; set; } = 14;

    public string Characters { get; set; } = DefaultCharacters;

    [DefaultValue(0)]
    public float OutlineThickness { get; set; }
}
