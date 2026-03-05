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

    public bool HasKeyboard => Keyboard.HasKeyboard;
    public bool HasMouse => Pointer.HasMouse;
    public bool HasTouch => Pointer.HasTouch;

    protected InputManager()
    {
    }

    public void Update()
    {
        Gamepad.Update();
    }

    public bool IsKeyDown(Keys key) => Keyboard.IsKeyDown(key);

    public bool IsKeyPressed(Keys key) => Keyboard.IsKeyPressed(key);

    public bool IsKeyReleased(Keys key) => Keyboard.IsKeyPressed(key);

    public bool IsMouseButtonDown(MouseButton button) => Pointer.IsButtonDown(button);
    public bool IsMouseButtonPressed(MouseButton button) => Pointer.IsButtonPressed(button);
    public bool IsMouseButtonReleased(MouseButton button) => Pointer.IsButtonReleased(button);
}
