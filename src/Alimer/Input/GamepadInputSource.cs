// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;

namespace Alimer.Input;

public abstract class GamepadInputSource : IInputSource
{
    /// <summary>
    /// Gets the number of connected gamepads.
    /// </summary>
    public ObservableCollection<GamepadDevice> Gamepads { get; } = [];

    public virtual void Scan()
    {
    }

    public virtual void Update()
    {
        foreach (GamepadDevice gamepad in Gamepads)
        {
            gamepad.Update();
        }
    }
}
