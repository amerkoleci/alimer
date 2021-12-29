// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly struct GraphicsDeviceCaps
{
    public GraphicsDeviceFeatures Features { get; init; }
    public GraphicsDeviceLimits Limits { get; init; }
}
