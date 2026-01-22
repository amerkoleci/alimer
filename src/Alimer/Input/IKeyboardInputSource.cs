// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

/// <summary>
/// Represents a source of keyboard input, providing events and state information for key presses and releases.
/// </summary>
/// <remarks>Implementations of this interface allow consumers to monitor keyboard activity, including key down
/// and key up events, as well as to query the current and recent state of keys. This interface is typically used in
/// scenarios where precise or low-level keyboard input handling is required, such as in custom controls, games, or
/// input processing frameworks.
/// </remarks>
public interface IKeyboardInputSource : IInputSource
{
    /// <summary>
    /// Occurs when a key is pressed while the control/window has focus.
    /// </summary>
    public event EventHandler<KeyEventArgs> KeyDown;

    /// <summary>
    /// Occurs when a key is released while the control has focus.
    /// </summary>

    public event EventHandler<KeyEventArgs> KeyUp;

    /// <summary>
    /// The keys that have been pressed since the last frame.
    /// </summary>
    IReadOnlySet<Keys> PressedKeys { get; }

    /// <summary>
    /// The keys that have been released since the last frame.
    /// </summary>
    IReadOnlySet<Keys> ReleasedKeys { get; }

    /// <summary>
    /// Gets the set of keys that are currently pressed.
    /// </summary>
    IReadOnlySet<Keys> DownKeys { get; }
}
