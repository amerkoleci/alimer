// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Constants;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="TextureView"/>.
/// </summary>
public record struct TextureViewDescriptor
{
    public TextureViewDimension Dimension = TextureViewDimension.Undefined;
    public PixelFormat Format = PixelFormat.Undefined;
    public uint BaseMipLevel = 0;
    public uint MipLevelCount = MipLevelCountUndefined;
    public uint BaseArrayLayer = 0;                           // For Texture3D, this is WSlice.
    public uint ArrayLayerCount = ArrayLayerCountUndefined;   // For cube maps, this is a multiple of 6.
    public TextureSwizzleChannels Swizzle = TextureSwizzleChannels.Default;
    public TextureAspect Aspect = TextureAspect.All;

    /// <summary>
    /// Gets or sets the label of <see cref="TextureView"/>.
    /// </summary>
    public string? Label;

    public TextureViewDescriptor()
    {

    }
}
