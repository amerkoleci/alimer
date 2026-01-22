// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public enum Keys
{
    None = 0,

    Backspace = 0x08,
    Tab = 0x09,
    Clear = 0x0C,
    /// Return/ENTER key
    Return = 0x0D,

    Pause = 0x13,
    CapsLock = 0x14,
    Kana = 0x15,
    ImeOn = 0x16,

    Kanji = 0x19,

    ImeOff = 0x1a,
    Escape = 0x1b,
    ImeConvert = 0x1c,
    ImeNoConvert = 0x1d,

    Space = 0x20,
    PageUp = 0x21,
    PageDown = 0x22,
    End = 0x23,
    Home = 0x24,
    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,
    Select = 0x29,
    Print = 0x2a,
    Execute = 0x2b,
    PrintScreen = 0x2c,
    Insert = 0x2d,
    Delete = 0x2e,
    Help = 0x2f,
    D0 = 0x30,
    D1 = 0x31,
    D2 = 0x32,
    D3 = 0x33,
    D4 = 0x34,
    D5 = 0x35,
    D6 = 0x36,
    D7 = 0x37,
    D8 = 0x38,
    D9 = 0x39,

    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4a,
    K = 0x4b,
    L = 0x4c,
    M = 0x4d,
    N = 0x4e,
    O = 0x4f,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5a,

    LeftSuper = 0x5b, /* Left Windows */
    RightSuper = 0x5c, /* Right Windows */
    Apps = 0x5d,
    Sleep = 0x5f,

    /// Numeric keypad 0 key
    Numpad0 = 0x60,
    /// Numeric keypad 1 key
    Numpad1 = 0x61,
    /// Numeric keypad 2 key
    Numpad2 = 0x62,
    /// Numeric keypad 3 key
    Numpad3 = 0x63,
    /// Numeric keypad 4 key
    Numpad4 = 0x64,
    /// Numeric keypad 5 key
    Numpad5 = 0x65,
    /// Numeric keypad 6 key
    Numpad6 = 0x66,
    /// Numeric keypad 7 key
    Numpad7 = 0x67,
    /// Numeric keypad 8 key
    Numpad8 = 0x68,
    /// Numeric keypad 9 key
    Numpad9 = 0x69,
    /// Numeric keypad Multiply key '*'
    Multiply = 0x6A,
    /// Numeric keypad Add key '+'
    Add = 0x6B,
    /// Numeric Separator key
    Separator = 0x6C,
    /// Numeric keypad Subtract key '-'
    Subtract = 0x6D,
    /// Numeric keypad Decimal key ','
    Decimal = 0x6E,
    /// Numeric keypad Divide key '/'
    Divide = 0x6F,

    /// F1 function key
    F1 = 0x70,
    /// F2 function key
    F2 = 0x71,
    /// F3 function key
    F3 = 0x72,
    /// F4 function key
    F4 = 0x73,
    /// F5 function key
    F5 = 0x74,
    /// F6 function key
    F6 = 0x75,
    /// F7 function key
    F7 = 0x76,
    /// F8 function key
    F8 = 0x77,
    /// F9 function key
    F9 = 0x78,
    /// F10 function key
    F10 = 0x79,
    /// F11 function key
    F11 = 0x7A,
    /// F12 function key
    F12 = 0x7B,
    /// F13 function key
    F13 = 0x7C,
    /// F14 function key
    F14 = 0x7D,
    /// F15 function key
    F15 = 0x7E,
    /// F16 function key
    F16 = 0x7F,
    /// F17 function key
    F17 = 0x80,
    /// F18 function key
    F18 = 0x81,
    /// F19 function key
    F19 = 0x82,
    /// F20 function key
    F20 = 0x83,
    /// F21 function key
    F21 = 0x84,
    /// F22 function key
    F22 = 0x85,
    /// F23 function key
    F23 = 0x86,
    /// F24 function key
    F24 = 0x87,

    NumLock = 0x90,
    ScrollLock = 0x91,

    LeftShift = 0xa0,
    RightShift = 0xa1,
    LeftControl = 0xa2,
    RightControl = 0xa3,
    LeftAlt = 0xa4,
    RightAlt = 0xa5,
    BrowserBack = 0xa6,
    BrowserForward = 0xa7,
    BrowserRefresh = 0xa8,
    BrowserStop = 0xa9,
    BrowserSearch = 0xaa,
    BrowserFavorites = 0xab,
    BrowserHome = 0xac,
    VolumeMute = 0xad,
    VolumeDown = 0xae,
    VolumeUp = 0xaf,
    MediaNextTrack = 0xb0,
    MediaPreviousTrack = 0xb1,
    MediaStop = 0xb2,
    MediaPlayPause = 0xb3,
    LaunchMail = 0xb4,
    SelectMedia = 0xb5,
    LaunchApplication1 = 0xb6,
    LaunchApplication2 = 0xb7,

    OemSemicolon = 0xba,
    OemPlus = 0xbb,
    OemComma = 0xbc,
    OemMinus = 0xbd,
    OemPeriod = 0xbe,
    OemQuestion = 0xbf,
    OemTilde = 0xc0,

    OemOpenBrackets = 0xdb,
    OemPipe = 0xdc,
    OemCloseBrackets = 0xdd,
    OemQuotes = 0xde,
    Oem8 = 0xdf,

    OemBackslash = 0xe2,

    ProcessKey = 0xe5,

    OemCopy = 0xf2,
    OemAuto = 0xf3,
    OemEnlW = 0xf4,

    Attn = 0xf6,
    Crsel = 0xf7,
    Exsel = 0xf8,
    EraseEof = 0xf9,
    Play = 0xfa,
    Zoom = 0xfb,

    Pa1 = 0xfd,
    OemClear = 0xfe,
}
