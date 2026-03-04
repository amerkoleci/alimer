// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;
using static Alimer.SDL3.SDL_GamepadButton;
namespace Alimer.Input;

internal unsafe class SDLGamepadDevice : GamepadDevice
{
    private bool[] _currentButtons = new bool[(int)GamepadButton.Count];
    private bool[] _previousButtons = new bool[(int)GamepadButton.Count];
    //private float[] _axes = (int)GamepadAxis.Count];

    public SDLGamepadDevice(SDL_JoystickID id)
    {
        Id = id;
        Handle = SDL_OpenGamepad(id);
        Name = SDL_GetGamepadName(Handle);
        Type = SDL_GetGamepadType(Handle);
        Vendor = SDL_GetGamepadVendor(Handle);
        Product = SDL_GetGamepadProduct(Handle);
        Version = SDL_GetGamepadProductVersion(Handle);
    }

    public SDL_JoystickID Id { get; }
    public SDL_Gamepad* Handle { get; private set; }
    public string Name { get; }
    public SDL_GamepadType Type { get; }
    public ushort Vendor { get; }
    public ushort Product { get; }
    public ushort Version { get; }

    public void Close()
    {
        if (Handle != null)
        {
            SDL_CloseGamepad(Handle);
            Handle = null;
        }
    }

    /// <summary>
    /// Called before processing new events.
    /// </summary>
    public void BeginFrame()
    {
        _previousButtons = _currentButtons;
    }

    public override void Update()
    {

    }

    public void HandleButtonEvent(SDL_GamepadButtonEvent evt)
    {
        GamepadButton button = ConvertButton((SDL_GamepadButton)evt.button);
        int index = (int)button;
        if (index >= 0 && index < (int)GamepadButton.Count)
        {
            _currentButtons[index] = evt.down;
        }
    }

    public void HandleAxisEvent(SDL_GamepadAxisEvent evt)
    {
    }

    private static GamepadButton ConvertButton(SDL_GamepadButton sdlButton)
    {
        switch (sdlButton)
        {
            case SDL_GAMEPAD_BUTTON_SOUTH: return GamepadButton.South;
            case SDL_GAMEPAD_BUTTON_EAST: return GamepadButton.East;
            case SDL_GAMEPAD_BUTTON_WEST: return GamepadButton.West;
            case SDL_GAMEPAD_BUTTON_NORTH: return GamepadButton.North;
            case SDL_GAMEPAD_BUTTON_BACK: return GamepadButton.Back;
            case SDL_GAMEPAD_BUTTON_GUIDE: return GamepadButton.Guide;
            case SDL_GAMEPAD_BUTTON_START: return GamepadButton.Start;
            case SDL_GAMEPAD_BUTTON_LEFT_STICK: return GamepadButton.LeftStick;
            case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return GamepadButton.RightStick;
            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return GamepadButton.LeftShoulder;
            case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return GamepadButton.RightShoulder;
            case SDL_GAMEPAD_BUTTON_DPAD_UP: return GamepadButton.DPadUp;
            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return GamepadButton.DPadDown;
            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return GamepadButton.DPadLeft;
            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return GamepadButton.DPadRight;
            case SDL_GAMEPAD_BUTTON_MISC1: return GamepadButton.Misc1;
            case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1: return GamepadButton.RightPaddle1;
            case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1: return GamepadButton.LeftPaddle1;
            case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2: return GamepadButton.RightPaddle2;
            case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2: return GamepadButton.LeftPaddle2;
            case SDL_GAMEPAD_BUTTON_TOUCHPAD: return GamepadButton.Touchpad;
            default: return GamepadButton.Count;
        }
    }
}
