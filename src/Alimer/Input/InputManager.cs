// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Linq;
using Alimer.Engine;

namespace Alimer.Input;

public class InputManager : GameSystem
{
    private readonly List<IInputSource> _sources = [];
    private readonly List<IKeyboardInputSource> _keyboardSources = [];

    public IReadOnlyList<IInputSource> Sources => _sources;
    public IEnumerable<IKeyboardInputSource> KeyboardSources => _keyboardSources;
    public IKeyboardInputSource? Keyboard => _keyboardSources.Count > 0 ? _keyboardSources[0] : default;

    public InputManager(IInputSourceConfiguration configuration)
    {
        foreach (IInputSource inputSource in configuration.Sources)
        {
            AddSource(inputSource);
        }
    }

    public void AddSource(IInputSource source)
    {
        _sources.Add(source);

        if (source is IKeyboardInputSource keyboardSource)
        {
            _keyboardSources.Add(keyboardSource);
        }
    }

    public void Scan()
    {
        foreach (IInputSource source in Sources)
        {
            source.Scan();
        }
    }

    public void Update()
    {
        foreach (IInputSource source in Sources)
        {
            source.Update();
        }
    }

    public bool IsKeyDown(Keys key)
    {
        return Keyboard?.DownKeys.Contains(key) ?? false;
    }

    public bool IsKeyPressed(Keys key)
    {
        return Keyboard?.PressedKeys.Contains(key) ?? false;
    }

    public bool IsKeyReleased(Keys key)
    {
        return Keyboard?.ReleasedKeys.Contains(key) ?? false;
    }
}
