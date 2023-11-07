// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines dimension of <see cref="Image"/> or <see cref="Texture"/>
/// </summary>
public enum TextureDimension
{
    /// <summary>
    /// One-dimensional Texture.
    /// </summary>
    Texture1D,
    /// <summary>
    /// Two-dimensional Texture.
    /// </summary>
    Texture2D,
    /// <summary>
    /// Three-dimensional Texture.
    /// </summary>
    Texture3D,
    /// <summary>
    /// Cubemap Texture.
    /// </summary>
    TextureCube,

    Count
}
