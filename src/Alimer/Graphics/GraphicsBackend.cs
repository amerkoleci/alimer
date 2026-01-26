// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Enum defining backend type of <see cref="GraphicsDevice"/>.
/// </summary>
public enum GraphicsBackend
{
    Default = 0,
    Null,
    Vulkan,
    D3D12,
    Metal,
    WebGPU,
}
