// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Define a <see cref="PixelFormat"/> or <see cref="VertexAttributeFormat"/> kind.
/// </summary>
public enum FormatKind
{
    /// <summary>
    /// Unsigned normalized formats.
    /// </summary>
    UNorm,
    /// <summary>
    /// Unsigned normalized SRGB formats.
    /// </summary>
    UNormSrgb,
    /// <summary>
    /// Signed normalized formats
    /// </summary>
    SNorm,
    /// <summary>
    /// Unsigned integer formats
    /// </summary>
    UInt,
    /// <summary>
    /// Signed integer formats
    /// </summary>
    SInt,
    /// <summary>
    /// Floating-point formats.
    /// </summary>
    Float,
    /// <summary>
    /// HDR formats.
    /// </summary>
    HDR,
}
