// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_Scancode;
using static Alimer.SDL3.SDL_Keymod;

namespace Alimer.Input;

internal class SDLKeyboardInputSource : KeyboardInputSource
{
    private bool[] _currentState = new bool[(int)Keys.Count];
    private bool[] _previousState = new bool[(int)Keys.Count];
    private KeyModifiers _modifiers;

    public override bool HasKeyboard => SDL_HasKeyboard();
    public override KeyModifiers Modifiers => _modifiers;

    public void BeginFrame()
    {
        _previousState = _currentState;

    }

    public override bool IsKeyDown(Keys key)
    {
        int index = (int)key;
        if (index < 0 || index >= (int)Keys.Count)
            return false;

        return _currentState[index];
    }

    public override bool IsKeyPressed(Keys key)
    {
        int index = (int)key;
        if (index < 0 || index >= (int)Keys.Count)
            return false;

        return _currentState[index] && !_previousState[index];
    }

    public override bool IsKeyReleased(Keys key)
    {
        int index = (int)key;
        if (index < 0 || index >= (int)Keys.Count)
            return false;

        return !_currentState[index] && _previousState[index];
    }

    public void HandleKeyEvent(in KeyEvent evt, bool down)
    {
        // Update modifiers
        Keys key = evt.key;
        _currentState[(int)key] = down;
        _modifiers = evt.modifiers;

        KeyEventArgs args = new()
        {
            Key = key,
            IsDown = down,
        };
        OnKeyEvent(in args);
    }

    public unsafe void HandleTextInput(in TextInputEvent evt)
    {
        TextInputEventArgs args = new()
        {
            Text = PtrToStringUTF8(evt.text)!
        };

        OnTextInput(in args);
    }
}
