// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;
using static Alimer.SDL3.SDL_Scancode;

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

    public void HandleKeyEvent(in SDL_KeyboardEvent key, bool down)
    {
        _keyArgs.SetKey(FromSDLKeyboardKey(key.scancode));

        if (down)
        {
            OnKeyDown(_keyArgs);
        }
        else
        {
            OnKeyUp(_keyArgs);
        }
    }

    public static Keys FromSDLKeyboardKey(SDL_Scancode code)
    {
        return code switch
        {
            SDL_SCANCODE_KP_BACKSPACE => Keys.Backspace,
            SDL_SCANCODE_KP_TAB => Keys.Tab,
            SDL_SCANCODE_KP_CLEAR => Keys.Clear,
            SDL_SCANCODE_RETURN => Keys.Return,
            SDL_SCANCODE_PAUSE => Keys.Pause,
            SDL_SCANCODE_CAPSLOCK => Keys.CapsLock,
            SDL_SCANCODE_LANG3 => Keys.Kana,
            //ImeOn = 0x16,
            //Kanji = 0x19,
            //ImeOff = 0x1a,
            SDL_SCANCODE_ESCAPE => Keys.Escape,
            //ImeConvert = 0x1c,
            //ImeNoConvert = 0x1d,
            SDL_SCANCODE_SPACE => Keys.Space,
            SDL_SCANCODE_PAGEUP => Keys.PageUp,
            SDL_SCANCODE_PAGEDOWN => Keys.PageDown,
            SDL_SCANCODE_END => Keys.End,
            SDL_SCANCODE_HOME => Keys.Home,
            SDL_SCANCODE_LEFT => Keys.Left,
            SDL_SCANCODE_UP => Keys.Up,
            SDL_SCANCODE_RIGHT => Keys.Right,
            SDL_SCANCODE_DOWN => Keys.Down,
            SDL_SCANCODE_SELECT => Keys.Select,
            //case SDL_SCANCODE_PRINTSCREEN:  return KeyboardKeys::Print;
            SDL_SCANCODE_EXECUTE => Keys.Execute,
            SDL_SCANCODE_PRINTSCREEN => Keys.PrintScreen,
            SDL_SCANCODE_INSERT => Keys.Insert,
            SDL_SCANCODE_DELETE => Keys.Delete,
            SDL_SCANCODE_HELP => Keys.Help,
            SDL_SCANCODE_1 => Keys.D1,
            SDL_SCANCODE_2 => Keys.D2,
            SDL_SCANCODE_3 => Keys.D3,
            SDL_SCANCODE_4 => Keys.D4,
            SDL_SCANCODE_5 => Keys.D5,
            SDL_SCANCODE_6 => Keys.D6,
            SDL_SCANCODE_7 => Keys.D7,
            SDL_SCANCODE_8 => Keys.D8,
            SDL_SCANCODE_9 => Keys.D9,
            SDL_SCANCODE_0 => Keys.D0,
            SDL_SCANCODE_A => Keys.A,
            SDL_SCANCODE_B => Keys.B,
            SDL_SCANCODE_C => Keys.C,
            SDL_SCANCODE_D => Keys.D,
            SDL_SCANCODE_E => Keys.E,
            SDL_SCANCODE_F => Keys.F,
            SDL_SCANCODE_G => Keys.G,
            SDL_SCANCODE_H => Keys.H,
            SDL_SCANCODE_I => Keys.I,
            SDL_SCANCODE_J => Keys.J,
            SDL_SCANCODE_K => Keys.K,
            SDL_SCANCODE_L => Keys.L,
            SDL_SCANCODE_M => Keys.M,
            SDL_SCANCODE_N => Keys.N,
            SDL_SCANCODE_O => Keys.O,
            SDL_SCANCODE_P => Keys.P,
            SDL_SCANCODE_Q => Keys.Q,
            SDL_SCANCODE_R => Keys.R,
            SDL_SCANCODE_S => Keys.S,
            SDL_SCANCODE_T => Keys.T,
            SDL_SCANCODE_U => Keys.U,
            SDL_SCANCODE_V => Keys.V,
            SDL_SCANCODE_W => Keys.W,
            SDL_SCANCODE_X => Keys.X,
            SDL_SCANCODE_Y => Keys.Y,
            SDL_SCANCODE_Z => Keys.Z,
            SDL_SCANCODE_LGUI => Keys.LeftSuper,
            SDL_SCANCODE_RGUI => Keys.RightSuper,
            SDL_SCANCODE_APPLICATION => Keys.Apps,
            SDL_SCANCODE_SLEEP => Keys.Sleep,
            SDL_SCANCODE_KP_0 => Keys.Numpad0,
            SDL_SCANCODE_KP_1 => Keys.Numpad1,
            SDL_SCANCODE_KP_2 => Keys.Numpad2,
            SDL_SCANCODE_KP_3 => Keys.Numpad3,
            SDL_SCANCODE_KP_4 => Keys.Numpad4,
            SDL_SCANCODE_KP_5 => Keys.Numpad5,
            SDL_SCANCODE_KP_6 => Keys.Numpad6,
            SDL_SCANCODE_KP_7 => Keys.Numpad7,
            SDL_SCANCODE_KP_8 => Keys.Numpad8,
            SDL_SCANCODE_KP_9 => Keys.Numpad9,
            SDL_SCANCODE_KP_MULTIPLY => Keys.Multiply,
            SDL_SCANCODE_KP_PLUS => Keys.Add,
            SDL_SCANCODE_SEPARATOR => Keys.Separator,
            SDL_SCANCODE_KP_MINUS => Keys.Subtract,
            SDL_SCANCODE_KP_PERIOD => Keys.Decimal,
            SDL_SCANCODE_KP_DIVIDE => Keys.Divide,
            //case SDL_SCANCODE_KP_ENTER:   return Key::Divide;
            SDL_SCANCODE_F1 => Keys.F1,
            SDL_SCANCODE_F2 => Keys.F2,
            SDL_SCANCODE_F3 => Keys.F3,
            SDL_SCANCODE_F4 => Keys.F4,
            SDL_SCANCODE_F5 => Keys.F5,
            SDL_SCANCODE_F6 => Keys.F6,
            SDL_SCANCODE_F7 => Keys.F7,
            SDL_SCANCODE_F8 => Keys.F8,
            SDL_SCANCODE_F9 => Keys.F9,
            SDL_SCANCODE_F10 => Keys.F10,
            SDL_SCANCODE_F11 => Keys.F11,
            SDL_SCANCODE_F12 => Keys.F12,
            SDL_SCANCODE_F13 => Keys.F13,
            SDL_SCANCODE_F14 => Keys.F14,
            SDL_SCANCODE_F15 => Keys.F15,
            SDL_SCANCODE_F16 => Keys.F16,
            SDL_SCANCODE_F17 => Keys.F17,
            SDL_SCANCODE_F18 => Keys.F18,
            SDL_SCANCODE_F19 => Keys.F19,
            SDL_SCANCODE_F20 => Keys.F20,
            SDL_SCANCODE_F21 => Keys.F21,
            SDL_SCANCODE_F22 => Keys.F22,
            SDL_SCANCODE_F23 => Keys.F23,
            SDL_SCANCODE_F24 => Keys.F24,
            SDL_SCANCODE_NUMLOCKCLEAR => Keys.NumLock,
            SDL_SCANCODE_SCROLLLOCK => Keys.ScrollLock,
            SDL_SCANCODE_LSHIFT => Keys.LeftShift,
            SDL_SCANCODE_RSHIFT => Keys.RightShift,
            SDL_SCANCODE_LCTRL => Keys.LeftControl,
            SDL_SCANCODE_RCTRL => Keys.RightControl,
            SDL_SCANCODE_LALT => Keys.LeftAlt,
            SDL_SCANCODE_RALT => Keys.RightAlt,
            _ => Keys.None,
        };
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

    public void HandleKeyEvent(in SDL_KeyboardEvent key, bool down)
    {
        _keyboard.HandleKeyEvent(in key, down);
    }
}
