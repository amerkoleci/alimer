// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_EventType;

namespace Alimer.Input;

internal unsafe class SDLPointerInputSource : PointerInputSource
{
    private const int MaxMouseButtons = (int)MouseButton.X2 + 1;

    private readonly SDLCursor _defaultCursor;
    private SDLCursor _currentCursor;
    private Vector2 _position;
    private Vector2 _delta;
    private Vector2 _scroll;
    private bool[] _currentButtons = new bool[MaxMouseButtons];
    private bool[] _previousButtons = new bool[MaxMouseButtons];

    public SDLPointerInputSource()
    {
        // TODO
        //_defaultCursor = new(SDL_GetDefaultCursor());
        //_currentCursor = new(SDL_GetCursor());

        alimerPlatformGetMousePosition(out float x, out float y);
        _position = new(x, y);
    }

    public void BeginFrame()
    {
        _previousButtons = _currentButtons;
        _delta = Vector2.Zero;
        _scroll = Vector2.Zero;
    }

    public override bool HasMouse => SDL_HasMouse();
    public override bool HasTouch
    {
        get
        {
            _ = SDL_GetTouchDevices(out int count);
            return count > 0;
        }
    }

    /// <inheritdoc />
    public override Vector2 Position => _position;

    /// <inheritdoc />
    public override Vector2 Delta => _delta;

    /// <inheritdoc />
    public override Vector2 Scroll => _scroll;

    /// <inheritdoc />
    public override bool IsCursorVisible
    {
        get => SDL_CursorVisible();
        set
        {
            if (value)
                SDL_ShowCursor();
            else
                SDL_HideCursor();
        }
    }

    /// <inheritdoc />
    public override Cursor Cursor
    {
        get => _currentCursor;
        set
        {
            if (_currentCursor == value)
                return;

            _currentCursor = (SDLCursor)value;
            SDL_SetCursor(_currentCursor.Handle);
        }
    }
    public override bool IsButtonDown(MouseButton button)
    {
        return _currentButtons[(int)button];
    }

    public override bool IsButtonPressed(MouseButton button)
    {
        return _currentButtons[(int)button] && !_previousButtons[(int)button];
    }

    public override bool IsButtonReleased(MouseButton button)
    {
        return !_currentButtons[(int)button] && _previousButtons[(int)button];
    }

    /// <inheritdoc />
    public override void SetPointerCapture()
    {
        _ = SDL_CaptureMouse(true);
    }

    /// <inheritdoc />
    public override void ReleasePointerCapture()
    {
        _ = SDL_CaptureMouse(false);
    }

    public void HandleWindowMouseEnterOrLeaveEvent(in PlatformEvent evt, bool enter)
    {
#if TODO
        SDL_Keymod mod = SDL_GetModState();
        SDL_Window* window = SDL_GetWindowFromID(evt.window.windowID);
        bool isInContact = GetMousePosition(window, out Vector2 mousePosition);
        PointerPoint pointerPoint = new()
        {
            IsInContact = isInContact,
            PointerId = uint.MaxValue,
            Position = mousePosition
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            //KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        if (enter)
        {
            OnPointerEntered(in args);
        }
        else
        {
            OnPointerExited(in args);
        } 
#endif
    }

    public void HandleMotionEvent(in MouseMotionEvent evt)
    {
        _position = new(evt.x, evt.y);
        _delta.X += evt.xRelative;
        _delta.Y += evt.yRelative;

        //SDL_Keymod mod = SDL_GetModState();
        PointerPoint pointerPoint = new()
        {
            IsInContact = true, //evt.state != 0,
            PointerId = uint.MaxValue,
            Position = _position
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            //KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        OnPointerMoved(in args); 
    }

    public void HandleWheelEvent(in MouseWheelEvent evt)
    {
        //_position = new(evt.mouse_x, evt.mouse_y);
        _scroll.X += evt.x;
        _scroll.Y += evt.y;

        //SDL_Keymod mod = SDL_GetModState();

        PointerPoint pointerPoint = new()
        {
            IsInContact = false,
            Button = MouseButton.Left,
            PointerId = 0, // (uint)evt.which, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = _position
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            //KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        OnPointerWheelChanged(in args);
    }

    public void HandleButtonEvent(in MouseButtonEvent evt, bool down)
    {
        _position = new(evt.x, evt.y);
        MouseButton button = evt.button;
        _currentButtons[(int)button] = down;

        //SDL_Keymod mod = SDL_GetModState();

        PointerPoint pointerPoint = new()
        {
            IsInContact = true,
            Button = button,
            PointerId = 0, //(uint)evt.which, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = _position
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            //KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        if (down)
        {
            OnPointerPressed(in args);
        }
        else
        {
            OnPointerReleased(in args);
        }
    }

    public void HandleFingerDown(in SDL_TouchFingerEvent evt)
    {
        PointerPoint pointerPoint = new()
        {
            IsInContact = true,
            PointerId = (uint)evt.touchID, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = new Vector2(evt.x, evt.y), // Normalized in the range 0...1
            Pressure = evt.pressure // Normalized in the range 0...1
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
        };

        OnPointerPressed(in args);
    }

    public void HandleFingerUp(in SDL_TouchFingerEvent evt)
    {
        PointerPoint pointerPoint = new()
        {
            IsInContact = true,
            PointerId = (uint)evt.touchID, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = new Vector2(evt.x, evt.y), // Normalized in the range 0...1
            Pressure = evt.pressure // Normalized in the range 0...1
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
        };

        OnPointerReleased(in args);
    }

    public void HandleFingerMotion(in SDL_TouchFingerEvent evt)
    {
        PointerPoint pointerPoint = new()
        {
            IsInContact = true,
            PointerId = (uint)evt.touchID, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = new Vector2(evt.x, evt.y), // Normalized in the range 0...1
            Pressure = evt.pressure // Normalized in the range 0...1
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
        };

        OnPointerMoved(in args);
    }

    private static bool GetMousePosition(SDL_Window* window, out Vector2 position)
    {
        SDL_MouseButtonFlags flags = SDL_GetGlobalMouseState(out float globalX, out float globalY);

        SDL_GetWindowPosition(window, out int windowX, out int windowY);
        position = new(globalX - windowX, globalY - windowY);
        return flags != 0;
    }
}
