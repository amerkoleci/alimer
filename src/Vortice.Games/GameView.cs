// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Drawing;
using Vortice.Graphics;

namespace Vortice
{
    public abstract class GameView
    {
        public event EventHandler? SizeChanged;

        public abstract SizeF ClientSize { get; }

        public abstract SwapChainSource Source { get; }

        public SwapChain? SwapChain { get; private set; }

        public void CreateSwapChain(GraphicsDevice device)
        {
            SwapChainDescriptor descriptor = new()
            {
                Size = new Size((int)ClientSize.Width, (int)ClientSize.Height),
            };

            SwapChain = device.CreateSwapChain(Source, descriptor);
        }

        public void Present()
        {
            SwapChain?.Present();
        }

        protected virtual void OnSizeChanged()
        {
            SizeChanged?.Invoke(this, EventArgs.Empty);
        }
    }
}
