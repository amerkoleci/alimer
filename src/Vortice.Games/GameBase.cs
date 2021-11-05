// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.Extensions.DependencyInjection;
using Vortice.Audio;
using Vortice.Graphics;
using Vortice.Input;

namespace Vortice
{
    public abstract class GameBase : IGame, IDisposable
    {
        private readonly GameContext _context;
        private readonly ServiceProvider _serviceProvider;
        private readonly object _tickLock = new();
        private readonly Stopwatch _stopwatch = new();
        private bool _isExiting;

        protected GameBase(GameContext context)
        {
            _context = context;

            ServiceCollection services = new();
            context.ConfigureServices(services);
            ConfigureServices(services);

            _serviceProvider = services.BuildServiceProvider();

            // Get required services.
            View = _serviceProvider.GetRequiredService<GameView>();
            Input = _serviceProvider.GetRequiredService<InputManager>();

            // Get optional services.
            Audio = _serviceProvider.GetService<AudioSystem>();
        }

        ~GameBase()
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

        public GameTime Time { get; } = new GameTime();

        public GameView View { get; }

        public InputManager Input { get; }
        public AudioSystem? Audio { get; }


        public IList<IGameSystem> GameSystems { get; } = new List<IGameSystem>();

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
            services.AddSingleton<IGame>(this);
            //services.AddSingleton<IContentManager, ContentManager>();

            services.AddSingleton<InputManager>();
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
            Initialize();

            _stopwatch.Start();
            Time.Update(_stopwatch.Elapsed, TimeSpan.Zero);

            BeginRun();
        }

        public void Tick()
        {
            lock (_tickLock)
            {
                if (_isExiting)
                {
                    CheckEndRun();
                    return;
                }

                try
                {
                    TimeSpan elapsedTime = _stopwatch.Elapsed - Time.Total;
                    Time.Update(_stopwatch.Elapsed, elapsedTime);

                    Update(Time);

                    BeginDraw();
                    Draw(Time);
                }
                finally
                {
                    EndDraw();

                    CheckEndRun();
                }
            }
        }

        public virtual void BeginRun()
        {
        }

        public virtual void EndRun()
        {
        }

        public virtual void Update(GameTime gameTime)
        {
            foreach (IGameSystem system in GameSystems)
            {
                system.Update(gameTime);
            }
        }

        public virtual void BeginDraw()
        {
            foreach (IGameSystem system in GameSystems)
            {
                system.BeginDraw();
            }
        }

        public virtual void Draw(GameTime gameTime)
        {
            foreach (IGameSystem system in GameSystems)
            {
                system.Draw(gameTime);
            }
        }

        public virtual void EndDraw()
        {
            foreach (IGameSystem system in GameSystems)
            {
                system.EndDraw();
            }

            View.Present();
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

        private void CheckEndRun()
        {
            if (_isExiting && IsRunning)
            {
                EndRun();

                _stopwatch.Stop();

                IsRunning = false;
                _isExiting = false;
            }
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
