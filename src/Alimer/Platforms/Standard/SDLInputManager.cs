// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;
using static Alimer.SDL3.SDL_EventType;

namespace Alimer.Input;

internal class SDLInputManager : InputManager
{
    private readonly SDLKeyboardInputSource _keyboard;
    private readonly SDLPointerInputSource _pointer;
    private readonly SDLGamepadInputSource _gamepad;
    //private readonly List<SDLGamepadDevice> _gamepads = [];

    public SDLInputManager()
    {
        _keyboard = new SDLKeyboardInputSource();
        _pointer = new SDLPointerInputSource();
        _gamepad = new SDLGamepadInputSource();
    }

    public override KeyboardInputSource Keyboard => _keyboard;
    public override PointerInputSource Pointer => _pointer;
    public override GamepadInputSource Gamepad => _gamepad;

    public void BeginFrame()
    {
        _keyboard.BeginFrame();
        _pointer.BeginFrame();
        _gamepad.BeginFrame();
    }

    public void HandleEvent(in SDL_Event evt)
    {
        switch (evt.type)
        {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                _keyboard.HandleKeyEvent(in evt.key, evt.type == SDL_EVENT_KEY_DOWN);
                break;

            case SDL_EVENT_TEXT_INPUT:
                _keyboard.HandleTextInput(in evt.text);
                break;

            case SDL_EVENT_MOUSE_MOTION:
                _pointer.HandleMotionEvent(in evt.motion);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                _pointer.HandleButtonEvent(in evt.button);
                break;

            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                _pointer.HandleWindowMouseEnterOrLeaveEvent(in evt);
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                _pointer.HandleWheelEvent(in evt.wheel);
                break;

            case SDL_EVENT_GAMEPAD_ADDED:
                _gamepad.HandleGamepadAdded(in evt.gdevice);
                break;

            case SDL_EVENT_GAMEPAD_REMOVED:
                _gamepad.HandleGamepadRemoved(in evt.gdevice);
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                _gamepad.HandleGamepadButton(in evt.gbutton);
                break;

            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                _gamepad.HandleGamepadAxis(in evt.gaxis);
                break;

            default:
                break;
        }
    }
}
