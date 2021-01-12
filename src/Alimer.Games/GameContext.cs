// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Alimer.Graphics;
using Microsoft.Extensions.DependencyInjection;

namespace Alimer
{
    /// <summary>
    /// Defines a context for <see cref="Game"/> that handles platform logic.
    /// </summary>
    public abstract class GameContext
    {
        private GraphicsDevice? graphicsDevice;

        /// <summary>
        /// Get the main window.
        /// </summary>
        public abstract GameWindow? GameWindow { get; }

        public GraphicsDevice? GraphicsDevice { get => graphicsDevice ??= CreateGraphicsDevice(); set => graphicsDevice = value; }

        protected GameContext()
        {
        }

        public virtual void ConfigureServices(IServiceCollection services)
        {
            services.AddSingleton(GraphicsDevice);
        }

        /// <summary>
        /// Run main loop.
        /// </summary>
        /// <param name="loadAction">The load action to execute.</param>
        /// <param name="tickAction">The tick action to execute.</param>
        /// <returns>Return true if blocking otherwise false.</returns>
        public abstract bool Run(Action loadAction, Action tickAction);

        private static GraphicsDevice? CreateGraphicsDevice()
        {
            //return GraphicsDevice.CreateSystemDefault(BackendType.Vulkan);
            if (RuntimePlatform.PlatformType == PlatformType.Windows)
            {
                if (GraphicsDevice.IsBackendSupported(BackendType.Direct3D12))
                {
                    return GraphicsDevice.CreateSystemDefault(BackendType.Direct3D12);
                }
            }

            return GraphicsDevice.CreateSystemDefault();
        }
    }
}
