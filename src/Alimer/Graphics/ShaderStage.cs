// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum ShaderStage
{
    /// <summary>
    /// All stages.
    /// </summary>
    All = 0,

    /// <summary>
    /// Vertex shader stage.
    /// </summary>
    Vertex,

    /// <summary>
    /// Hull shader stage.
    /// </summary>
    Hull,

    /// <summary>
    /// Domain shader stage.
    /// </summary>
    Domain,

    /// <summary>
    /// Geometry shader stage.
    /// </summary>
    Geometry,

    /// <summary>
    /// Fragment (pixel) shader stage.
    /// </summary>
    Fragment,

    /// <summary>
    /// Compute shader stage.
    /// </summary>
    Compute,

    /// <summary>
    /// Amplification shader stage.
    /// </summary>
    Amplification,

    /// <summary>
    /// Mesh shader stage.
    /// </summary>
    Mesh,

    /// <summary>
    /// Library shader.
    /// </summary>
    Library,
}
