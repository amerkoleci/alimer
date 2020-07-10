// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SwapchainInfo
    {
        public uint Width;
        public uint Height;
        public PixelFormat ColorFrmat;
        public PixelFormat DepthStencilFormat;
        public PresentMode PresentMode;
        public Bool32 Fullscreen;
        public IntPtr WindowHandle;
    }
}
