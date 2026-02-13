// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum ShaderStages
{
    None = 0,

    /// <summary>
    /// Vertex shader stage.
    /// </summary>
    Vertex = 1 << 0,

    /// <summary>
    /// Fragment (pixel) shader stage.
    /// </summary>
    Fragment = 1 << 1,

    /// <summary>
    /// Compute shader stage.
    /// </summary>
    Compute = 1 << 2,

    /// <summary>
    /// Mesh shader stage.
    /// </summary>
    Mesh = 1 << 3,

    /// <summary>
    /// Amplification shader stage.
    /// </summary>
    Amplification = 1 << 4,

#if RAYTRACING
    /// <summary>
    /// Library shader.
    /// </summary>
    Library = 1 << 5, 
#endif

    All = 0x3FFF,
}
