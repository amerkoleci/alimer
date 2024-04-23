// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using static SDL.SDL3;
using SDL;
using System.Runtime.InteropServices;
using static SDL.SDL_InitFlags;
using static SDL.SDL_EventType;
using System.Runtime.CompilerServices;

namespace Alimer;

internal unsafe class SDLPlatform : AppPlatform
{
    private const int _eventsPerPeep = 64;
    private readonly SDL_Event* _events = (SDL_Event*)NativeMemory.Alloc(_eventsPerPeep, (nuint)sizeof(SDL_Event));

    private readonly SDLInput _input;
    private readonly SDLWindow _window;
    private Dictionary<SDL_WindowID, SDLWindow> _idLookup = [];
    private bool _exitRequested;

    public SDLPlatform()
    {
        //SDL_LogSetPriority(SDL_LogCategory.Error, SDL_LogPriority.Debug);
        //SDL_LogSetPriority(SDL_LogCategory.SDL_LOG_CATEGORY_ERROR, SDL_LogPriority.SDL_LOG_PRIORITY_DEBUG);
        SDL_SetLogOutputFunction(&OnLog, IntPtr.Zero);

        SDL_Version version;
        SDL_GetVersion(&version);
        ApiVersion = new Version(version.major, version.minor, version.patch);

        // Init SDL3
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
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

    /// <inheritdoc />
    public override void RequestExit()
    {
        _exitRequested = true;
    }

    private void PollEvents()
    {
        SDL_PumpEvents();
        int eventsRead;

        do
        {
            eventsRead = SDL_PeepEvents(_events, _eventsPerPeep, SDL_eventaction.SDL_GETEVENT, (uint)SDL_EVENT_FIRST, (uint)SDL_EVENT_LAST);
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
            case (uint)SDL_EVENT_QUIT:
            case (uint)SDL_EVENT_TERMINATING:
                _exitRequested = true;
                break;

            default:
                if (evt.type >= (uint)SDL_EVENT_WINDOW_FIRST && evt.type <= (uint)SDL_EVENT_WINDOW_LAST)
                {
                    HandleWindowEvent(evt);
                }
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

    internal void WindowClosed(SDL_WindowID windowID)
    {
        _idLookup.Remove(windowID);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void OnLog(IntPtr _, SDL_LogCategory category, SDL_LogPriority priority, byte* messagePtr)
    {
        string? message = PtrToStringUTF8(messagePtr);
        Log.Info($"SDL: {message}");
    }
}
