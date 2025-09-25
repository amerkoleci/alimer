// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum TextureAspect : uint
{
    All = 0,
    DepthOnly = 1,
    StencilOnly = 2,
}
