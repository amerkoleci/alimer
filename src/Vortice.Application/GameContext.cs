// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice.Graphics;
using Microsoft.Extensions.DependencyInjection;
using Vortice.Content;

namespace Vortice
{
    /// <summary>
    /// Defines a context for <see cref="Application"/> that handles platform logic.
    /// </summary>
    public abstract class GameContext
    {
        private GraphicsDevice? graphicsDevice;

        public IFileProvider? FileProvider { get; set; }

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
            //IFileProvider fileProvider = FileProvider ?? new FileSystemProvider(Package.Current.InstalledLocation, ApplicationData.Current.TemporaryFolder);
            //services.AddSingleton(fileProvider);
            services.AddSingleton(GraphicsDevice);
        }

        /// <summary>
        /// Run main loop.
        /// </summary>
        /// <param name="loadAction">The load action to execute.</param>
        /// <param name="tickAction">The tick action to execute.</param>
        /// <returns>Return true if blocking otherwise false.</returns>
        public abstract bool Run(Action loadAction, Action tickAction);

        protected abstract GraphicsDevice? CreateGraphicsDevice();
    }
}
