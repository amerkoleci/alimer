// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;
using Vortice.Graphics;

namespace Vortice
{
    public abstract class GameContext
    {
        public virtual void ConfigureServices(IServiceCollection services)
        {
        }

        public abstract void RunMainLoop(Action callback);
    }

    public abstract class GameContextWithGraphics : GameContext
    {
        private GraphicsDevice? graphicsDevice;

        public GraphicsDevice GraphicsDevice { get => graphicsDevice ??= GraphicsDevice.Default; set => graphicsDevice = value; }

        public override void ConfigureServices(IServiceCollection services)
        {
            base.ConfigureServices(services);

            services.AddSingleton(GraphicsDevice);
        }
    }
}
