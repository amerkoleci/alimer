// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Alimer.Audio;
using Alimer.Content;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Input;

namespace Alimer;

/// <summary>
/// Alimer Game class.
/// </summary>
public abstract class Game : DisposableObject, IGame
{
    private readonly GamePlatform _platform;
    private readonly ServiceRegistry _services;
    private readonly ContentManager _content;
    private readonly Lock _tickLock = new();
    private readonly Stopwatch _stopwatch = new();
    private readonly GameTime _appTime = new();

    /// <summary>
    /// Initializes a new instance of the <see cref="Game" /> class.
    /// </summary>
    /// <param name="name">The optional name of the application.</param>
    protected Game(GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Default)
    {
        _platform = GamePlatform.CreateDefault();
        PrintSystemInformation();

        _services = new();
        //Platform.ConfigureServices(services);
        _content = new ContentManager(_services);
        ConfigureServices(_services);

        GraphicsManagerOptions graphicsManagerOptions = new()
        {
            PreferredBackend = preferredGraphicsBackend,
            //PowerPreference = GpuPowerPreference.LowPower,
#if DEBUG
            ValidationMode = GraphicsValidationMode.Enabled
#endif
        };

        GraphicsManager = GraphicsManager.Create(in graphicsManagerOptions);
        GraphicsAdapter = GraphicsManager.GetBestAdapter();
        GraphicsDeviceDescription deviceDescription = new()
        {
            MaxFramesInFlight = 2u
        };

        GraphicsDevice = GraphicsAdapter.CreateDevice(in deviceDescription);

        AudioDeviceOptions audioOptions = new();
        AudioDevice = AudioDevice.Create(in audioOptions);

        _services.AddService(GraphicsManager);
        _services.AddService(GraphicsAdapter);
        _services.AddService(GraphicsDevice);
        _services.AddService(AudioDevice);
        _services.AddService(MainWindow);
        SceneSystem = new SceneSystem(Services);
        Services.AddService(SceneSystem);
        GameSystems.Add(SceneSystem);
    }

    /// <summary>
    /// Gets the name of the game.
    /// </summary>
    public virtual string? Name { get; }

    /// <inheritdoc />
    public bool IsRunning { get; private set; }

    /// <inheritdoc />
    public bool IsExiting { get; private set; }

    public GameTime Time => _appTime;

    public IServiceRegistry Services => _services;

    /// <summary>
    /// Gets the content manager.
    /// </summary>
    public IContentManager Content => _content;

    /// <summary>
    /// Gets the main window, automatically created or managed by the <see cref="GamePlatform"/> module.
    /// </summary>
    public Window MainWindow => _platform.MainWindow;

    /// <summary>
    /// Gets the system input, created by the <see cref="GamePlatform"/> module.
    /// </summary>
    public InputManager Input => _platform.Input;

    /// <summary>
    /// Gets the <see cref="Graphics.GraphicsManager"/> created by the application.
    /// </summary>
    public GraphicsManager GraphicsManager { get; }

    /// <summary>
    /// Gets the <see cref="Graphics.GraphicsAdapter"/> used to create the <see cref="GraphicsDevice"/>.
    /// </summary>
    public GraphicsAdapter GraphicsAdapter { get; }

    /// <summary>
    /// Gets the <see cref="Graphics.GraphicsDevice"/> created by the application.
    /// </summary>
    public GraphicsDevice GraphicsDevice { get; }

    /// <summary>
    /// Gets the <see cref="Audio.AudioDevice"/> instance.
    /// </summary>
    public AudioDevice AudioDevice { get; private set; }

    /// <summary>
    /// Get the list of game systems.
    /// </summary>
    public List<IGameSystem> GameSystems { get; } = [];

    /// <summary>
    /// Gets the <see cref="SceneSystem"/> instance.
    /// </summary>
    public SceneSystem SceneSystem { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            // Dispose game systems first.
            foreach (IGameSystem system in GameSystems)
            {
                if (system is IDisposable disposable)
                {
                    disposable.Dispose();
                }
            }

            GraphicsDevice.WaitIdle();
            MainWindow.Destroy();
            GraphicsDevice.Dispose();
            AudioDevice.Dispose();
            GraphicsManager.Dispose();
        }
    }

    public virtual void ConfigureServices(IServiceRegistry services)
    {
        services.AddService<IGame>(this);
        services.AddService<IContentManager>(_content);
    }

    public void Run()
    {
        if (IsRunning)
        {
            throw new InvalidOperationException("This application is already running.");
        }

        if (IsExiting)
        {
            throw new InvalidOperationException("App is still exiting");
        }

#if DEBUG
        Launch();
#else
        try
        {
            Launch();
        }
        catch (Exception e)
        {
            string path = _platform.UserDirectory(Name);

            Log.Error(e.Message);
            Log.WriteToFile(Path.Combine(path, $"{Name}.txt"));
            throw;
        }
#endif
        void Launch()
        {
            // Startup application
            _platform.Ready = OnPlatformReady;
            _platform.TickRequested += OnTickRequested;
            _platform.RunMainLoop();
        }

        IsRunning = true;
    }

    public void RequestExit()
    {
        if (IsRunning && !IsExiting)
        {
            _platform.RequestExit();
            IsExiting = true;
        }
    }

    protected virtual void Initialize()
    {
    }

    public virtual Task LoadContentAsync()
    {
        return Task.CompletedTask;
    }

    protected virtual void BeginRun()
    {
    }

    protected virtual void EndRun()
    {
    }

    protected virtual void Update(GameTime time)
    {
        foreach (IGameSystem system in GameSystems)
        {
            system.Update(time);
        }
    }

    protected virtual void BeginDraw()
    {
        foreach (IGameSystem system in GameSystems)
        {
            system.BeginDraw();
        }
    }

    protected virtual void Draw(CommandBuffer commandBuffer, Texture outputTexture, GameTime time)
    {
        // Draw for all game systems
        foreach (IGameSystem system in GameSystems)
        {
            system.Draw(commandBuffer, outputTexture, time);
        }
    }

    protected virtual void EndDraw()
    {
        // End drawing for all game systems
        foreach (IGameSystem system in GameSystems)
        {
            system.EndDraw();
        }

        _ = GraphicsDevice.CommitFrame();
    }

    public void Tick()
    {
        lock (_tickLock)
        {
            if (IsExiting)
            {
                CheckEndRun();
                return;
            }

            try
            {
                TimeSpan elapsedTime = _stopwatch.Elapsed - Time.Total;
                _appTime.Update(_stopwatch.Elapsed, elapsedTime);

                Update(_appTime);

                BeginDraw();

                // Begin rendering commands
                CommandBuffer commandBuffer = GraphicsDevice.AcquireCommandBuffer(CommandQueueType.Graphics, "Frame"u8);
                Texture? swapChainTexture = MainWindow.SwapChain!.GetCurrentTexture();
                if (swapChainTexture is not null)
                {
                    Draw(commandBuffer, swapChainTexture, _appTime);
                }

                commandBuffer.Present(MainWindow.SwapChain!);

                // Execute 
                GraphicsDevice.GraphicsCommandQueue.Execute(commandBuffer);
            }
            finally
            {
                EndDraw();

                CheckEndRun();
            }
        }
    }

    private void InitializeBeforeRun()
    {
        MainWindow.CreateSwapChain(GraphicsDevice);
        IsRunning = true;

        Initialize();
        LoadContentAsync();

        _stopwatch.Start();
        _appTime.Update(_stopwatch.Elapsed, TimeSpan.Zero);

        BeginRun();
    }

    private void CheckEndRun()
    {
        if (IsExiting && IsRunning)
        {
            EndRun();

            _stopwatch.Stop();

            IsRunning = false;
            IsExiting = false;
        }
    }

    private void OnPlatformReady()
    {
        InitializeBeforeRun();
    }

    private void OnTickRequested(object? sender, EventArgs e)
    {
        Tick();
    }

    private static void PrintSystemInformation()
    {
        Log.Info($"Platform: {RuntimeInformation.OSDescription} ({RuntimeInformation.OSArchitecture})");
        Log.Info($"Framework: {RuntimeInformation.FrameworkDescription}");
        Log.Info($"Runtime Identifier - {RuntimeInformation.RuntimeIdentifier}");
        Log.Info($"Process Architecture - {RuntimeInformation.ProcessArchitecture}");
    }
}
