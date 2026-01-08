// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets.Graphics;

public sealed class TextureMetadata : AssetMetadata
{
    public float Width { get; set; } = 100.0f;
    public float Height { get; set; } = 100.0f;
    public bool IsSizeInPercentage { get; set; } = true;
    public bool GenerateMipmaps { get; set; }
    public TextureCompressionFormat CompressionFormat { get; set; }
}
