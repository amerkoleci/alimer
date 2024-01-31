// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Alimer.Audio;
using Alimer.Content;
using Alimer.Graphics;
using Alimer.Input;

namespace Alimer;

/// <summary>
/// Alimer Application class.
/// </summary>
public abstract class Application : DisposableObject, IApplication
{
    private readonly ServiceRegistry _services;
    private readonly ContentManager _content;
    private readonly object _tickLock = new();
    private readonly Stopwatch _stopwatch = new();
    private readonly AppTime _appTime = new();

    /// <summary>
    /// Initializes a new instance of the <see cref="Application" /> class.
    /// </summary>
    /// <param name="name">The optional name of the application.</param>
    protected Application(AppPlatform? platform = default,
        GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count,
        string? name = default)
    {
        Platform = platform ?? AppPlatform.CreateDefault();
        Name = name ?? GetType().Name;
        PrintSystemInformation();

        _services = new();
        //Platform.ConfigureServices(services);
        _content = new ContentManager(_services);
        ConfigureServices(_services);

        GraphicsDeviceDescription deviceDescription = new()
        {
            PreferredBackend = preferredGraphicsBackend,
            //PowerPreference = GpuPowerPreference.LowPower,
#if DEBUG
            ValidationMode = ValidationMode.Enabled
#endif
        };

        GraphicsDevice = GraphicsDevice.CreateDefault(in deviceDescription);

        AudioDeviceOptions audioOptions = new();
        AudioDevice = new(in audioOptions);

        _services.AddService(GraphicsDevice);
        _services.AddService(AudioDevice);
        _services.AddService(MainWindow);
    }

    /// <summary>
    /// Gets the Application name.
    /// </summary>
    public string Name { get; }

    /// <inheritdoc />
    public bool IsRunning { get; private set; }

    /// <inheritdoc />
    public bool IsExiting { get; private set; }

    public AppTime Time => _appTime;

    public IServiceRegistry Services => _services;

    public IContentManager Content => _content;

    /// <summary>
    /// Gets the platform module.
    /// </summary>
    public AppPlatform Platform { get; }

    /// <summary>
    /// Gets the main window, automatically created or managed by the <see cref="AppPlatform"/> module.
    /// </summary>
    public Window MainWindow => Platform.MainWindow;

    /// <summary>
    /// Gets the system input, created by the <see cref="AppPlatform"/> module.
    /// </summary>
    public InputManager Input => Platform.Input;

    /// <summary>
    /// Gets the <see cref="Graphics.GraphicsDevice"/> Module
    /// </summary>
    public GraphicsDevice GraphicsDevice { get; }

    /// <summary>
    /// Gets the <see cref="AudioDevice"/> instance.
    /// </summary>
    public AudioDevice AudioDevice { get; private set; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            GraphicsDevice.WaitIdle();
            MainWindow.Destroy();
            GraphicsDevice.Dispose();
            AudioDevice.Dispose();
        }
    }

    public virtual void ConfigureServices(IServiceRegistry services)
    {
        services.AddService<IApplication>(this);
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
            string path = Platform.UserDirectory(Name);

            Log.Error(e.Message);
            Log.WriteToFile(Path.Combine(path, $"{Name}.txt"));
            throw;
        }
#endif
        void Launch()
        {
            // Startup application
            Platform.Ready = OnPlatformReady;
            Platform.TickRequested += OnTickRequested;
            Platform.RunMainLoop();
        }

        IsRunning = true;
    }

    public void RequestExit()
    {
        if (IsRunning && !IsExiting)
        {
            Platform.RequestExit();
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

    protected virtual void Update(AppTime time)
    {
    }

    protected virtual void BeginDraw()
    {

    }

    protected virtual void Draw(RenderContext renderContext, Texture outputTexture, AppTime time)
    {

    }

    protected virtual void EndDraw()
    {
        GraphicsDevice.FinishFrame();
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
                RenderContext renderContext = GraphicsDevice.BeginRenderContext("Frame");
                Texture? swapChainTexture = MainWindow.SwapChain!.GetCurrentTexture();
                if (swapChainTexture is not null)
                {
                    Draw(renderContext, swapChainTexture, _appTime);
                }

                renderContext.Present(MainWindow.SwapChain!);

                //GraphicsDevice.Submit(commandBuffer);
                renderContext.Flush(waitForCompletion: false);
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
