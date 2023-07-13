// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using static SDL2.SDL.SDL_EventType;
using static SDL2.SDL.SDL_LogPriority;
using System.Runtime.InteropServices;
using static SDL2.SDL;

namespace Alimer;

internal unsafe class SDLPlatform : AppPlatform
{
    private SDL_LogOutputFunction logOutputDelegate;

    private const int _eventsPerPeep = 64;
    private readonly SDL_Event[] _events = new SDL_Event[_eventsPerPeep];

    private readonly SDLInput _input;
    private readonly SDLWindow _window;
    private Dictionary<uint, SDLWindow> _idLookup = new();
    private bool _exitRequested;

    public SDLPlatform()
    {
        SDL_LogSetPriority((int)SDL_LogCategory.SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_DEBUG);
        //SDL_LogSetOutputFunction(OnLog);
        SDL_LogSetOutputFunction(logOutputDelegate = logOutput, IntPtr.Zero);

        SDL_GetVersion(out SDL_version version);
        ApiVersion = new Version(version.major, version.minor, version.patch);

        // Init SDL2
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            Log.Error($"Unable to initialize SDL: {SDL_GetError()}");
            throw new Exception("");
        }

        _input = new SDLInput(this);
        MainWindow = (_window = new SDLWindow(this, WindowFlags.Resizable));
        _idLookup.Add(_window.Id, _window);
    }

    public Version ApiVersion { get; }

    // <inheritdoc />
    public override InputManager Input => _input;

    // <inheritdoc />
    public override bool SupportsMultipleViews => true;

    // <inheritdoc />
    public override Window MainWindow { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="SDLPlatform" /> class.
    /// </summary>
    ~SDLPlatform() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            SDL_Quit();
        }
    }

    /// <inheritdoc />
    public override void RunMainLoop()
    {
        OnReady();

        _window.Show();

        while (!_exitRequested)
        {
            PollEvents();
            OnTick();
        }

        SDL_Quit();
    }

    private void PollEvents()
    {
        SDL_PumpEvents();
        int eventsRead;

        do
        {
            eventsRead = SDL_PeepEvents(_events, _eventsPerPeep, SDL_eventaction.SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            for (int i = 0; i < eventsRead; i++)
            {
                HandleSDLEvent(_events[i]);
            }
        } while (eventsRead == _eventsPerPeep);
    }

    private void HandleSDLEvent(SDL_Event evt)
    {
        switch (evt.type)
        {
            case SDL_QUIT:
#if SDL3
            case SDL_EVENT_TERMINATING:
#else
            case SDL_APP_TERMINATING:
#endif
                _exitRequested = true;
                break;


#if SDL3
            default:
                if (evt.type >= SDL_EVENT_WINDOW_FIRST && evt.type <= SDL_EVENT_WINDOW_LAST)
                {
                    HandleWindowEvent(evt);
                }
#else
            case SDL_WINDOWEVENT:
                HandleWindowEvent(evt);
#endif
                break;
        }
    }

    private void HandleWindowEvent(in SDL_Event evt)
    {
        if (_idLookup.TryGetValue(evt.window.windowID, out SDLWindow? window))
        {
            window.HandleEvent(evt);
        }
    }

    internal void WindowClosed(uint windowID)
    {
        _idLookup.Remove(windowID);
    }

    #if SDL3
    private static void OnLog(SDL_LogCategory category, SDL_LogPriority priority, string message)
    {
        Log.Info($"SDL: {message}");
    }
#else
    [MonoPInvokeCallback(typeof(SDL_LogOutputFunction))]
    private static void logOutput(IntPtr _, int categoryInt, SDL_LogPriority priority, IntPtr messagePtr)
    {
        var category = (SDL_LogCategory)categoryInt;
        string? message = Marshal.PtrToStringUTF8(messagePtr);

        Log.Info($@"SDL {category} log [{priority}]: {message}");
    }
#endif
}
