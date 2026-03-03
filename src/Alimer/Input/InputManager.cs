// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Linq;
using Alimer.Engine;

namespace Alimer.Input;

public abstract class InputManager
{
    /// <summary>
    /// Gets the keyboard input device.
    /// </summary>
    public abstract IKeyboardInputSource Keyboard { get; }

    /// <summary>
    /// Gets the number of connected gamepads.
    /// </summary>
	public abstract IReadOnlyList<IGamepadDevice> Gamepads { get; }

    protected InputManager()
    {
    }

#if TODO
    public void Scan()
    {
        foreach (IInputSource source in Sources)
        {
            source.Scan();
        }
    } 
#endif

    public virtual void Update()
    {
    } 

    public bool IsKeyDown(Keys key)
    {
        return Keyboard.DownKeys.Contains(key);
    }

    public bool IsKeyPressed(Keys key)
    {
        return Keyboard.PressedKeys.Contains(key);
    }

    public bool IsKeyReleased(Keys key)
    {
        return Keyboard.ReleasedKeys.Contains(key);
    }
}
