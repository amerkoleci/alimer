// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    /// <summary>
    /// A bitmask indicating how a <see cref="Texture"/> is permitted to be used.
    /// </summary>
    public enum TextureUsage
    {
        None = 0,
        Sampled = 1 << 0,
        Storage = 1 << 1,
        RenderTarget = 1 << 2,
    }
}
