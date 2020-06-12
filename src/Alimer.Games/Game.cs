// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Alimer.Graphics;
using Alimer.Input;
using Microsoft.Extensions.DependencyInjection;
using static Alimer.Graphics.VGPU;

namespace Alimer
{
    public abstract class Game : IDisposable
    {
        private readonly object _tickLock = new object();
        private bool _isExiting;
        private readonly Stopwatch _stopwatch = new Stopwatch();
        private bool _endRunRequired;

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
            Guard.ThrowIfNull(context, nameof(context));

            Context = context;

            // Configure and build services
            ServiceCollection services = new ServiceCollection();

            context.ConfigureServices(services);
            ConfigureServices(services);

            Services = services.BuildServiceProvider();

            // Get required services.
            Input = Services.GetRequiredService<InputManager>();

            // Setup graphics.
            vgpuSetLogCallback(GPULogCallback, IntPtr.Zero);

            GPUDevice = vgpuCreateDevice(BackendType.Default, new GPUDeviceInfo
            {
                Debug = false,
                DevicePreference = GPUDevicePreference.HighPerformance,
                SwapchainInfo = new SwapchainInfo
                {
                    WindowHandle = context.GameWindow.Handle
                }
            });

            vgpuGetDeviceCaps(GPUDevice, out var caps);
        }

        public GameContext Context { get; }
        public IServiceProvider Services { get; }

        public GPUDevice GPUDevice { get; }

        public InputManager Input { get; }

        public virtual void Dispose()
        {
            foreach (var gameSystem in GameSystems)
            {
                gameSystem.Dispose();
            }

            vgpuDestroyDevice(GPUDevice);
        }

        private static void GPULogCallback(IntPtr userData, GPULogLevel level, string message)
        {
            Console.WriteLine($"{level}: {message}");
            System.Diagnostics.Debug.WriteLine($"{level}: {message}");
        }

        protected virtual void ConfigureServices(IServiceCollection services)
        {
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
            vgpuBeginFrame(GPUDevice);

            var renderPass = new RenderPassDescription();
            renderPass.colorAttachments0.clearColor = new Color4(1.0f, 0.0f, 1.0f);
            renderPass.colorAttachments0.loadOp = LoadOp.Clear;
            renderPass.colorAttachments0.texture = vgpuGetBackbufferTexture(GPUDevice);
            vgpuCmdBeginRenderPass(GPUDevice, renderPass);
            vgpuCmdEndRenderPass(GPUDevice);
            vgpuEndFrame(GPUDevice);
        }

        private void InitializeBeforeRun()
        {
            IsRunning = true;
        }
    }
}
