// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

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
        _defaultCursor = new(SDL_GetDefaultCursor());
        _currentCursor = new(SDL_GetCursor());

        _ = SDL_GetGlobalMouseState(out float x, out float y);
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

    public void HandleWindowMouseEnterOrLeaveEvent(in SDL_Event evt)
    {
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
            KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        bool enter = evt.type == SDL_EVENT_WINDOW_MOUSE_ENTER;
        if (enter)
        {
            OnPointerEntered(in args);
        }
        else
        {
            OnPointerExited(in args);
        }
    }

    public void HandleMotionEvent(in SDL_MouseMotionEvent evt)
    {
        _position = new(evt.x, evt.y);
        _delta.X += evt.xrel;
        _delta.Y += evt.yrel;

        SDL_Keymod mod = SDL_GetModState();
        PointerPoint pointerPoint = new()
        {
            IsInContact = evt.state != 0,
            PointerId = uint.MaxValue,
            Position = _position
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        OnPointerMoved(in args);
    }

    public void HandleWheelEvent(in SDL_MouseWheelEvent evt)
    {
        _position = new(evt.mouse_x, evt.mouse_y);
        _scroll.X += evt.x;
        _scroll.Y += evt.y;

        SDL_Keymod mod = SDL_GetModState();

        PointerPoint pointerPoint = new()
        {
            IsInContact = false,
            Button = MouseButton.Left,
            PointerId = (uint)evt.which, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = _position
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        OnPointerWheelChanged(in args);
    }

    public void HandleButtonEvent(in SDL_MouseButtonEvent evt)
    {
        _position = new(evt.x, evt.y);
        MouseButton button = ConvertButton(evt.button);
        _currentButtons[(int)button] = evt.down;

        SDL_Keymod mod = SDL_GetModState();

        PointerPoint pointerPoint = new()
        {
            IsInContact = true,
            Button = button,
            PointerId = (uint)evt.which, // The mouse instance id in relative mode, SDL_TOUCH_MOUSEID for touch events, or 0
            Position = _position
        };

        PointerEventArgs args = new()
        {
            CurrentPoint = pointerPoint,
            KeyModifiers = SDLKeyboardInputSource.FromSDLModifiers(mod)
        };

        if (evt.down)
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

    private static MouseButton ConvertButton(byte sdlButton)
    {
        return sdlButton switch
        {
            1 => MouseButton.Left,
            2 => MouseButton.Middle,
            3 => MouseButton.Right,
            4 => MouseButton.X1,
            5 => MouseButton.X2,
            _ => MouseButton.Left,
        };
    }

    private static bool GetMousePosition(SDL_Window* window, out Vector2 position)
    {
        SDL_MouseButtonFlags flags = SDL_GetGlobalMouseState(out float globalX, out float globalY);

        SDL_GetWindowPosition(window, out int windowX, out int windowY);
        position = new(globalX - windowX, globalY - windowY);
        return flags != 0;
    }
}
