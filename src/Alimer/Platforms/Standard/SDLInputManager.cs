// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
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

    public void HandleWindowMouseEnterOrLeaveEvent(in PlatformEvent evt, bool enter)
    {
        _pointer.HandleWindowMouseEnterOrLeaveEvent(in evt, enter);
    }

    public void HandleEvent(in PlatformEvent evt)
    {
        switch (evt.type)
        {
            case EventType.KeyDown:
            case EventType.KeyUp:
                _keyboard.HandleKeyEvent(in evt.key, evt.type == EventType.KeyDown);
                break;

            case EventType.TextInput:
                _keyboard.HandleTextInput(in evt.text);
                break;

            case EventType.MouseMotion:
                _pointer.HandleMotionEvent(in evt.motion);
                break;

            case EventType.MouseButtonDown:
                _pointer.HandleButtonEvent(in evt.button, true);
                break;

            case EventType.MouseButtonUp:
                _pointer.HandleButtonEvent(in evt.button, false);
                break;

            case EventType.MouseWheel:
                _pointer.HandleWheelEvent(in evt.wheel);
                break;

#if TODO
            case SDL_EVENT_FINGER_DOWN:
                _pointer.HandleFingerDown(in evt.tfinger);
                break;

            case SDL_EVENT_FINGER_UP:
                _pointer.HandleFingerUp(in evt.tfinger);
                break;

            case SDL_EVENT_FINGER_MOTION:
                _pointer.HandleFingerMotion(in evt.tfinger);
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
#endif

            default:
                break;
        }
    }
}
