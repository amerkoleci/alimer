// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Vortice.Graphics
{
    public struct SwapChainDescriptor
    {
        public int Width;
        public int Height;
        public PixelFormat ColorFormat;
        public PixelFormat DepthStencilFormat;
        public PresentMode PresentMode;
        public bool Fullscreen;
        public IntPtr WindowHandle;
    }
}
