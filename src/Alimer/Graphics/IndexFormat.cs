// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Index buffer element format.
/// </summary>
public enum IndexFormat
{
    /// <summary>
    /// Undefined index format, used to disable index buffer stripping <see cref="RenderPipelineDescriptor.StripIndexFormat"/>.
    /// </summary>
    Undefined,
    /// <summary>
    /// 16-bit unsigned integer indices.
    /// </summary>
	Uint16,
    /// <summary>
    /// 32-bit unsigned integer indices.
    /// </summary>
    Uint32,
}
