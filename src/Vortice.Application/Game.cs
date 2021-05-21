// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;

namespace Vortice
{
    public abstract partial class Game : IDisposable
    {
        private GameContext _context;
        private readonly ServiceProvider _serviceProvider;
        private bool _isInitialized;

        protected Game(GameContext context)
        {
            _context = context;
            ServiceCollection services = new();

            context.ConfigureServices(services);
            ConfigureServices(services);

            _serviceProvider = services.BuildServiceProvider();
        }

        public event EventHandler<EventArgs>? Disposed;

        public IServiceProvider Services => _serviceProvider;

        public bool IsRunning { get; private set; }

        public bool IsDisposed { get; private set; }

        ~Game()
        {
            Dispose(dispose: false);
        }

        public void Dispose()
        {
            Dispose(dispose: true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool dispose)
        {
            if (dispose && !IsDisposed)
            {
                Disposed?.Invoke(this, EventArgs.Empty);

                IsDisposed = true;
            }
        }

        protected virtual void ConfigureServices(IServiceCollection services)
        {
            //services.AddSingleton<IGame>(this);
            //services.AddSingleton<IContentManager, ContentManager>();
        }

        public void Run()
        {
            if (IsRunning)
            {
                throw new InvalidOperationException("This game is already running.");
            }

            IsRunning = true;

            _context.RunMainLoop(Tick);

            Initialize();
        }

        protected virtual void Initialize()
        {
        }

        public void Tick()
        {
            if (_isInitialized)
            {
                Initialize();
                _isInitialized = true;
            }
        }
    }
}
