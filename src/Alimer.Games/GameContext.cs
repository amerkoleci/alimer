﻿// Copyright (c) Amer Koleci and contributors.
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
        /// <summary>
        /// Get the main window.
        /// </summary>
        public abstract GameWindow? GameWindow { get; }

        public GraphicsDevice GraphicsDevice { get; set; }

        public virtual void ConfigureServices(IServiceCollection services)
        {
            if (GraphicsDevice == null)
            {
                GraphicsDevice = GraphicsDevice.Create(new DeviceDescriptor(BackendType.Direct3D11));
            }

            services.AddSingleton(GraphicsDevice);
        }

        /// <summary>
        /// Run main loop.
        /// </summary>
        /// <param name="loadAction">The load action to execute.</param>
        /// <param name="tickAction">The tick action to execute.</param>
        /// <returns>Return true if blocking otherwise false.</returns>
        public abstract bool Run(Action loadAction, Action tickAction);
    }
}
