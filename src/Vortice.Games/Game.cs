// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;
using Vortice.Graphics;

namespace Vortice
{
    public abstract class Game : IGame, IDisposable
    {
        private readonly GameContext _context;
        private readonly GamePlatform _platform;
        private readonly ServiceProvider _serviceProvider;

        protected Game(GameContext context)
        {
            _context = context;
            _platform = GamePlatform.Create(this);
            _platform.Activated += GamePlatform_Activated;
            _platform.Deactivated += GamePlatform_Deactivated;

            ServiceCollection services = new();
            context.ConfigureServices(services);
            ConfigureServices(services);

            _serviceProvider = services.BuildServiceProvider();

            // Get services.
            View = _serviceProvider.GetRequiredService<GameView>();

            // Create GraphicsDevice
            GraphicsDevice = GraphicsDevice.Create();
        }

        ~Game()
        {
            Dispose(dispose: false);
        }

        public event EventHandler<EventArgs>? Activated;
        public event EventHandler<EventArgs>? Deactivated;

        public event EventHandler<EventArgs>? Disposed;

        public IServiceProvider Services => _serviceProvider;

        public bool IsActive { get; private set; }

        public bool IsRunning { get; private set; }

        public bool IsDisposed { get; private set; }

        public GameView View { get; }

        public GraphicsDevice? GraphicsDevice { get; }

        public void Dispose()
        {
            Dispose(dispose: true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool dispose)
        {
            if (dispose && !IsDisposed)
            {
                View.SwapChain?.Dispose();
                GraphicsDevice?.Dispose();

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

            _context.RunMainLoop(InitializeBeforeRun, Tick);
        }

        protected virtual void Initialize()
        {
        }

        private void InitializeBeforeRun()
        {
            View.CreateSwapChain(GraphicsDevice!);

            Initialize();
        }

        public void Tick()
        {
            View.SwapChain!.Present();
        }

        /// <summary>
        /// Raises the <see cref="Activated"/> event. Override this method to add code to handle when the game gains focus.
        /// </summary>
        /// <param name="sender">The Game.</param>
        /// <param name="args">Arguments for the Activated event.</param>
        protected virtual void OnActivated(object sender, EventArgs args)
        {
            Activated?.Invoke(this, args);
        }

        /// <summary>
        /// Raises the <see cref="Deactivated"/> event. Override this method to add code to handle when the game loses focus.
        /// </summary>
        /// <param name="sender">The Game.</param>
        /// <param name="args">Arguments for the Deactivated event.</param>
        protected virtual void OnDeactivated(object sender, EventArgs args)
        {
            Deactivated?.Invoke(this, args);
        }


        #region GamePlatform Events
        private void GamePlatform_Activated(object? sender, EventArgs e)
        {
            if (!IsActive)
            {
                IsActive = true;
                OnActivated(this, EventArgs.Empty);
            }
        }

        private void GamePlatform_Deactivated(object? sender, EventArgs e)
        {
            if (IsActive)
            {
                IsActive = false;
                OnDeactivated(this, EventArgs.Empty);
            }
        }
        #endregion
    }
}
