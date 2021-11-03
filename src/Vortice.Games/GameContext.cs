// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Toolkit.Diagnostics;
using Vortice.Graphics;

namespace Vortice
{
    public abstract class GameContext
    {
        public virtual void ConfigureServices(IServiceCollection services)
        {
        }

        public abstract void RunMainLoop(Action init, Action callback);
    }

    public abstract class GameContextWithGraphics : GameContext
    {
        public GraphicsDevice GraphicsDevice { get; set; }

        public GameContextWithGraphics(GraphicsDevice graphicsDevice)
        {
            Guard.IsNotNull(graphicsDevice, nameof(graphicsDevice));

            GraphicsDevice = graphicsDevice;
        }

        public override void ConfigureServices(IServiceCollection services)
        {
            base.ConfigureServices(services);

            services.AddSingleton(GraphicsDevice);
        }
    }
}
