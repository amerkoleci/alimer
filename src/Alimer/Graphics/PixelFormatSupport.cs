// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum PixelFormatSupport
{
    None = 0,

    Texture = (1 << 0),
    RenderTarget = (1 << 1),
    DepthStencil = (1 << 2),
    Blendable = (1 << 3),

    ShaderLoad = (1 << 4),
    ShaderSample = (1 << 5),
    ShaderUavLoad = (1 << 6),
    ShaderUavStore = (1 << 7),
    ShaderAtomic = (1 << 8),
}
