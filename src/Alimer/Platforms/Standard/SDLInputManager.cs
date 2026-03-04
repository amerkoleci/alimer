// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;
using static Alimer.SDL3.SDL_EventType;

namespace Alimer.Input;

internal class SDLInputManager : InputManager
{
    private readonly SDLKeyboardDevice _keyboard;
    private readonly SDLPointerDevice _pointer;
    //private readonly List<SDLGamepadDevice> _gamepads = [];

    public SDLInputManager()
    {
        _keyboard = new SDLKeyboardDevice();
        _pointer = new SDLPointerDevice();
    }

    public override KeyboardDevice Keyboard => _keyboard;
    public override PointerDevice Pointer => _pointer;

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

            default:
                break;
        }
    }

    public override void Update()
    {
        base.Update();

        _keyboard.Update();
        _pointer.Update();
    }

    public void BeginFrame()
    {
    }
}
