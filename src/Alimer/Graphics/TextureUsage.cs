// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// A bitmask indicating how a <see cref="Texture"/> is permitted to be used.
/// </summary>
[Flags]
public enum TextureUsage
{
    /// <summary>
    /// None usage.
    /// </summary>
    None = 0,
    /// <summary>
    /// Supports shader read access.
    /// </summary>
    ShaderRead = 1 << 0,
    /// <summary>
    /// Supports write read access.
    /// </summary>
    ShaderWrite = 1 << 1,
    /// <summary>
    /// Supports shader read and write access.
    /// </summary>
    ShaderReadWrite = ShaderRead | ShaderWrite,
    RenderTarget = 1 << 2,
    Transient = 1 << 3,
    ShadingRate = 1 << 4,
    Shared = 1 << 5,
}
