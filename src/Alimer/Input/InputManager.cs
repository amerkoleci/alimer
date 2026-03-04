// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;

namespace Alimer.Input;

public abstract class InputManager
{
    /// <summary>
    /// Gets the keyboard input source.
    /// </summary>
    public abstract KeyboardInputSource Keyboard { get; }

    /// <summary>
    /// Gets the pointer input source.
    /// </summary>
    public abstract PointerInputSource Pointer { get; }

    /// <summary>
    /// Gets the gamepad input source.
    /// </summary>
    public abstract GamepadInputSource Gamepad { get; }

    protected InputManager()
    {
    }

    public void Scan()
    {
        Gamepad.Scan();
    }

    public void Update()
    {
        Keyboard.Update();
        Pointer.Update();
        Gamepad.Update();
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

    public bool IsMouseButtonDown(MouseButton button) => Pointer.IsButtonDown(button);
    public bool IsMouseButtonPressed(MouseButton button) => Pointer.IsButtonPressed(button);
    public bool IsMouseButtonReleased(MouseButton button) => Pointer.IsButtonReleased(button);
}
