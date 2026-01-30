// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using Alimer.Input;

namespace Alimer;

internal static unsafe partial class SDL3
{
    public enum SDL_Scancode
    {
        SDL_SCANCODE_UNKNOWN = 0,
        SDL_SCANCODE_A = 4,
        SDL_SCANCODE_B = 5,
        SDL_SCANCODE_C = 6,
        SDL_SCANCODE_D = 7,
        SDL_SCANCODE_E = 8,
        SDL_SCANCODE_F = 9,
        SDL_SCANCODE_G = 10,
        SDL_SCANCODE_H = 11,
        SDL_SCANCODE_I = 12,
        SDL_SCANCODE_J = 13,
        SDL_SCANCODE_K = 14,
        SDL_SCANCODE_L = 15,
        SDL_SCANCODE_M = 16,
        SDL_SCANCODE_N = 17,
        SDL_SCANCODE_O = 18,
        SDL_SCANCODE_P = 19,
        SDL_SCANCODE_Q = 20,
        SDL_SCANCODE_R = 21,
        SDL_SCANCODE_S = 22,
        SDL_SCANCODE_T = 23,
        SDL_SCANCODE_U = 24,
        SDL_SCANCODE_V = 25,
        SDL_SCANCODE_W = 26,
        SDL_SCANCODE_X = 27,
        SDL_SCANCODE_Y = 28,
        SDL_SCANCODE_Z = 29,
        SDL_SCANCODE_1 = 30,
        SDL_SCANCODE_2 = 31,
        SDL_SCANCODE_3 = 32,
        SDL_SCANCODE_4 = 33,
        SDL_SCANCODE_5 = 34,
        SDL_SCANCODE_6 = 35,
        SDL_SCANCODE_7 = 36,
        SDL_SCANCODE_8 = 37,
        SDL_SCANCODE_9 = 38,
        SDL_SCANCODE_0 = 39,
        SDL_SCANCODE_RETURN = 40,
        SDL_SCANCODE_ESCAPE = 41,
        SDL_SCANCODE_BACKSPACE = 42,
        SDL_SCANCODE_TAB = 43,
        SDL_SCANCODE_SPACE = 44,
        SDL_SCANCODE_MINUS = 45,
        SDL_SCANCODE_EQUALS = 46,
        SDL_SCANCODE_LEFTBRACKET = 47,
        SDL_SCANCODE_RIGHTBRACKET = 48,
        SDL_SCANCODE_BACKSLASH = 49,
        SDL_SCANCODE_NONUSHASH = 50,
        SDL_SCANCODE_SEMICOLON = 51,
        SDL_SCANCODE_APOSTROPHE = 52,
        SDL_SCANCODE_GRAVE = 53,
        SDL_SCANCODE_COMMA = 54,
        SDL_SCANCODE_PERIOD = 55,
        SDL_SCANCODE_SLASH = 56,
        SDL_SCANCODE_CAPSLOCK = 57,
        SDL_SCANCODE_F1 = 58,
        SDL_SCANCODE_F2 = 59,
        SDL_SCANCODE_F3 = 60,
        SDL_SCANCODE_F4 = 61,
        SDL_SCANCODE_F5 = 62,
        SDL_SCANCODE_F6 = 63,
        SDL_SCANCODE_F7 = 64,
        SDL_SCANCODE_F8 = 65,
        SDL_SCANCODE_F9 = 66,
        SDL_SCANCODE_F10 = 67,
        SDL_SCANCODE_F11 = 68,
        SDL_SCANCODE_F12 = 69,
        SDL_SCANCODE_PRINTSCREEN = 70,
        SDL_SCANCODE_SCROLLLOCK = 71,
        SDL_SCANCODE_PAUSE = 72,
        SDL_SCANCODE_INSERT = 73,
        SDL_SCANCODE_HOME = 74,
        SDL_SCANCODE_PAGEUP = 75,
        SDL_SCANCODE_DELETE = 76,
        SDL_SCANCODE_END = 77,
        SDL_SCANCODE_PAGEDOWN = 78,
        SDL_SCANCODE_RIGHT = 79,
        SDL_SCANCODE_LEFT = 80,
        SDL_SCANCODE_DOWN = 81,
        SDL_SCANCODE_UP = 82,
        SDL_SCANCODE_NUMLOCKCLEAR = 83,
        SDL_SCANCODE_KP_DIVIDE = 84,
        SDL_SCANCODE_KP_MULTIPLY = 85,
        SDL_SCANCODE_KP_MINUS = 86,
        SDL_SCANCODE_KP_PLUS = 87,
        SDL_SCANCODE_KP_ENTER = 88,
        SDL_SCANCODE_KP_1 = 89,
        SDL_SCANCODE_KP_2 = 90,
        SDL_SCANCODE_KP_3 = 91,
        SDL_SCANCODE_KP_4 = 92,
        SDL_SCANCODE_KP_5 = 93,
        SDL_SCANCODE_KP_6 = 94,
        SDL_SCANCODE_KP_7 = 95,
        SDL_SCANCODE_KP_8 = 96,
        SDL_SCANCODE_KP_9 = 97,
        SDL_SCANCODE_KP_0 = 98,
        SDL_SCANCODE_KP_PERIOD = 99,
        SDL_SCANCODE_NONUSBACKSLASH = 100,
        SDL_SCANCODE_APPLICATION = 101,
        SDL_SCANCODE_POWER = 102,
        SDL_SCANCODE_KP_EQUALS = 103,
        SDL_SCANCODE_F13 = 104,
        SDL_SCANCODE_F14 = 105,
        SDL_SCANCODE_F15 = 106,
        SDL_SCANCODE_F16 = 107,
        SDL_SCANCODE_F17 = 108,
        SDL_SCANCODE_F18 = 109,
        SDL_SCANCODE_F19 = 110,
        SDL_SCANCODE_F20 = 111,
        SDL_SCANCODE_F21 = 112,
        SDL_SCANCODE_F22 = 113,
        SDL_SCANCODE_F23 = 114,
        SDL_SCANCODE_F24 = 115,
        SDL_SCANCODE_EXECUTE = 116,
        SDL_SCANCODE_HELP = 117,
        SDL_SCANCODE_MENU = 118,
        SDL_SCANCODE_SELECT = 119,
        SDL_SCANCODE_STOP = 120,
        SDL_SCANCODE_AGAIN = 121,
        SDL_SCANCODE_UNDO = 122,
        SDL_SCANCODE_CUT = 123,
        SDL_SCANCODE_COPY = 124,
        SDL_SCANCODE_PASTE = 125,
        SDL_SCANCODE_FIND = 126,
        SDL_SCANCODE_MUTE = 127,
        SDL_SCANCODE_VOLUMEUP = 128,
        SDL_SCANCODE_VOLUMEDOWN = 129,
        SDL_SCANCODE_KP_COMMA = 133,
        SDL_SCANCODE_KP_EQUALSAS400 = 134,
        SDL_SCANCODE_INTERNATIONAL1 = 135,
        SDL_SCANCODE_INTERNATIONAL2 = 136,
        SDL_SCANCODE_INTERNATIONAL3 = 137,
        SDL_SCANCODE_INTERNATIONAL4 = 138,
        SDL_SCANCODE_INTERNATIONAL5 = 139,
        SDL_SCANCODE_INTERNATIONAL6 = 140,
        SDL_SCANCODE_INTERNATIONAL7 = 141,
        SDL_SCANCODE_INTERNATIONAL8 = 142,
        SDL_SCANCODE_INTERNATIONAL9 = 143,
        SDL_SCANCODE_LANG1 = 144,
        SDL_SCANCODE_LANG2 = 145,
        SDL_SCANCODE_LANG3 = 146,
        SDL_SCANCODE_LANG4 = 147,
        SDL_SCANCODE_LANG5 = 148,
        SDL_SCANCODE_LANG6 = 149,
        SDL_SCANCODE_LANG7 = 150,
        SDL_SCANCODE_LANG8 = 151,
        SDL_SCANCODE_LANG9 = 152,
        SDL_SCANCODE_ALTERASE = 153,
        SDL_SCANCODE_SYSREQ = 154,
        SDL_SCANCODE_CANCEL = 155,
        SDL_SCANCODE_CLEAR = 156,
        SDL_SCANCODE_PRIOR = 157,
        SDL_SCANCODE_RETURN2 = 158,
        SDL_SCANCODE_SEPARATOR = 159,
        SDL_SCANCODE_OUT = 160,
        SDL_SCANCODE_OPER = 161,
        SDL_SCANCODE_CLEARAGAIN = 162,
        SDL_SCANCODE_CRSEL = 163,
        SDL_SCANCODE_EXSEL = 164,
        SDL_SCANCODE_KP_00 = 176,
        SDL_SCANCODE_KP_000 = 177,
        SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
        SDL_SCANCODE_DECIMALSEPARATOR = 179,
        SDL_SCANCODE_CURRENCYUNIT = 180,
        SDL_SCANCODE_CURRENCYSUBUNIT = 181,
        SDL_SCANCODE_KP_LEFTPAREN = 182,
        SDL_SCANCODE_KP_RIGHTPAREN = 183,
        SDL_SCANCODE_KP_LEFTBRACE = 184,
        SDL_SCANCODE_KP_RIGHTBRACE = 185,
        SDL_SCANCODE_KP_TAB = 186,
        SDL_SCANCODE_KP_BACKSPACE = 187,
        SDL_SCANCODE_KP_A = 188,
        SDL_SCANCODE_KP_B = 189,
        SDL_SCANCODE_KP_C = 190,
        SDL_SCANCODE_KP_D = 191,
        SDL_SCANCODE_KP_E = 192,
        SDL_SCANCODE_KP_F = 193,
        SDL_SCANCODE_KP_XOR = 194,
        SDL_SCANCODE_KP_POWER = 195,
        SDL_SCANCODE_KP_PERCENT = 196,
        SDL_SCANCODE_KP_LESS = 197,
        SDL_SCANCODE_KP_GREATER = 198,
        SDL_SCANCODE_KP_AMPERSAND = 199,
        SDL_SCANCODE_KP_DBLAMPERSAND = 200,
        SDL_SCANCODE_KP_VERTICALBAR = 201,
        SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
        SDL_SCANCODE_KP_COLON = 203,
        SDL_SCANCODE_KP_HASH = 204,
        SDL_SCANCODE_KP_SPACE = 205,
        SDL_SCANCODE_KP_AT = 206,
        SDL_SCANCODE_KP_EXCLAM = 207,
        SDL_SCANCODE_KP_MEMSTORE = 208,
        SDL_SCANCODE_KP_MEMRECALL = 209,
        SDL_SCANCODE_KP_MEMCLEAR = 210,
        SDL_SCANCODE_KP_MEMADD = 211,
        SDL_SCANCODE_KP_MEMSUBTRACT = 212,
        SDL_SCANCODE_KP_MEMMULTIPLY = 213,
        SDL_SCANCODE_KP_MEMDIVIDE = 214,
        SDL_SCANCODE_KP_PLUSMINUS = 215,
        SDL_SCANCODE_KP_CLEAR = 216,
        SDL_SCANCODE_KP_CLEARENTRY = 217,
        SDL_SCANCODE_KP_BINARY = 218,
        SDL_SCANCODE_KP_OCTAL = 219,
        SDL_SCANCODE_KP_DECIMAL = 220,
        SDL_SCANCODE_KP_HEXADECIMAL = 221,
        SDL_SCANCODE_LCTRL = 224,
        SDL_SCANCODE_LSHIFT = 225,
        SDL_SCANCODE_LALT = 226,
        SDL_SCANCODE_LGUI = 227,
        SDL_SCANCODE_RCTRL = 228,
        SDL_SCANCODE_RSHIFT = 229,
        SDL_SCANCODE_RALT = 230,
        SDL_SCANCODE_RGUI = 231,
        SDL_SCANCODE_MODE = 257,
        SDL_SCANCODE_SLEEP = 258,
        SDL_SCANCODE_WAKE = 259,
        SDL_SCANCODE_CHANNEL_INCREMENT = 260,
        SDL_SCANCODE_CHANNEL_DECREMENT = 261,
        SDL_SCANCODE_MEDIA_PLAY = 262,
        SDL_SCANCODE_MEDIA_PAUSE = 263,
        SDL_SCANCODE_MEDIA_RECORD = 264,
        SDL_SCANCODE_MEDIA_FAST_FORWARD = 265,
        SDL_SCANCODE_MEDIA_REWIND = 266,
        SDL_SCANCODE_MEDIA_NEXT_TRACK = 267,
        SDL_SCANCODE_MEDIA_PREVIOUS_TRACK = 268,
        SDL_SCANCODE_MEDIA_STOP = 269,
        SDL_SCANCODE_MEDIA_EJECT = 270,
        SDL_SCANCODE_MEDIA_PLAY_PAUSE = 271,
        SDL_SCANCODE_MEDIA_SELECT = 272,
        SDL_SCANCODE_AC_NEW = 273,
        SDL_SCANCODE_AC_OPEN = 274,
        SDL_SCANCODE_AC_CLOSE = 275,
        SDL_SCANCODE_AC_EXIT = 276,
        SDL_SCANCODE_AC_SAVE = 277,
        SDL_SCANCODE_AC_PRINT = 278,
        SDL_SCANCODE_AC_PROPERTIES = 279,
        SDL_SCANCODE_AC_SEARCH = 280,
        SDL_SCANCODE_AC_HOME = 281,
        SDL_SCANCODE_AC_BACK = 282,
        SDL_SCANCODE_AC_FORWARD = 283,
        SDL_SCANCODE_AC_STOP = 284,
        SDL_SCANCODE_AC_REFRESH = 285,
        SDL_SCANCODE_AC_BOOKMARKS = 286,
        SDL_SCANCODE_SOFTLEFT = 287,
        SDL_SCANCODE_SOFTRIGHT = 288,
        SDL_SCANCODE_CALL = 289,
        SDL_SCANCODE_ENDCALL = 290,
        SDL_SCANCODE_RESERVED = 400,
        SDL_SCANCODE_COUNT = 512,
    }

    [Flags]
    public enum SDL_Keymod : ushort
    {
        SDL_KMOD_NONE = 0x0000,
        SDL_KMOD_LSHIFT = 0x0001,
        SDL_KMOD_RSHIFT = 0x0002,
        SDL_KMOD_LCTRL = 0x0040,
        SDL_KMOD_RCTRL = 0x0080,
        SDL_KMOD_LALT = 0x0100,
        SDL_KMOD_RALT = 0x0200,
        SDL_KMOD_LGUI = 0x0400,
        SDL_KMOD_RGUI = 0x0800,
        SDL_KMOD_NUM = 0x1000,
        SDL_KMOD_CAPS = 0x2000,
        SDL_KMOD_MODE = 0x4000,
        SDL_KMOD_SCROLL = 0x8000,
        SDL_KMOD_CTRL = SDL_KMOD_LCTRL | SDL_KMOD_RCTRL,
        SDL_KMOD_SHIFT = SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT,
        SDL_KMOD_ALT = SDL_KMOD_RALT | SDL_KMOD_LALT,
        SDL_KMOD_GUI = SDL_KMOD_RGUI | SDL_KMOD_LGUI,
    }

    public enum SDL_PowerState
    {
        SDL_POWERSTATE_ERROR = -1,
        SDL_POWERSTATE_UNKNOWN = 0,
        SDL_POWERSTATE_ON_BATTERY = 1,
        SDL_POWERSTATE_NO_BATTERY = 2,
        SDL_POWERSTATE_CHARGING = 3,
        SDL_POWERSTATE_CHARGED = 4,
    }

    [Flags]
    public enum SDL_MouseButtonFlags : uint
    {
        SDL_BUTTON_LMASK = 0x1,
        SDL_BUTTON_MMASK = 0x2,
        SDL_BUTTON_RMASK = 0x4,
        SDL_BUTTON_X1MASK = 0x08,
        SDL_BUTTON_X2MASK = 0x10,
    }

    public enum SDL_MouseWheelDirection
    {
        SDL_MOUSEWHEEL_NORMAL = 0,
        SDL_MOUSEWHEEL_FLIPPED = 1,
    }

    [Flags]
    public enum SDL_PenInputFlags : uint
    {
        SDL_PEN_INPUT_DOWN = 0x1,
        SDL_PEN_INPUT_BUTTON_1 = 0x2,
        SDL_PEN_INPUT_BUTTON_2 = 0x4,
        SDL_PEN_INPUT_BUTTON_3 = 0x08,
        SDL_PEN_INPUT_BUTTON_4 = 0x10,
        SDL_PEN_INPUT_BUTTON_5 = 0x20,
        SDL_PEN_INPUT_ERASER_TIP = 0x40000000,
    }

    public enum SDL_PenAxis
    {
        SDL_PEN_AXIS_PRESSURE = 0,
        SDL_PEN_AXIS_XTILT = 1,
        SDL_PEN_AXIS_YTILT = 2,
        SDL_PEN_AXIS_DISTANCE = 3,
        SDL_PEN_AXIS_ROTATION = 4,
        SDL_PEN_AXIS_SLIDER = 5,
        SDL_PEN_AXIS_TANGENTIAL_PRESSURE = 6,
        SDL_PEN_AXIS_COUNT = 7,
    }

    public enum SDL_PenDeviceType
    {
        SDL_PEN_DEVICE_TYPE_INVALID = -1,
        SDL_PEN_DEVICE_TYPE_UNKNOWN = 0,
        SDL_PEN_DEVICE_TYPE_DIRECT = 1,
        SDL_PEN_DEVICE_TYPE_INDIRECT = 2,
    }

    public enum SDL_EventType
    {
        SDL_EVENT_FIRST = 0,
        SDL_EVENT_QUIT = 256,
        SDL_EVENT_TERMINATING = 257,
        SDL_EVENT_LOW_MEMORY = 258,
        SDL_EVENT_WILL_ENTER_BACKGROUND = 259,
        SDL_EVENT_DID_ENTER_BACKGROUND = 260,
        SDL_EVENT_WILL_ENTER_FOREGROUND = 261,
        SDL_EVENT_DID_ENTER_FOREGROUND = 262,
        SDL_EVENT_LOCALE_CHANGED = 263,
        SDL_EVENT_SYSTEM_THEME_CHANGED = 264,
        SDL_EVENT_DISPLAY_ORIENTATION = 337,
        SDL_EVENT_DISPLAY_ADDED = 338,
        SDL_EVENT_DISPLAY_REMOVED = 339,
        SDL_EVENT_DISPLAY_MOVED = 340,
        SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED = 341,
        SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED = 342,
        SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED = 343,
        SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED = 344,
        SDL_EVENT_DISPLAY_FIRST = 337,
        SDL_EVENT_DISPLAY_LAST = 344,
        SDL_EVENT_WINDOW_SHOWN = 514,
        SDL_EVENT_WINDOW_HIDDEN = 515,
        SDL_EVENT_WINDOW_EXPOSED = 516,
        SDL_EVENT_WINDOW_MOVED = 517,
        SDL_EVENT_WINDOW_RESIZED = 518,
        SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED = 519,
        SDL_EVENT_WINDOW_METAL_VIEW_RESIZED = 520,
        SDL_EVENT_WINDOW_MINIMIZED = 521,
        SDL_EVENT_WINDOW_MAXIMIZED = 522,
        SDL_EVENT_WINDOW_RESTORED = 523,
        SDL_EVENT_WINDOW_MOUSE_ENTER = 524,
        SDL_EVENT_WINDOW_MOUSE_LEAVE = 525,
        SDL_EVENT_WINDOW_FOCUS_GAINED = 526,
        SDL_EVENT_WINDOW_FOCUS_LOST = 527,
        SDL_EVENT_WINDOW_CLOSE_REQUESTED = 528,
        SDL_EVENT_WINDOW_HIT_TEST = 529,
        SDL_EVENT_WINDOW_ICCPROF_CHANGED = 530,
        SDL_EVENT_WINDOW_DISPLAY_CHANGED = 531,
        SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED = 532,
        SDL_EVENT_WINDOW_SAFE_AREA_CHANGED = 533,
        SDL_EVENT_WINDOW_OCCLUDED = 534,
        SDL_EVENT_WINDOW_ENTER_FULLSCREEN = 535,
        SDL_EVENT_WINDOW_LEAVE_FULLSCREEN = 536,
        SDL_EVENT_WINDOW_DESTROYED = 537,
        SDL_EVENT_WINDOW_HDR_STATE_CHANGED = 538,
        SDL_EVENT_WINDOW_FIRST = 514,
        SDL_EVENT_WINDOW_LAST = 538,
        SDL_EVENT_KEY_DOWN = 768,
        SDL_EVENT_KEY_UP = 769,
        SDL_EVENT_TEXT_EDITING = 770,
        SDL_EVENT_TEXT_INPUT = 771,
        SDL_EVENT_KEYMAP_CHANGED = 772,
        SDL_EVENT_KEYBOARD_ADDED = 773,
        SDL_EVENT_KEYBOARD_REMOVED = 774,
        SDL_EVENT_TEXT_EDITING_CANDIDATES = 775,
        SDL_EVENT_SCREEN_KEYBOARD_SHOWN = 776,
        SDL_EVENT_SCREEN_KEYBOARD_HIDDEN = 777,
        SDL_EVENT_MOUSE_MOTION = 1024,
        SDL_EVENT_MOUSE_BUTTON_DOWN = 1025,
        SDL_EVENT_MOUSE_BUTTON_UP = 1026,
        SDL_EVENT_MOUSE_WHEEL = 1027,
        SDL_EVENT_MOUSE_ADDED = 1028,
        SDL_EVENT_MOUSE_REMOVED = 1029,
        SDL_EVENT_JOYSTICK_AXIS_MOTION = 1536,
        SDL_EVENT_JOYSTICK_BALL_MOTION = 1537,
        SDL_EVENT_JOYSTICK_HAT_MOTION = 1538,
        SDL_EVENT_JOYSTICK_BUTTON_DOWN = 1539,
        SDL_EVENT_JOYSTICK_BUTTON_UP = 1540,
        SDL_EVENT_JOYSTICK_ADDED = 1541,
        SDL_EVENT_JOYSTICK_REMOVED = 1542,
        SDL_EVENT_JOYSTICK_BATTERY_UPDATED = 1543,
        SDL_EVENT_JOYSTICK_UPDATE_COMPLETE = 1544,
        SDL_EVENT_GAMEPAD_AXIS_MOTION = 1616,
        SDL_EVENT_GAMEPAD_BUTTON_DOWN = 1617,
        SDL_EVENT_GAMEPAD_BUTTON_UP = 1618,
        SDL_EVENT_GAMEPAD_ADDED = 1619,
        SDL_EVENT_GAMEPAD_REMOVED = 1620,
        SDL_EVENT_GAMEPAD_REMAPPED = 1621,
        SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN = 1622,
        SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION = 1623,
        SDL_EVENT_GAMEPAD_TOUCHPAD_UP = 1624,
        SDL_EVENT_GAMEPAD_SENSOR_UPDATE = 1625,
        SDL_EVENT_GAMEPAD_UPDATE_COMPLETE = 1626,
        SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED = 1627,
        SDL_EVENT_FINGER_DOWN = 1792,
        SDL_EVENT_FINGER_UP = 1793,
        SDL_EVENT_FINGER_MOTION = 1794,
        SDL_EVENT_FINGER_CANCELED = 1795,
        SDL_EVENT_PINCH_BEGIN = 1808,
        SDL_EVENT_PINCH_UPDATE = 1809,
        SDL_EVENT_PINCH_END = 1810,
        SDL_EVENT_CLIPBOARD_UPDATE = 2304,
        SDL_EVENT_DROP_FILE = 4096,
        SDL_EVENT_DROP_TEXT = 4097,
        SDL_EVENT_DROP_BEGIN = 4098,
        SDL_EVENT_DROP_COMPLETE = 4099,
        SDL_EVENT_DROP_POSITION = 4100,
        SDL_EVENT_AUDIO_DEVICE_ADDED = 4352,
        SDL_EVENT_AUDIO_DEVICE_REMOVED = 4353,
        SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED = 4354,
        SDL_EVENT_SENSOR_UPDATE = 4608,
        SDL_EVENT_PEN_PROXIMITY_IN = 4864,
        SDL_EVENT_PEN_PROXIMITY_OUT = 4865,
        SDL_EVENT_PEN_DOWN = 4866,
        SDL_EVENT_PEN_UP = 4867,
        SDL_EVENT_PEN_BUTTON_DOWN = 4868,
        SDL_EVENT_PEN_BUTTON_UP = 4869,
        SDL_EVENT_PEN_MOTION = 4870,
        SDL_EVENT_PEN_AXIS = 4871,
        SDL_EVENT_CAMERA_DEVICE_ADDED = 5120,
        SDL_EVENT_CAMERA_DEVICE_REMOVED = 5121,
        SDL_EVENT_CAMERA_DEVICE_APPROVED = 5122,
        SDL_EVENT_CAMERA_DEVICE_DENIED = 5123,
        SDL_EVENT_RENDER_TARGETS_RESET = 8192,
        SDL_EVENT_RENDER_DEVICE_RESET = 8193,
        SDL_EVENT_RENDER_DEVICE_LOST = 8194,
        SDL_EVENT_PRIVATE0 = 16384,
        SDL_EVENT_PRIVATE1 = 16385,
        SDL_EVENT_PRIVATE2 = 16386,
        SDL_EVENT_PRIVATE3 = 16387,
        SDL_EVENT_POLL_SENTINEL = 32512,
        SDL_EVENT_USER = 32768,
        SDL_EVENT_LAST = 65535,
        SDL_EVENT_ENUM_PADDING = 2147483647,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_CommonEvent
    {
        public uint type;
        public uint reserved;
        public ulong timestamp;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_DisplayEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint displayID;
        public int data1;
        public int data2;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_WindowEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public int data1;
        public int data2;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_KeyboardDeviceEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_KeyboardEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public SDL_Scancode scancode;
        public uint key;
        public SDL_Keymod mod;
        public ushort raw;
        public SDLBool down;
        public SDLBool repeat;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_TextEditingEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public byte* text;
        public int start;
        public int length;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_TextEditingCandidatesEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public byte** candidates;
        public int num_candidates;
        public int selected_candidate;
        public SDLBool horizontal;
        public byte padding1;
        public byte padding2;
        public byte padding3;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_TextInputEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public byte* text;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_MouseDeviceEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_MouseMotionEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public SDL_MouseButtonFlags state;
        public float x;
        public float y;
        public float xrel;
        public float yrel;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_MouseButtonEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public byte button;
        public SDLBool down;
        public byte clicks;
        public byte padding;
        public float x;
        public float y;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_MouseWheelEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public float x;
        public float y;
        public SDL_MouseWheelDirection direction;
        public float mouse_x;
        public float mouse_y;
        public int integer_x;
        public int integer_y;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_JoyAxisEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public byte axis;
        public byte padding1;
        public byte padding2;
        public byte padding3;
        public short value;
        public ushort padding4;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_JoyBallEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public byte ball;
        public byte padding1;
        public byte padding2;
        public byte padding3;
        public short xrel;
        public short yrel;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_JoyHatEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public byte hat;
        public byte value;
        public byte padding1;
        public byte padding2;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_JoyButtonEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public byte button;
        public SDLBool down;
        public byte padding1;
        public byte padding2;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_JoyDeviceEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_JoyBatteryEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public SDL_PowerState state;
        public int percent;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_GamepadAxisEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public byte axis;
        public byte padding1;
        public byte padding2;
        public byte padding3;
        public short value;
        public ushort padding4;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_GamepadButtonEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public byte button;
        public SDLBool down;
        public byte padding1;
        public byte padding2;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_GamepadDeviceEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_GamepadTouchpadEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public int touchpad;
        public int finger;
        public float x;
        public float y;
        public float pressure;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_GamepadSensorEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public int sensor;
        public fixed float data[3];
        public ulong sensor_timestamp;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_AudioDeviceEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public SDLBool recording;
        public byte padding1;
        public byte padding2;
        public byte padding3;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_CameraDeviceEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_RenderEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_TouchFingerEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public ulong touchID;
        public ulong fingerID;
        public float x;
        public float y;
        public float dx;
        public float dy;
        public float pressure;
        public uint windowID;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_PinchFingerEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public float scale;
        public uint windowID;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_PenProximityEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_PenMotionEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public SDL_PenInputFlags pen_state;
        public float x;
        public float y;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_PenTouchEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public SDL_PenInputFlags pen_state;
        public float x;
        public float y;
        public SDLBool eraser;
        public SDLBool down;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_PenButtonEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public SDL_PenInputFlags pen_state;
        public float x;
        public float y;
        public byte button;
        public SDLBool down;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_PenAxisEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public uint which;
        public SDL_PenInputFlags pen_state;
        public float x;
        public float y;
        public SDL_PenAxis axis;
        public float value;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_DropEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public float x;
        public float y;
        public byte* source;
        public byte* data;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_ClipboardEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public SDLBool owner;
        public int num_mime_types;
        public byte** mime_types;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_SensorEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
        public uint which;
        public fixed float data[6];
        public ulong sensor_timestamp;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_QuitEvent
    {
        public SDL_EventType type;
        public uint reserved;
        public ulong timestamp;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SDL_UserEvent
    {
        public uint type;
        public uint reserved;
        public ulong timestamp;
        public uint windowID;
        public int code;
        public IntPtr data1;
        public IntPtr data2;
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct SDL_Event
    {
        [FieldOffset(0)]
        public SDL_EventType type;
        [FieldOffset(0)]
        public SDL_CommonEvent common;
        [FieldOffset(0)]
        public SDL_DisplayEvent display;
        [FieldOffset(0)]
        public SDL_WindowEvent window;
        [FieldOffset(0)]
        public SDL_KeyboardDeviceEvent kdevice;
        [FieldOffset(0)]
        public SDL_KeyboardEvent key;
        [FieldOffset(0)]
        public SDL_TextEditingEvent edit;
        [FieldOffset(0)]
        public SDL_TextEditingCandidatesEvent edit_candidates;
        [FieldOffset(0)]
        public SDL_TextInputEvent text;
        [FieldOffset(0)]
        public SDL_MouseDeviceEvent mdevice;
        [FieldOffset(0)]
        public SDL_MouseMotionEvent motion;
        [FieldOffset(0)]
        public SDL_MouseButtonEvent button;
        [FieldOffset(0)]
        public SDL_MouseWheelEvent wheel;
        [FieldOffset(0)]
        public SDL_JoyDeviceEvent jdevice;
        [FieldOffset(0)]
        public SDL_JoyAxisEvent jaxis;
        [FieldOffset(0)]
        public SDL_JoyBallEvent jball;
        [FieldOffset(0)]
        public SDL_JoyHatEvent jhat;
        [FieldOffset(0)]
        public SDL_JoyButtonEvent jbutton;
        [FieldOffset(0)]
        public SDL_JoyBatteryEvent jbattery;
        [FieldOffset(0)]
        public SDL_GamepadDeviceEvent gdevice;
        [FieldOffset(0)]
        public SDL_GamepadAxisEvent gaxis;
        [FieldOffset(0)]
        public SDL_GamepadButtonEvent gbutton;
        [FieldOffset(0)]
        public SDL_GamepadTouchpadEvent gtouchpad;
        [FieldOffset(0)]
        public SDL_GamepadSensorEvent gsensor;
        [FieldOffset(0)]
        public SDL_AudioDeviceEvent adevice;
        [FieldOffset(0)]
        public SDL_CameraDeviceEvent cdevice;
        [FieldOffset(0)]
        public SDL_SensorEvent sensor;
        [FieldOffset(0)]
        public SDL_QuitEvent quit;
        [FieldOffset(0)]
        public SDL_UserEvent user;
        [FieldOffset(0)]
        public SDL_TouchFingerEvent tfinger;
        [FieldOffset(0)]
        public SDL_PinchFingerEvent pinch;
        [FieldOffset(0)]
        public SDL_PenProximityEvent pproximity;
        [FieldOffset(0)]
        public SDL_PenTouchEvent ptouch;
        [FieldOffset(0)]
        public SDL_PenMotionEvent pmotion;
        [FieldOffset(0)]
        public SDL_PenButtonEvent pbutton;
        [FieldOffset(0)]
        public SDL_PenAxisEvent paxis;
        [FieldOffset(0)]
        public SDL_RenderEvent render;
        [FieldOffset(0)]
        public SDL_DropEvent drop;
        [FieldOffset(0)]
        public SDL_ClipboardEvent clipboard;
        [FieldOffset(0)]
        public fixed byte padding[128];
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_PollEvent(SDL_Event* @event);
}
