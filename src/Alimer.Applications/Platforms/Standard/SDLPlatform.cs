// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using static Alimer.AlimerApi;

namespace Alimer;

internal unsafe class SDLPlatform : AppPlatform
{
    private readonly SDLInput _input;
    private readonly SDLWindow _window;
    private Dictionary<uint, SDLWindow> _idLookup = [];
    private bool _exitRequested;

    public SDLPlatform()
    {
        // Init platform layer
        if (!alimerPlatformInit())
        {
            throw new InvalidOperationException($"Failed to initialise platform layer");
        }

        _input = new SDLInput(this);
        MainWindow = (_window = new SDLWindow(this, WindowFlags.Resizable));
        _idLookup.Add(_window.Id, _window);
    }

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
            while (alimerPollEvent(out Event @event))
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

    private void HandleEvent(in Event evt)
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
        }
    }

    private void FetchDisplays()
    {
    }

    //private void HandleDisplayEvent(SDL_DisplayEvent _) => FetchDisplays();

    private void HandleWindowEvent(in WindowEvent evt)
    {
        if (_idLookup.TryGetValue(evt.windowID, out SDLWindow? window))
        {
            window.HandleEvent(evt);
        }
    }

    internal void WindowClosed(uint windowID)
    {
        _idLookup.Remove(windowID);
    }
}
