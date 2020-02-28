// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Alimer.Input;
using Microsoft.Extensions.DependencyInjection;

namespace Alimer
{
    public abstract class Game : IDisposable
    {
        private readonly object _tickLock = new object();
        private bool _isExiting;
        private readonly Stopwatch _stopwatch = new Stopwatch();
        private bool _endRunRequired;

        public GameContext Context { get; }
        public IServiceProvider Services { get; }

        /// <summary>
        /// Gets a list of registered <see cref="GameSystem"/>.
        /// </summary>
        public IList<GameSystem> GameSystems { get; } = new List<GameSystem>();

        /// <summary>
        /// Gets value whether the game is running.
        /// </summary>
        public bool IsRunning { get; private set; }

        protected Game(GameContext context)
        {
            Context = context;

            // Configure and build services
            ServiceCollection services = new ServiceCollection();
            ConfigureServices(services);
            Services = services.BuildServiceProvider();

            Platform = Services.GetRequiredService<IRuntimePlatform>();
            //Input = Services.GetRequiredService<InputManager>();
        }

        public IRuntimePlatform Platform { get; }

        public InputManager Input { get; }

        public virtual void Dispose()
        {
            foreach (var gameSystem in GameSystems)
            {
                gameSystem.Dispose();
            }
        }

        protected virtual void ConfigureServices(IServiceCollection services)
        {
            Context.ConfigureServices(services);

            services.AddSingleton(this);
            //services.AddSingleton<IContentManager, ContentManager>();
            services.AddSingleton<InputManager>();
        }

        public void Run()
        {
            if (IsRunning)
            {
                throw new InvalidOperationException("This game is already running.");
            }

            try
            {
                // Enter main loop.
                var blocking = Context.Run(InitializeBeforeRun, Tick);

                if (blocking)
                {
                    // If the previous call was blocking, then we can call EndRun
                    //EndRun();
                }
                else
                {
                    // EndRun will be executed on Exit
                    _endRunRequired = true;
                }
            }
            finally
            {
                if (!_endRunRequired)
                {
                    IsRunning = false;
                }
            }
        }

        public void Tick()
        {
        }

        private void InitializeBeforeRun()
        {
            IsRunning = true;
        }
    }
}
