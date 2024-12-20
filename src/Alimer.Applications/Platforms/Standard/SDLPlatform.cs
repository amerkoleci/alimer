// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using static SDL3.SDL3;
using SDL3;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using static SDL3.SDL_EventAction;

namespace Alimer;

internal unsafe class SDLPlatform : AppPlatform
{
    private const int _eventsPerPeep = 64;
    private readonly SDL_Event[] _events = new SDL_Event[_eventsPerPeep];

    private readonly SDLInput _input;
    private readonly SDLWindow _window;
    private Dictionary<SDL_WindowID, SDLWindow> _idLookup = [];
    private bool _exitRequested;

    public SDLPlatform()
    {
        //SDL_LogSetPriority(SDL_LogCategory.Error, SDL_LogPriority.Debug);
        //SDL_LogSetPriority(SDL_LogCategory.SDL_LOG_CATEGORY_ERROR, SDL_LogPriority.SDL_LOG_PRIORITY_DEBUG);
        SDL_SetLogOutputFunction(&OnLog, 0);

        int version = SDL_GetVersion();
        string? revision = SDL_GetRevision();
        ApiVersion = new Version(
            SDL_VERSIONNUM_MAJOR(version),
            SDL_VERSIONNUM_MINOR(version),
            SDL_VERSIONNUM_MICRO(version));

        // Init SDL3
        if (!SDL_Init((SDL_InitFlags)(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)))
        {
            Log.Error($"Unable to initialize SDL: {SDL_GetError()}");
            throw new Exception("");
        }

        Log.Info($@"SDL3 Initialized
                  SDL3 Version: {SDL_VERSIONNUM_MAJOR(version)}.{SDL_VERSIONNUM_MINOR(version)}.{SDL_VERSIONNUM_MICRO(version)}
                  SDL3 Revision: {SDL_GetRevision()}"
                  );

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
            eventsRead = SDL_PeepEvents(_events, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);
            for (int i = 0; i < eventsRead; i++)
            {
                HandleSDLEvent(_events[i]);
            }
        } while (eventsRead == _eventsPerPeep);
    }

    private void HandleSDLEvent(SDL_Event evt)
    {
        if (evt.type >= SDL_EventType.DisplayFirst && evt.type <= SDL_EventType.DisplayLast)
        {
            HandleDisplayEvent(evt.display);
            return;
        }

        if (evt.type >= SDL_EventType.WindowFirst && evt.type <= SDL_EventType.WindowLast)
        {
            HandleWindowEvent(in evt.window);
            return;
        }

        switch (evt.type)
        {
            case SDL_EventType.Quit:
            case SDL_EventType.Terminating:
                _exitRequested = true;
                break;
        }
    }

    private void FetchDisplays()
    {
    }

    private void HandleDisplayEvent(SDL_DisplayEvent _) => FetchDisplays();

    private void HandleWindowEvent(in SDL_WindowEvent evt)
    {
        if (_idLookup.TryGetValue(evt.windowID, out SDLWindow? window))
        {
            window.HandleEvent(evt);
        }
    }

    internal void WindowClosed(SDL_WindowID windowID)
    {
        _idLookup.Remove(windowID);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void OnLog(nint userdata, int category, SDL_LogPriority priority, byte* messagePtr)
    {
        string message = ConvertToManaged(messagePtr)!;
        Log.Info($"SDL: {message}");
    }
}
