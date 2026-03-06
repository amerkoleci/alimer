// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Defines a <see cref="Font"/> asset.
/// </summary>
public class FontAsset : AssetWithSource
{
    public required int AtlasWidth { get; set; }
    public required int AtlasHeight { get; set; }
    public required byte[] AtlasData { get; set; }

    public required IList<FontGlyph> Glyphs { get; set; } = [];
}
