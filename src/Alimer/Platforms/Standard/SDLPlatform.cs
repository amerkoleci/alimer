// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_LogCategory;
using static Alimer.SDL3.SDL_LogPriority;
using static Alimer.SDL3.SDL_InitFlags;
using static Alimer.SDL3.SDL_EventAction;
using static Alimer.SDL3.SDL_EventType;
using System.Runtime.InteropServices.Marshalling;

namespace Alimer;

internal unsafe class SDLPlatform : GamePlatform
{
    private const int EventsPerPeep = 64;
    private readonly SDL_Event[] _events = new SDL_Event[EventsPerPeep];

    private readonly SDLInputManager _input;

    private readonly Window _window;
    private readonly Dictionary<uint, Window> _idLookup = [];
    private bool _exitRequested;

    public SDLPlatform(Game game, string appName = "Alimer")
        : base(game)
    {
        SDL_SetHint(SDL_HINT_APP_NAME, appName).LogErrorIfFailed();

        //SDL_SetHint(SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4, "0");
        //SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        //SDL_SetHint(SDL_HINT_APP_NAME, settings.name.c_str());

#if DEBUG
        SDL_SetLogPriority((int)SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_DEBUG);
#endif
        SDL_SetLogOutputFunction(&OnNativeSDLLogCallback, 0);

        // Init SDL_ platform layer
        SDL_InitFlags sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
        if (!SDL_Init(sdl_init_flags))
        {
            throw new InvalidOperationException($"Alimer: SDL_Init Failed: {SDL_GetError()}");
        }

        int version = SDL_GetVersion();
        Log.Info($@"SDL3 Initialized
                          SDL3 Version: {SDL_VERSIONNUM_MAJOR(version)}.{SDL_VERSIONNUM_MINOR(version)}.{SDL_VERSIONNUM_MICRO(version)}
                          SDL3 Revision: {SDL_GetRevision()}
                          SDL3 Video driver: {SDL_GetCurrentVideoDriver()}");

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

            SDL_Event @event = default;
            while (SDL_PollEvent(&@event) && @event.type != SDL_EVENT_POLL_SENTINEL)
            {
                HandleEvent(in @event);
            }

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

    internal void WindowClosed(uint windowID)
    {
        _idLookup.Remove(windowID);
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static void OnNativeSDLLogCallback(nint _, int category, SDL_LogPriority priority, byte* messagePtr)
    {
        SDL_LogCategory categoryEnum = (SDL_LogCategory)category;
        string? message = Utf8StringMarshaller.ConvertToManaged(messagePtr)!;

        switch (priority)
        {
            case SDL_LOG_PRIORITY_VERBOSE:
            case SDL_LOG_PRIORITY_DEBUG:
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
                Log.Fatal(message);
                break;
        }
    }
}

partial class GamePlatform
{
    public static GamePlatform CreateDefault(Game game) => new SDLPlatform(game);
}
