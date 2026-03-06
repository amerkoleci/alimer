// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

/// <summary>
/// Represents an abstract base class for keyboard input devices, providing events and properties to track the state of
/// keyboard keys and modifiers.
/// </summary>
/// <remarks>Derived classes implement specific keyboard device behavior while leveraging common functionality for
/// managing key events and key state tracking. The class exposes events for key press and release, and maintains sets
/// of keys that are currently pressed, released, or held down. This enables consumers to query the current keyboard
/// state and respond to user input in a consistent manner.</remarks>
public abstract class KeyboardInputSource : IInputSource
{
    /// <summary>
    /// Occurs when a key is pressed or released while the control/window has focus.
    /// </summary>
    public event EventHandler<KeyEventArgs>? KeyEvent;

    /// <summary>
    /// Occurs when a text input is received.
    /// </summary>
    public event EventHandler<TextInputEventArgs>? TextInput;

    /// <summary>
    /// Return whether a keyboard is currently connected.
    /// </summary>
    public abstract bool HasKeyboard { get; }

    /// <summary>
    /// Gets the current modifier key state.
    /// </summary>
    public abstract KeyModifiers Modifiers { get; }

    /// <summary>
    /// Returns true if the key is currently held down.
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
	public abstract bool IsKeyDown(Keys key);

    /// <summary>
    /// Returns true if the key was just pressed this frame.
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    public abstract bool IsKeyPressed(Keys key);

    /// <summary>
    /// Returns true if the key was just released this frame.
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    public abstract bool IsKeyReleased(Keys key);

    protected virtual void OnKeyEvent(in KeyEventArgs e)
    {
        KeyEvent?.Invoke(this, e);
    }

    protected virtual void OnTextInput(in TextInputEventArgs e)
    {
        TextInput?.Invoke(this, e);
    }
}
