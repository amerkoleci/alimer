// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Describes the CPU access for <see cref="GraphicsBuffer"/> and <see cref="Texture"/>.
/// </summary>
public enum MemoryType
{
    /// <summary>
    /// CPU no access, GPU read/write
    /// </summary>
    Private = 0,
    /// <summary>
    /// CPU write, GPU read
    /// </summary>
    Upload,
    /// <summary>
    /// CPU read, GPU write
    /// </summary>
    Readback
}
