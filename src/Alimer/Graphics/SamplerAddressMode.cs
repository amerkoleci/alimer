// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum SamplerAddressMode
{
    Clamp,
    Wrap,
    Mirror,
    MirrorOnce,
    Border,

    // Vulkan names
    ClampToEdge = Clamp,
    Repeat = Wrap,
    MirroredRepeat = Mirror,
    ClampToBorder = Border,
    MirrorClampToEdge = MirrorOnce
}
