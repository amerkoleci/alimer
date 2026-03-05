// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;

namespace Alimer.Input;

internal class SDLGamepadInputSource : GamepadInputSource
{
    private readonly Dictionary<SDL_JoystickID, SDLGamepadDevice> _gamepads = [];

    public void BeginFrame()
    {
        SDL_UpdateGamepads();

        foreach (SDLGamepadDevice gamepad in _gamepads.Values)
        {
            gamepad.BeginFrame();
        }
    }

    public void HandleGamepadAdded(in SDL_GamepadDeviceEvent gdevice)
    {
        SDL_JoystickID id = gdevice.which;

        if (!_gamepads.ContainsKey(id))
        {
            SDLGamepadDevice newGamepad = new(id);
            _gamepads.Add(id, newGamepad);
            Gamepads.Add(newGamepad);
        }
    }

    public void HandleGamepadRemoved(in SDL_GamepadDeviceEvent gdevice)
    {
        SDL_JoystickID id = gdevice.which;

        if (_gamepads.TryGetValue(id, out SDLGamepadDevice? currentGamepad))
        {
            currentGamepad.Close();
            _gamepads.Remove(id);
            Gamepads.Remove(currentGamepad);
        }
    }
    public void HandleGamepadButton(in SDL_GamepadButtonEvent gbutton)
    {
        foreach (SDLGamepadDevice gamepad in _gamepads.Values)
        {
            if (gamepad.Id == gbutton.which)
            {
                gamepad.HandleButtonEvent(gbutton);
                break;
            }
        }
    }

    public void HandleGamepadAxis(in SDL_GamepadAxisEvent gaxis)
    {
        foreach (SDLGamepadDevice gamepad in _gamepads.Values)
        {
            if (gamepad.Id == gaxis.which)
            {
                gamepad.HandleAxisEvent(gaxis);
                break;
            }
        }
    }
}
