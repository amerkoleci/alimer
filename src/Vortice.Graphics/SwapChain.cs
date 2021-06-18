// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;

namespace Vortice.Graphics
{
    public abstract class SwapChain : GraphicsResource
    {
        protected SwapChain(GraphicsDevice device, in SwapChainDescriptor descriptor)
            : base(device)
        {
            Size = new(descriptor.Width, descriptor.Height);
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
