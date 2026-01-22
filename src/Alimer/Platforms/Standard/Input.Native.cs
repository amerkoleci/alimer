// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer.Input;

internal class NativeKeyEventArgs : KeyEventArgs
{
    private Keys _key;

    public void SetKey(Keys key)
    {
        _key = key;
    }

    public override Keys Key => _key;
}

internal class NativeKeyboard : KeyboardInputSourceBase
{
    private readonly NativeKeyEventArgs _keyArgs = new();

    public void HandleKeyEvent(in KeyEvent key, bool down)
    {
        _keyArgs.SetKey(key.key);

        if (down)
        {
            OnKeyDown(_keyArgs);
        }
        else
        {
            OnKeyUp(_keyArgs);
        }
    }
}

internal class NativeInput : IInputSourceConfiguration
{
    private readonly NativeKeyboard _keyboard;
    public IList<IInputSource> Sources { get; } = [];

    public NativeInput()
    {
        _keyboard = new NativeKeyboard();

        Sources.Add(_keyboard);
    }

    public void HandleKeyEvent(in KeyEvent key, bool down)
    {
        _keyboard.HandleKeyEvent(in key, down);
    }
}
