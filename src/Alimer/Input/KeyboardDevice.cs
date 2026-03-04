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
public abstract class KeyboardDevice : InputDevice
{
    private readonly List<KeyEventArgs> _keyEvents = [];
    private readonly HashSet<Keys> _pressedKeys = [];
    private readonly HashSet<Keys> _releasedKeys = [];
    private readonly HashSet<Keys> _downKeys = [];

    /// <summary>
    /// Occurs when a key is pressed while the control/window has focus.
    /// </summary>
    public event EventHandler<KeyEventArgs>? KeyDown;

    /// <summary>
    /// Occurs when a key is released while the control has focus.
    /// </summary>
    public event EventHandler<KeyEventArgs>? KeyUp;

    /// <summary>
    /// The keys that have been pressed since the last frame.
    /// </summary>
    public IReadOnlySet<Keys> PressedKeys => _pressedKeys;

    /// <summary>
    /// The keys that have been released since the last frame.
    /// </summary>
    public IReadOnlySet<Keys> ReleasedKeys => _releasedKeys;


    /// <summary>
    /// Gets the set of keys that are currently pressed.
    /// </summary>
    public IReadOnlySet<Keys> DownKeys => _downKeys;

    /// <summary>
    /// Gets the current modifier key state.
    /// </summary>
    public KeyModifiers Modifiers { get; protected set; }

    public virtual void Scan()
    {
    }

    public virtual void Update()
    {
        _pressedKeys.Clear();
        _releasedKeys.Clear();

        foreach (KeyEventArgs keyEvent in _keyEvents)
        {
            if (DownKeys.Contains(keyEvent.Key))
            {
                _pressedKeys.Add(keyEvent.Key);
            }
            else
            {
                _releasedKeys.Add(keyEvent.Key);
            }
        }

        _keyEvents.Clear();
    }

    protected virtual void OnKeyDown(in KeyEventArgs e)
    {
        // TODO: Handle key repeat
        if (!_downKeys.Contains(e.Key))
        {
            _downKeys.Add(e.Key);
            _keyEvents.Add(e);
        }

        KeyDown?.Invoke(this, e);
    }

    protected virtual void OnKeyUp(in KeyEventArgs e)
    {
        if (_downKeys.Contains(e.Key))
        {
            _downKeys.Remove(e.Key);
            _keyEvents.Add(e);
        }

        KeyUp?.Invoke(this, e);
    }
}
