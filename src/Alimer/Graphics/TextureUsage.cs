// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Alimer.Graphics
{
    /// <summary>
    /// A bitmask indicating how a <see cref="Texture"/> is permitted to be used.
    /// </summary>
    [Flags]
    public enum TextureUsage
    {
        None = 0,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        ShaderReadWrite = ShaderRead | ShaderWrite,
        RenderTarget = 1 << 2,
    }
}
