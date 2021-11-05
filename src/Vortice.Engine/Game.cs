// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.Extensions.DependencyInjection;
using Vortice.Graphics;

namespace Vortice.Engine
{
    public class Game : GameBase
    {
        public Game(GameContext context)
            : base(context)
        {
            GraphicsDevice = Services.GetRequiredService<GraphicsDevice>();
        }

        public GraphicsDevice GraphicsDevice { get; }

        protected override void Dispose(bool dispose)
        {
            if (dispose && !IsDisposed)
            {
                GraphicsDevice.WaitIdle();

                View.SwapChain?.Dispose();
                GraphicsDevice.Dispose();
            }

            base.Dispose(dispose);
        }

        protected override void Initialize()
        {
            View.CreateSwapChain(GraphicsDevice);

            base.Initialize();
        }
    }
}
