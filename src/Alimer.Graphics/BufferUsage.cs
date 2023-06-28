// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// A bitmask indicating how a <see cref="Buffer"/> is permitted to be used.
/// </summary>
[Flags]
public enum BufferUsage
{
    None = 0,
    /// <summary>
    /// Supports input assembly access as VertexBuffer.
    /// </summary>
    Vertex = 1 << 0,
    /// <summary>
    /// Supports input assembly access as IndexBuffer.
    /// </summary>
    Index = 1 << 1,
    /// <summary>
    /// Supports constant buffer access.
    /// </summary>
    Constant = 1 << 2,
    /// <summary>
    /// Supports shader read access.
    /// </summary>
    ShaderRead = 1 << 3,
    /// <summary>
    /// Supports write read access.
    /// </summary>
    ShaderWrite = 1 << 4,
    /// <summary>
    /// Supports shader read and write access.
    /// </summary>
    ShaderReadWrite = ShaderRead | ShaderWrite,
    /// <summary>
    /// Supports indirect buffer access for indirect draw/dispatch.
    /// </summary>
    Indirect = 1 << 5,
    /// <summary>
    /// Supports ray tracing acceleration structure usage.
    /// </summary>
    RayTracing = 1 << 6,
    /// <summary>
    /// Supports predication access for conditional rendering.
    /// </summary>
    Predication = 1 << 7,
}
