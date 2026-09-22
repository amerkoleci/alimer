// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Input;
using static SDL3;
using static SDL3.SDL_EventAction;
using static SDL3.SDL_EventType;
using static SDL3.SDL_InitFlags;
using static SDL3.SDL_LogCategory;
using static SDL3.SDL_LogPriority;

namespace Alimer;

internal unsafe class SDLPlatform : GamePlatform
{
    private const int EventsPerPeep = 64;
    private readonly SDL_Event[] _events = new SDL_Event[EventsPerPeep];
    private readonly SDLInputManager _input;

    private readonly Window _window;
    private readonly Dictionary<SDL_WindowID, Window> _idLookup = [];
    private bool _exitRequested;

    public SDLPlatform(Game game, string appName = "Alimer")
        : base(game)
    {
#if DEBUG
        SDL_SetLogPriority((int)SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_DEBUG);
#endif
        SDL_SetLogOutputFunction(&AlimerLog_SDL, 0);

        if (!string.IsNullOrEmpty(appName))
        {
            SDL_SetHint(SDL_HINT_APP_NAME, appName);
        }

        //SDL_SetHint(SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4, "0");
        //SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

        SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0"); // Disable touch events generating synthetic mouse events on desktop platforms
        SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0"); // Disable mouse events generating synthetic touch events on mobile platforms
        SDL_SetHint(SDL_HINT_PEN_TOUCH_EVENTS, "0");
        SDL_SetHint(SDL_HINT_PEN_MOUSE_EVENTS, "0");
        SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");

        // Init SDL
        //SDL_SetMainReady();
        SDL_InitFlags sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
        if (!SDL_Init(sdl_init_flags))
        {
            throw new InvalidOperationException($"Alimer: SDL_Init Failed: {SDL_GetError()}");
        }

        int version = SDL_GetVersion();
        Log.Info($@"SDL3 Initialized, Version: {SDL_VERSIONNUM_MAJOR(version)}.{SDL_VERSIONNUM_MINOR(version)}.{SDL_VERSIONNUM_MICRO(version)}, Video driver: {SDL_GetCurrentVideoDriver()}");



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

            PollEvents();

            if (_exitRequested)
                break;

            OnTick();
        }

        //alimerPlatformShutdown();
    }

    private void PollEvents()
    {
        SDL_PumpEvents();

        int eventsRead;

        do
        {
            eventsRead = SDL_PeepEvents(_events, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST).LogErrorIfFailed();
            for (int i = 0; i < eventsRead; i++)
            {
                HandleEvent(_events[i]);
            }

        } while (eventsRead == EventsPerPeep);
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
        SDL_Quit();
    }

    private void HandleEvent(in SDL_Event evt)
    {
        //if (evt.type >= EventType.DisplayFirst && evt.type <= SDL_EventType.DisplayLast)
        //{
        //    HandleDisplayEvent(evt.display);
        //    return;
        //}

        switch (evt.type)
        {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_TERMINATING:
                _exitRequested = true;
                break;

            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                _input.HandleEvent(in evt);
                break;

            //case SDL_EVENT_AUDIO_DEVICE_ADDED:
            //case SDL_EVENT_AUDIO_DEVICE_REMOVED:
            //    AudioSystem.ScanDevices();
            //    break;

            default:
                if (evt.type >= SDL_EVENT_WINDOW_FIRST
                    && evt.type <= SDL_EVENT_WINDOW_LAST)
                {
                    HandleWindowEvent(in evt.window);
                }
                else
                {
                    // Process event by input manager
                    _input.HandleEvent(in evt);
                }
                break;
        }
    }

    private void FetchDisplays()
    {
    }

    //private void HandleDisplayEvent(SDL_DisplayEvent _) => FetchDisplays();

    private void HandleWindowEvent(in SDL_WindowEvent evt)
    {
        if (_idLookup.TryGetValue(evt.windowID, out Window? window))
        {
            window.HandleEvent(evt);
        }
    }

    internal void WindowClosed(SDL_WindowID windowID)
    {
        _idLookup.Remove(windowID);
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static void AlimerLog_SDL(nint userData, int category, SDL_LogPriority priority, byte* messagePtr)
    {
        string message = PtrToStringUTF8(messagePtr, false)!;
        switch (priority)
        {
            case SDL_LOG_PRIORITY_VERBOSE:
                Log.Trace(message);
                break;
            case SDL_LOG_PRIORITY_DEBUG:
                Log.Debug(message);
                break;
            case SDL_LOG_PRIORITY_INFO:
                Log.Info(message);
                break;
            case SDL_LOG_PRIORITY_WARN:
                Log.Warn(message);
                break;
            case SDL_LOG_PRIORITY_ERROR:
                Log.Error(message);
                break;
            case SDL_LOG_PRIORITY_CRITICAL:
                Log.Critical(message);
                break;
            default:
                break;
        }
    }
}

partial class GamePlatform
{
    public static GamePlatform CreateDefault(Game game) => new SDLPlatform(game);
}
