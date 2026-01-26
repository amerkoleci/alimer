// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Specifies the possible swizzle components for a texture channel mapping operation.
/// </summary>
/// <remarks>Texture swizzling allows remapping of texture color channels when sampling or reading from a texture.
/// This enumeration is typically used to define how each output channel (red, green, blue, alpha) is derived from the
/// input texture's channels or from constant values. The values correspond to selecting a specific channel or a
/// constant (zero or one) for the output.
/// </remarks>
public enum TextureSwizzle
{
    /// <summary>
    /// A value of 0.0 is copied to the destination channel.
    /// </summary>
    Zero,
    /// <summary>
    /// A value of 1.0 is copied to the destination channel.
    /// </summary>
    One,
    /// <summary>
    /// The red channel of the source pixel is copied to the destination channel.
    /// </summary>
    Red,
    /// <summary>
    /// The green channel of the source pixel is copied to the destination channel.
    /// </summary>
    Green,
    /// <summary>
    /// The blue channel of the source pixel is copied to the destination channel.
    /// </summary>
    Blue,
    /// <summary>
    /// The alpha channel of the source pixel is copied to the destination channel.
    /// </summary>
    Alpha,
}
