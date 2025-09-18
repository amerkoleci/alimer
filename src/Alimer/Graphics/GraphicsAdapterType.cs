// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum GraphicsAdapterType
{
    /// <summary>
    /// The device is typically a separate processor connected to the host via an interlink.
    /// </summary>
    DiscreteGpu,

    /// <summary>
    /// The device is typically one embedded in or tightly coupled with the host.
    /// </summary>
    IntegratedGpu,

    /// <summary>
    /// The device is typically a virtual node in a virtualization environment.
    /// </summary>
    VirtualGpu,

    /// <summary>
    /// The device is typically running on the same processors as the host.
    /// </summary>
    Cpu,

    /// <summary>
    /// The device does not match any other available types.
    /// </summary>
    Other,
}
