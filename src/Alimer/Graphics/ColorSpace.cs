// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines surface color space
/// </summary>
public enum ColorSpace
{
    /// <summary>
    /// SDR color space (8 or 10 bits per channel) 
    /// </summary>
    SRGB,
    /// <summary>
    /// HDR10 color space (10 bits per channel)
    /// </summary>
    HDR10_ST2084,
    /// <summary>
    /// HDR color space (16 bits per channel)
    /// </summary>
    HDR_LINEAR,
}
