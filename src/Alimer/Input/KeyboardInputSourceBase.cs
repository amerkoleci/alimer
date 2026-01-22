// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public class KeyboardInputSourceBase : IKeyboardInputSource
{
    private readonly List<KeyEventArgs> _keyEvents = [];
    private readonly HashSet<Keys> _pressedKeys = [];
    private readonly HashSet<Keys> _releasedKeys = [];
    private readonly HashSet<Keys> _downKeys = [];

    /// <inheritdoc/>
    public event EventHandler<KeyEventArgs>? KeyDown;

    /// <inheritdoc/>
    public event EventHandler<KeyEventArgs>? KeyUp;

    /// <inheritdoc/>
    public IReadOnlySet<Keys> PressedKeys => _pressedKeys;

    /// <inheritdoc/>
    public IReadOnlySet<Keys> ReleasedKeys => _releasedKeys;

    /// <inheritdoc/>
    public IReadOnlySet<Keys> DownKeys => _downKeys;

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

    protected virtual void OnKeyDown(KeyEventArgs e)
    {
        // TODO: Handle key repeat
        if (!_downKeys.Contains(e.Key))
        {
            _downKeys.Add(e.Key);
            _keyEvents.Add(e);
        }

        KeyDown?.Invoke(this, e);
    }

    protected virtual void OnKeyUp(KeyEventArgs e)
    {
        if (_downKeys.Contains(e.Key))
        {
            _downKeys.Remove(e.Key);
            _keyEvents.Add(e);
        }

        KeyUp?.Invoke(this, e);
    }
}
