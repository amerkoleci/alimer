// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Vortice.Graphics
{
    public readonly struct SwapChainDescriptor
    {
        public int Width { get; }
        public int Height { get; }
        public TextureFormat ColorFormat { get; }
        public TextureFormat DepthStencilFormat { get; }
        public PresentMode PresentMode { get; }
        public bool IsFullscreen { get; }
    }
}
