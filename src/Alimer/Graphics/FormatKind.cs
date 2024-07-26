// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Define a <see cref="PixelFormat"/> or <see cref="VertexFormat"/> kind.
/// </summary>
public enum FormatKind
{
    /// <summary>
    /// Unsigned normalized formats.
    /// </summary>
    Unorm,
    /// <summary>
    /// Unsigned normalized SRGB formats.
    /// </summary>
    UnormSrgb,
    /// <summary>
    /// Signed normalized formats
    /// </summary>
    Snorm,
    /// <summary>
    /// Unsigned integer formats
    /// </summary>
    Uint,
    /// <summary>
    /// Signed integer formats
    /// </summary>
    Sint,
    /// <summary>
    /// Floating-point formats.
    /// </summary>
    Float,
    /// <summary>
    /// Hdr formats.
    /// </summary>
    Hdr,
}
