// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using static Alimer.AlimerApi;
using System.Runtime.InteropServices.Marshalling;

namespace Alimer;

internal unsafe class NativePlatform : GamePlatform
{
    private readonly NativeInputManager _input;
    private readonly Window _window;
    private readonly Dictionary<uint, Window> _idLookup = [];
    private bool _exitRequested;

    public NativePlatform(Game game, string appName = "Alimer")
        : base(game)
    {
        // Init platform layer
        if (!alimerPlatformInit())
        {
            throw new InvalidOperationException($"Failed to initialise platform layer");
        }

        _input = new NativeInputManager();
        MainWindow = (_window = new Window(this, WindowFlags.Resizable));
        _idLookup.Add(_window.Id, _window);
    }

    // <inheritdoc />
    public override InputManager Input => _input;

    // <inheritdoc />
    public override Window MainWindow { get; }

    /// <inheritdoc />
    public override void RunMainLoop()
    {
        OnReady();

        _window.Show();

        while (!_exitRequested)
        {
            _input.BeginFrame();

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

    /// <inheritdoc />
    public override void Destroy()
    {
        alimerPlatformShutdown();
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

            default:
                // Process event by input manager
                _input.HandleEvent(in evt);
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
    public static GamePlatform CreateDefault(Game game) => new NativePlatform(game);
}
