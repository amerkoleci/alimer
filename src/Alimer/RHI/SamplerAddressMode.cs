// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.RHI;

/// <summary>
/// Specifies the addressing mode used by a texture sampler to handle texture coordinates outside the standard [0, 1]
/// range.
/// </summary>
public enum SamplerAddressMode
{
    /// <summary>
    /// Texture coordinates wrap to the other side of the texture, effectively keeping only the fractional part of the texture coordinate.
    /// </summary>
    Repeat,
    /// <summary>
    /// Texture coordinates between -1.0 and 1.0 are mirrored across the axis; outside -1.0 and 1.0, the image is repeated.
    /// </summary>
    MirrorRepeat,
    /// <summary>
    /// Texture coordinates are clamped between 0.0 and 1.0, inclusive
    /// </summary>
    ClampToEdge,
    /// <summary>
    /// Texture coordinates outside range return the value specified by the <see cref="SamplerDescriptor.BorderColor"/> property.
    /// </summary>
    ClampToBorder,
    /// <summary>
    /// Texture coordinates between -1.0 and 1.0 are mirrored across the axis; outside -1.0 and 1.0, texture coordinates are clamped.
    /// </summary>
    MirrorClampToEdge,
}
