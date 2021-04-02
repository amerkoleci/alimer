// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.Mathematics;

namespace Vortice.Graphics
{
    public abstract class SwapChain : GraphicsResource
    {
        protected SwapChain(GraphicsDevice device, in SwapChainDescriptor descriptor)
            : base(device)
        {
            Size = new Size(descriptor.Width, descriptor.Height);
            ColorFormat = descriptor.ColorFormat;
            EnableVerticalSync = descriptor.EnableVerticalSync;
            IsFullscreen = descriptor.IsFullscreen;
        }

        public Size Size { get; protected set; }

        public TextureFormat ColorFormat { get; protected set; }
        public bool EnableVerticalSync { get; }
        public bool IsFullscreen { get; }

        public abstract void Present();
    }
}
