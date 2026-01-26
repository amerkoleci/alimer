// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes swizzle channels in <see cref="TextureView"/>.
/// </summary>
public record struct TextureSwizzleChannels
{
    /// <summary>
    /// Gets or sets the swizzle operation for the red channel of the texture.
    /// </summary>
    public TextureSwizzle Red = TextureSwizzle.Red;

    /// <summary>
    /// Gets or sets the swizzle operation for the green channel of the texture.
    /// </summary>
    public TextureSwizzle Green = TextureSwizzle.Green;

    /// <summary>
    /// Gets or sets the swizzle operation for the blue channel of the texture.
    /// </summary>
    public TextureSwizzle Blue = TextureSwizzle.Blue;

    /// <summary>
    /// Specifies the swizzle operation for the alpha channel in a texture format mapping.
    /// </summary>
    public TextureSwizzle Alpha = TextureSwizzle.Alpha;

    public TextureSwizzleChannels()
    {
    }

    public TextureSwizzleChannels(TextureSwizzle red, TextureSwizzle green, TextureSwizzle blue, TextureSwizzle alpha)
    {
        Red = red;
        Green = green;
        Blue = blue;
        Alpha = alpha;
    }

    public static TextureSwizzleChannels Default => new();
}
