// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;

namespace Alimer.Input;

public abstract class GamepadInputSource : IInputSource
{
    /// <summary>
    /// Return whether a gamepad is currently connected.
    /// </summary>
    public abstract bool HasGamepad { get; }

    /// <summary>
    /// Gets the number of connected gamepads.
    /// </summary>
    public ObservableCollection<GamepadDevice> Gamepads { get; } = [];

    public virtual void Update()
    {
        foreach (GamepadDevice gamepad in Gamepads)
        {
            gamepad.Update();
        }
    }

    public bool IsButtonDown(GamepadButton button)
    {
        foreach (GamepadDevice gamepad in Gamepads)
        {
            if (gamepad.IsButtonDown(button))
                return true;
        }

        return false;
    }

    public bool IsButtonPressed(GamepadButton button)
    {
        foreach (GamepadDevice gamepad in Gamepads)
        {
            if (gamepad.IsButtonPressed(button))
                return true;
        }

        return false;
    }

    public bool IsButtonReleased(GamepadButton button)
    {
        foreach (GamepadDevice gamepad in Gamepads)
        {
            if (gamepad.IsButtonDown(button))
                return true;
        }

        return false;
    }
}
