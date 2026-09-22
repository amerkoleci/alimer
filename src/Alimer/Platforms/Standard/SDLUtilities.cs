// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Input;
using static SDL3;
using static SDL3.SDL_GamepadAxis;
using static SDL3.SDL_GamepadButton;
using static SDL3.SDL_PowerState;

namespace Alimer;

internal static class SDLUtilities
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static GamepadButton FromSDL(this SDL_GamepadButton sdlButton)
    {
        return sdlButton switch
        {
            SDL_GAMEPAD_BUTTON_SOUTH => GamepadButton.South,
            SDL_GAMEPAD_BUTTON_EAST => GamepadButton.East,
            SDL_GAMEPAD_BUTTON_WEST => GamepadButton.West,
            SDL_GAMEPAD_BUTTON_NORTH => GamepadButton.North,
            SDL_GAMEPAD_BUTTON_BACK => GamepadButton.Back,
            SDL_GAMEPAD_BUTTON_GUIDE => GamepadButton.Guide,
            SDL_GAMEPAD_BUTTON_START => GamepadButton.Start,
            SDL_GAMEPAD_BUTTON_LEFT_STICK => GamepadButton.LeftStick,
            SDL_GAMEPAD_BUTTON_RIGHT_STICK => GamepadButton.RightStick,
            SDL_GAMEPAD_BUTTON_LEFT_SHOULDER => GamepadButton.LeftShoulder,
            SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER => GamepadButton.RightShoulder,
            SDL_GAMEPAD_BUTTON_DPAD_UP => GamepadButton.DPadUp,
            SDL_GAMEPAD_BUTTON_DPAD_DOWN => GamepadButton.DPadDown,
            SDL_GAMEPAD_BUTTON_DPAD_LEFT => GamepadButton.DPadLeft,
            SDL_GAMEPAD_BUTTON_DPAD_RIGHT => GamepadButton.DPadRight,
            SDL_GAMEPAD_BUTTON_MISC1 => GamepadButton.Misc1,
            SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1 => GamepadButton.RightPaddle1,
            SDL_GAMEPAD_BUTTON_LEFT_PADDLE1 => GamepadButton.LeftPaddle1,
            SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2 => GamepadButton.RightPaddle2,
            SDL_GAMEPAD_BUTTON_LEFT_PADDLE2 => GamepadButton.LeftPaddle2,
            SDL_GAMEPAD_BUTTON_TOUCHPAD => GamepadButton.Touchpad,
            _ => GamepadButton.Count,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static GamepadAxis FromSDL(this SDL_GamepadAxis axis)
    {
        return axis switch
        {
            SDL_GAMEPAD_AXIS_LEFTX => GamepadAxis.LeftX,
            SDL_GAMEPAD_AXIS_LEFTY => GamepadAxis.LeftY,
            SDL_GAMEPAD_AXIS_RIGHTX => GamepadAxis.RightX,
            SDL_GAMEPAD_AXIS_RIGHTY => GamepadAxis.RightY,
            SDL_GAMEPAD_AXIS_LEFT_TRIGGER => GamepadAxis.LeftTrigger,
            SDL_GAMEPAD_AXIS_RIGHT_TRIGGER => GamepadAxis.RightTrigger,
            _ => GamepadAxis.Count,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static BatteryStatus FromSDL(this SDL_PowerState value)
    {
        return value switch
        {
            SDL_POWERSTATE_UNKNOWN => BatteryStatus.Unknown,
            SDL_POWERSTATE_NO_BATTERY => BatteryStatus.NotPresent,
            SDL_POWERSTATE_ON_BATTERY => BatteryStatus.OnBattery,
            SDL_POWERSTATE_CHARGING => BatteryStatus.Charging,
            SDL_POWERSTATE_CHARGED => BatteryStatus.Charged,
            _ => BatteryStatus.Unknown
        };
    }
}
