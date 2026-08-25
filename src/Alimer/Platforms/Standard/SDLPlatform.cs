// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using static Alimer.AlimerApi;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_EventAction;
using static Alimer.SDL3.SDL_EventType;

namespace Alimer;

internal unsafe class SDLPlatform : GamePlatform
{
    private readonly SDLInputManager _input;

    private readonly Window _window;
    private readonly Dictionary<uint, Window> _idLookup = [];
    private bool _exitRequested;

    public SDLPlatform(Game game, string appName = "Alimer")
        : base(game)
    {
        // Init SDL_ platform layer
        if (!alimerPlatformInit())
        {
            throw new InvalidOperationException($"Alimer: SDL_Init Failed: {SDL_GetError()}");
        }

        _input = new SDLInputManager();
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

            if (_exitRequested)
                break;

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
        Cursors.Shutdown();
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
                switch (evt.window.type)
                {
                    case WindowEventType.MouseEnter:
                        _input.HandleWindowMouseEnterOrLeaveEvent(in evt, true);
                        break;

                    case WindowEventType.MouseLeave:
                        _input.HandleWindowMouseEnterOrLeaveEvent(in evt, false);
                        break;

                    default:
                        HandleWindowEvent(in evt.window);
                        break;
                }

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
    public static GamePlatform CreateDefault(Game game) => new SDLPlatform(game);
}
