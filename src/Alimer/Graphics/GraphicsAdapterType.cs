// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum GraphicsAdapterType
{
    /// <summary>
    /// Other or Unknown.
    /// </summary>
    Other,
    /// <summary>
    /// Integrated GPU with shared CPU/GPU memory.
    /// </summary>
    IntegratedGPU,
    /// <summary>
    /// Discrete GPU with separate CPU/GPU memory.
    /// </summary>
    DiscreteGPU,
    /// <summary>
    /// Virtual / Hosted.
    /// </summary>
    VirtualGPU,
    /// <summary>
    /// CPU / Software / WARP Rendering.
    /// </summary>
    CPU,
}
