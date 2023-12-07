// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using static SDL.SDL;
using SDL;
using System.Runtime.InteropServices;

namespace Alimer;

internal unsafe class SDLPlatform : AppPlatform
{
    private const int _eventsPerPeep = 64;
    private readonly SDL_Event* _events = (SDL_Event*)NativeMemory.Alloc(_eventsPerPeep, (nuint)sizeof(SDL_Event));

    private readonly SDLInput _input;
    private readonly SDLWindow _window;
    private Dictionary<uint, SDLWindow> _idLookup = new();
    private bool _exitRequested;

    public SDLPlatform()
    {
        //SDL_LogSetPriority(SDL_LogCategory.Error, SDL_LogPriority.Debug);
        SDL_LogSetOutputFunction(OnLog);

        SDL_GetVersion(out SDL_version version);
        ApiVersion = new Version(version.major, version.minor, version.patch);

        // Init SDL3
        if (SDL_Init(SDL_InitFlags.Video | SDL_InitFlags.Timer | SDL_InitFlags.Gamepad) != 0)
        {
            Log.Error($"Unable to initialize SDL: {SDL_GetErrorString()}");
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
            eventsRead = SDL_PeepEvents(_events, _eventsPerPeep, SDL_eventaction.GetEvent, SDL_EventType.First, SDL_EventType.Last);
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
            case SDL_EventType.Quit:
            case SDL_EventType.Terminating:
                _exitRequested = true;
                break;

            default:
                if (evt.type >= SDL_EventType.WindowFirst && evt.type <= SDL_EventType.WindowLast)
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

    internal void WindowClosed(uint windowID)
    {
        _idLookup.Remove(windowID);
    }

    private static void OnLog(SDL_LogCategory category, SDL_LogPriority priority, string message)
    {
        Log.Info($"SDL: {message}");
    }
}
