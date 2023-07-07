// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum ShaderStages 
{
    /// <summary>
    /// No stages.
    /// </summary>
    None = 0,

    /// <summary>
    /// Vertex shader stage.
    /// </summary>
    Vertex = 1 << 0,

    /// <summary>
    /// Hull shader stage.
    /// </summary>
    Hull = 1 << 1,

    /// <summary>
    /// Domain shader stage.
    /// </summary>
    Domain = 1 << 2,

    /// <summary>
    /// Fragment (pixel) shader stage.
    /// </summary>
    Fragment = 1 << 3,

    /// <summary>
    /// Compute shader stage.
    /// </summary>
    Compute = 1 << 4,

    /// <summary>
    /// Amplification shader stage.
    /// </summary>
    Amplification = 1 << 5,

    /// <summary>
    /// Mesh shader stage.
    /// </summary>
    Mesh = 1 << 6,
}
