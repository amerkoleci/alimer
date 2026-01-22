// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using static Alimer.AlimerApi;

namespace Alimer;

internal unsafe class NativePlatform : GamePlatform
{
    private readonly NativeInput _input;
    private readonly Window _window;
    private readonly Dictionary<uint, Window> _idLookup = [];
    private bool _exitRequested;

    public NativePlatform()
    {
        // Init platform layer
        if (!alimerPlatformInit())
        {
            throw new InvalidOperationException($"Failed to initialise platform layer");
        }

        _input = new NativeInput();
        MainWindow = (_window = new Window(this, WindowFlags.Resizable));
        _idLookup.Add(_window.Id, _window);
    }

    // <inheritdoc />
    public override IInputSourceConfiguration InputConfiguration => _input;

    // <inheritdoc />
    public override bool SupportsMultipleViews => true;

    // <inheritdoc />
    public override Window MainWindow { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="SDLPlatform" /> class.
    /// </summary>
    ~NativePlatform() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            alimerPlatformShutdown();
        }
    }

    /// <inheritdoc />
    public override void RunMainLoop()
    {
        OnReady();

        _window.Show();

        while (!_exitRequested)
        {
            PlatformEvent @event = default;
            while (alimerPlatformPollEvent(&@event))
            {
                HandleEvent(in @event);
            }
            OnTick();
        }

        //alimerPlatformShutdown();
    }

    /// <inheritdoc />
    public override void RequestExit()
    {
        _exitRequested = true;
    }

    private void HandleEvent(in PlatformEvent evt)
    {
        //if (evt.type >= EventType.DisplayFirst && evt.type <= SDL_EventType.DisplayLast)
        //{
        //    HandleDisplayEvent(evt.display);
        //    return;
        //}

        switch (evt.type)
        {
            case EventType.Quit:
            case EventType.Terminating:
                _exitRequested = true;
                break;
            case EventType.Window:
                HandleWindowEvent(in evt.window);
                break;
            case EventType.KeyDown:
            case EventType.KeyUp:
                _input.HandleKeyEvent(in evt.key, evt.type == EventType.KeyDown);
                break;
        }
    }

    private void FetchDisplays()
    {
    }

    //private void HandleDisplayEvent(SDL_DisplayEvent _) => FetchDisplays();

    private void HandleWindowEvent(in WindowEvent evt)
    {
        if (_idLookup.TryGetValue(evt.windowID, out Window? window))
        {
            window.HandleEvent(evt);
        }
    }

    internal void WindowClosed(uint windowID)
    {
        _idLookup.Remove(windowID);
    }
}

partial class GamePlatform
{
    public static GamePlatform CreateDefault() => new NativePlatform();
}
