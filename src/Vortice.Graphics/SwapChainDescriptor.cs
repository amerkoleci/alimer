// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public readonly struct SwapChainDescriptor
    {
        public int Width { get; }
        public int Height { get; }
        public TextureFormat ColorFormat { get; }
        //public TextureFormat DepthStencilFormat { get; }
        public bool EnableVerticalSync { get; }
        public bool IsFullscreen { get; }

        public SwapChainDescriptor(int width, int height,
            TextureFormat colorFormat = TextureFormat.BGRA8UNormSrgb,
            bool enableVerticalSync = true,
            bool isFullscreen = false)
        {
            Width = width;
            Height = height;
            ColorFormat = colorFormat;
            EnableVerticalSync = enableVerticalSync;
            IsFullscreen = isFullscreen;
        }
    }
}
