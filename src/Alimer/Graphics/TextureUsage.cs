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
    /// Supports shader reading or sampling access.
    /// </summary>
    ShaderRead = 1 << 0,
    /// <summary>
    /// Supports shader write access.
    /// </summary>
    ShaderWrite = 1 << 1,
    /// <summary>
    /// Supports shader read-write access.
    /// </summary>
    ShaderReadWrite = ShaderRead | ShaderWrite,
    /// <summary>
    /// Supports rendering to the texture in a render pass.
    /// </summary>
    RenderTarget = 1 << 2,
    /// <summary>
    /// Supports transient usage, which allows the texture to be used as a render target or depth-stencil attachment without the need for explicit resource transitions.
    /// This can improve performance by reducing synchronization overhead when the texture is only used within a single render pass and does not need to be accessed by shaders outside of that pass.
    /// </summary>
    Transient = 1 << 3,
    /// <summary>
    /// Supports creating <see cref="TextureView"/> suitable for use with variable rate shading (VRS) pipelines.
    /// </summary>
    ShadingRate = 1 << 4,
    /// <summary>
    /// Supports shared handle usage.
    /// </summary>
    Shared = 1 << 5,
}
