// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public readonly record struct SwapChainDescription
{
    public SwapChainDescription()
    {

    }

    public SwapChainDescription(
        PixelFormat colorFormat = PixelFormat.BGRA8UnormSrgb,
        PresentMode presentMode = PresentMode.Fifo)
    {
        Format = colorFormat;
        PresentMode = presentMode;
    }

    public PixelFormat Format { get; init; } = PixelFormat.BGRA8UnormSrgb;
    public PresentMode PresentMode { get; init; } = PresentMode.Fifo;
    public bool IsFullscreen { get; init; } = false;

    public string? Label { get; init; } = default;
}
