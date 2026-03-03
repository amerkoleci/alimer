// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer.Input;

internal class NativeKeyboard : KeyboardInputSourceBase
{
    public void HandleKeyEvent(in KeyEvent key, bool down)
    {
        KeyEventArgs args = new()
        {
            Key = key.key
        };

        if (down)
        {
            OnKeyDown(args);
        }
        else
        {
            OnKeyUp(args);
        }


        // Update modifiers
        Modifiers = key.modifiers;
    }

    public void HandleTextInput(in TextInputEvent evt)
    {

    }
}

internal class NativeGamepadDevice : GamepadDeviceBase
{
}

internal class NativeInputManager : InputManager
{
    private readonly NativeKeyboard _keyboard;
    private readonly List<NativeGamepadDevice> _gamepads = [];

    public NativeInputManager()
    {
        _keyboard = new NativeKeyboard();
    }

    public override IKeyboardInputSource Keyboard => _keyboard;

    public override IReadOnlyList<IGamepadDevice> Gamepads => _gamepads;

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

            default:
                break;
        }
    }

    public void BeginFrame()
    {
        _keyboard.Update();
    }
}
