// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public record SwapChainDescriptor
    {
        public int Width { get; init; }
        public int Height { get; init; }
        public TextureFormat ColorFormat { get; init; } = TextureFormat.BGRA8UNormSrgb;
        //public TextureFormat DepthStencilFormat { get; }
        public bool EnableVerticalSync { get; init; } = true;
        public bool IsFullscreen { get; init; } = false;
    }
}
