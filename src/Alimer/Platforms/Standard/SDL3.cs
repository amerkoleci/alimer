// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;

#pragma warning disable CS0649

namespace Alimer;

internal unsafe static partial class SDL3
{
    #region Constants
    public static ReadOnlySpan<byte> SDL_HINT_APP_NAME => "SDL_APP_NAME"u8;
    public static ReadOnlySpan<byte> SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4 => "SDL_WINDOWS_CLOSE_ON_ALT_F4"u8;
    public static ReadOnlySpan<byte> SDL_HINT_TOUCH_MOUSE_EVENTS => "SDL_TOUCH_MOUSE_EVENTS"u8;
    public static ReadOnlySpan<byte> SDL_HINT_MOUSE_TOUCH_EVENTS => "SDL_MOUSE_TOUCH_EVENTS"u8;
    public static ReadOnlySpan<byte> SDL_HINT_PEN_MOUSE_EVENTS => "SDL_PEN_MOUSE_EVENTS"u8;
    public static ReadOnlySpan<byte> SDL_HINT_PEN_TOUCH_EVENTS => "SDL_PEN_TOUCH_EVENTS"u8;
    public static ReadOnlySpan<byte> SDL_HINT_IME_IMPLEMENTED_UI => "SDL_IME_IMPLEMENTED_UI"u8;
    public static ReadOnlySpan<byte> SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS => "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"u8;
    #endregion

    #region Enums

    public enum SDL_LogCategory
    {
        SDL_LOG_CATEGORY_APPLICATION,
        SDL_LOG_CATEGORY_ERROR,
        SDL_LOG_CATEGORY_ASSERT,
        SDL_LOG_CATEGORY_SYSTEM,
        SDL_LOG_CATEGORY_AUDIO,
        SDL_LOG_CATEGORY_VIDEO,
        SDL_LOG_CATEGORY_RENDER,
        SDL_LOG_CATEGORY_INPUT,
        SDL_LOG_CATEGORY_TEST,
        SDL_LOG_CATEGORY_GPU,
        SDL_LOG_CATEGORY_RESERVED2,
        SDL_LOG_CATEGORY_RESERVED3,
        SDL_LOG_CATEGORY_RESERVED4,
        SDL_LOG_CATEGORY_RESERVED5,
        SDL_LOG_CATEGORY_RESERVED6,
        SDL_LOG_CATEGORY_RESERVED7,
        SDL_LOG_CATEGORY_RESERVED8,
        SDL_LOG_CATEGORY_RESERVED9,
        SDL_LOG_CATEGORY_RESERVED10,
        SDL_LOG_CATEGORY_CUSTOM,
    }

    public enum SDL_LogPriority
    {
        SDL_LOG_PRIORITY_INVALID,
        SDL_LOG_PRIORITY_TRACE,
        SDL_LOG_PRIORITY_VERBOSE,
        SDL_LOG_PRIORITY_DEBUG,
        SDL_LOG_PRIORITY_INFO,
        SDL_LOG_PRIORITY_WARN,
        SDL_LOG_PRIORITY_ERROR,
        SDL_LOG_PRIORITY_CRITICAL,
        SDL_LOG_PRIORITY_COUNT,
    }

    [Flags]
    public enum SDL_InitFlags : uint
    {
        SDL_INIT_AUDIO = 0x00000010U,
        SDL_INIT_VIDEO = 0x00000020U,
        SDL_INIT_JOYSTICK = 0x00000200U,
        SDL_INIT_HAPTIC = 0x00001000U,
        SDL_INIT_GAMEPAD = 0x00002000U,
        SDL_INIT_EVENTS = 0x00004000U,
        SDL_INIT_SENSOR = 0x00008000U,
        SDL_INIT_CAMERA = 0x00010000U,
    }
    #endregion

    #region Types
    public enum SDL_PropertiesID : uint;

    public readonly struct SDLBool : IEquatable<SDLBool>
    {
        private readonly byte _value;

        public static SDLBool True => new(true);
        public static SDLBool False => new(false);

        internal SDLBool(bool boolValue)
        {
            _value = boolValue ? (byte)1 : (byte)0;
        }

        public static implicit operator bool(SDLBool value) => value._value != 0;
        public static implicit operator SDLBool(bool boolValue) => new(boolValue);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(SDLBool left, SDLBool right) => left.Equals(right);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(SDLBool left, SDLBool right) => !left.Equals(right);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Equals(SDLBool other) => other._value == _value;

        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is SDLBool rawBool && Equals(rawBool);

        /// <inheritdoc/>
        public override int GetHashCode() => _value.GetHashCode();

        public override string ToString() => _value != 0 ? "True" : "False";
    }
    #endregion

    #region Marshallers
    public static string? PtrToStringUTF8(byte* ptr, bool free = false)
    {
        string? s = Marshal.PtrToStringUTF8((IntPtr)ptr);

        if (free)
            SDL_free(ptr);

        return s;
    }

    [CustomMarshaller(typeof(string), MarshalMode.ManagedToUnmanagedOut, typeof(SDLOwnedStringMarshaller))]
    public static class SDLOwnedStringMarshaller
    {
        /// <summary>
        /// Converts an unmanaged string to a managed version.
        /// </summary>
        /// <returns>A managed string.</returns>
        public static string ConvertToManaged(byte* unmanaged)
            => Marshal.PtrToStringUTF8((nint)unmanaged) ?? string.Empty;
    }

    // Custom marshaller for caller-owned strings returned by SDL.
    [CustomMarshaller(typeof(string), MarshalMode.ManagedToUnmanagedOut, typeof(CallerOwnedStringMarshaller))]
    public static class CallerOwnedStringMarshaller
    {
        /// <summary>
        /// Converts an unmanaged string to a managed version.
        /// </summary>
        /// <returns>A managed string.</returns>
        public static string ConvertToManaged(byte* unmanaged)
            => Marshal.PtrToStringUTF8((nint)unmanaged) ?? string.Empty;

        /// <summary>
        /// Free the memory for a specified unmanaged string.
        /// </summary>
        public static void Free(byte* unmanaged)
            => SDL_free(unmanaged);
    }

    #endregion

    private const string LibraryName = "SDL3";

    public enum SDL_KeyboardID : uint;
    public enum SDL_MouseID : uint;
    public enum SDL_TouchID : ulong;
    public enum SDL_FingerID : ulong;
    public enum SDL_SensorID : uint;
    public enum SDL_JoystickID : uint;

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_free(void* mem);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetHint(ReadOnlySpan<byte> name, string value);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetHint(ReadOnlySpan<byte> name, ReadOnlySpan<byte> value);

    public static SDLBool SDL_SetHint(ReadOnlySpan<byte> name, bool value)
    {
        return SDL_SetHint(name, value ? "1"u8 : "0"u8);
    }

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetHint(string name, string value);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_SetLogPriority(int category, SDL_LogPriority priority);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SDL_LogOutputFunction(nint userdata, int category, SDL_LogPriority priority, byte* message);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_SetLogOutputFunction(/*SDL_LogOutputFunction*/delegate* unmanaged[Cdecl]<nint, int, SDL_LogPriority, byte*, void> callback, nint userdata = 0);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_Init(SDL_InitFlags flags);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_Quit();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(SDLOwnedStringMarshaller))]
    public static partial string SDL_GetError();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetVersion();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(SDLOwnedStringMarshaller))]
    public static partial string SDL_GetRevision();


    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_GetPointerProperty(SDL_PropertiesID props, string name, nint defaultValue = 0);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_GetPointerProperty(SDL_PropertiesID props, ReadOnlySpan<byte> name, nint defaultValue = 0);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial long SDL_GetNumberProperty(SDL_PropertiesID props, string name, long defaultValue = 0);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial long SDL_GetNumberProperty(SDL_PropertiesID props, ReadOnlySpan<byte> name, long defaultValue = 0);


    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(SDLOwnedStringMarshaller))]
    public static partial string SDL_GetCurrentVideoDriver();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HasClipboardText();

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetClipboardText(string text);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(CallerOwnedStringMarshaller))]
    public static partial string SDL_GetClipboardText();

    #region Macros
    public static int SDL_VERSIONNUM(int major, int minor, int patch) => ((major) * 1000000 + (minor) * 1000 + (patch));
    public static int SDL_VERSIONNUM_MAJOR(int version) => ((version) / 1000000);
    public static int SDL_VERSIONNUM_MINOR(int version) => (((version) / 1000) % 1000);
    public static int SDL_VERSIONNUM_MICRO(int version) => ((version) % 1000);
    //public static bool SDL_VERSION_ATLEAST(int X, int Y, int Z) => SDL_VERSION >= SDL_VERSIONNUM(X, Y, Z);
    #endregion

    #region Keyboard
    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HasKeyboard();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_KeyboardID* SDL_GetKeyboards(int* count);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial string SDL_GetKeyboardNameForID(SDL_KeyboardID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_Keymod SDL_GetModState();
    #endregion

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HasMouse();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_MouseID* SDL_GetMice(int* count);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial string SDL_GetMouseNameForID(SDL_MouseID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_MouseButtonFlags SDL_GetMouseState(out float x, out float y);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_MouseButtonFlags SDL_GetGlobalMouseState(out float x, out float y);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_WarpMouseInWindow(nint window, float x, float y);
    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_WarpMouseGlobal(float x, float y);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_CaptureMouse(SDLBool enabled);

    public enum SDL_SystemCursor
    {
        SDL_SYSTEM_CURSOR_DEFAULT,
        SDL_SYSTEM_CURSOR_TEXT,
        SDL_SYSTEM_CURSOR_WAIT,
        SDL_SYSTEM_CURSOR_CROSSHAIR,
        SDL_SYSTEM_CURSOR_PROGRESS,
        SDL_SYSTEM_CURSOR_NWSE_RESIZE,
        SDL_SYSTEM_CURSOR_NESW_RESIZE,
        SDL_SYSTEM_CURSOR_EW_RESIZE,
        SDL_SYSTEM_CURSOR_NS_RESIZE,
        SDL_SYSTEM_CURSOR_MOVE,
        SDL_SYSTEM_CURSOR_NOT_ALLOWED,
        SDL_SYSTEM_CURSOR_POINTER,
        SDL_SYSTEM_CURSOR_NW_RESIZE,
        SDL_SYSTEM_CURSOR_N_RESIZE,
        SDL_SYSTEM_CURSOR_NE_RESIZE,
        SDL_SYSTEM_CURSOR_E_RESIZE,
        SDL_SYSTEM_CURSOR_SE_RESIZE,
        SDL_SYSTEM_CURSOR_S_RESIZE,
        SDL_SYSTEM_CURSOR_SW_RESIZE,
        SDL_SYSTEM_CURSOR_W_RESIZE,
        SDL_SYSTEM_CURSOR_COUNT,
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_CreateSystemCursor(SDL_SystemCursor id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetCursor(nint cursor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_GetCursor();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_GetDefaultCursor();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_DestroyCursor(nint cursor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_ShowCursor();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HideCursor();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_CursorVisible();

    #region Touch
    public enum SDL_TouchDeviceType
    {
        SDL_TOUCH_DEVICE_INVALID = -1,
        SDL_TOUCH_DEVICE_DIRECT,
        SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE,
        SDL_TOUCH_DEVICE_INDIRECT_RELATIVE,
    }

    public partial struct SDL_Finger
    {
        public SDL_FingerID id;

        public float x;

        public float y;

        public float pressure;
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_TouchID* SDL_GetTouchDevices(out int count);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial string SDL_GetTouchDeviceName(SDL_TouchID touchID);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_TouchDeviceType SDL_GetTouchDeviceType(SDL_TouchID touchID);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_Finger** SDL_GetTouchFingers(SDL_TouchID touchID, int* count);
    #endregion

    #region Sensor
    public partial struct SDL_Sensor
    {
    }

    public enum SDL_SensorType
    {
        SDL_SENSOR_INVALID = -1,
        SDL_SENSOR_UNKNOWN,
        SDL_SENSOR_ACCEL,
        SDL_SENSOR_GYRO,
        SDL_SENSOR_ACCEL_L,
        SDL_SENSOR_GYRO_L,
        SDL_SENSOR_ACCEL_R,
        SDL_SENSOR_GYRO_R,
        SDL_SENSOR_COUNT,
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_SensorID* SDL_GetSensors(int* count);

    [LibraryImport(LibraryName, EntryPoint = "SDL_GetSensorNameForID")]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial byte* Unsafe_SDL_GetSensorNameForID(SDL_SensorID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_SensorType SDL_GetSensorTypeForID(SDL_SensorID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetSensorNonPortableTypeForID(SDL_SensorID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_Sensor* SDL_OpenSensor(SDL_SensorID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_Sensor* SDL_GetSensorFromID(SDL_SensorID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_PropertiesID SDL_GetSensorProperties(SDL_Sensor* sensor);

    [LibraryImport(LibraryName, EntryPoint = "SDL_GetSensorName")]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial byte* Unsafe_SDL_GetSensorName(SDL_Sensor* sensor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_SensorType SDL_GetSensorType(SDL_Sensor* sensor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetSensorNonPortableType(SDL_Sensor* sensor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_SensorID SDL_GetSensorID(SDL_Sensor* sensor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetSensorData(SDL_Sensor* sensor, float* data, int num_values);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_CloseSensor(SDL_Sensor* sensor);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_UpdateSensors();

    public const float SDL_STANDARD_GRAVITY = 9.80665f;
    #endregion

    #region Joystick + Gamepad
    public enum SDL_JoystickType
    {
        SDL_JOYSTICK_TYPE_UNKNOWN,
        SDL_JOYSTICK_TYPE_GAMEPAD,
        SDL_JOYSTICK_TYPE_WHEEL,
        SDL_JOYSTICK_TYPE_ARCADE_STICK,
        SDL_JOYSTICK_TYPE_FLIGHT_STICK,
        SDL_JOYSTICK_TYPE_DANCE_PAD,
        SDL_JOYSTICK_TYPE_GUITAR,
        SDL_JOYSTICK_TYPE_DRUM_KIT,
        SDL_JOYSTICK_TYPE_ARCADE_PAD,
        SDL_JOYSTICK_TYPE_THROTTLE,
        SDL_JOYSTICK_TYPE_COUNT,
    }

    public enum SDL_JoystickConnectionState
    {
        SDL_JOYSTICK_CONNECTION_INVALID = -1,
        SDL_JOYSTICK_CONNECTION_UNKNOWN,
        SDL_JOYSTICK_CONNECTION_WIRED,
        SDL_JOYSTICK_CONNECTION_WIRELESS,
    }


    public enum SDL_GamepadType
    {
        SDL_GAMEPAD_TYPE_UNKNOWN = 0,
        SDL_GAMEPAD_TYPE_STANDARD,
        SDL_GAMEPAD_TYPE_XBOX360,
        SDL_GAMEPAD_TYPE_XBOXONE,
        SDL_GAMEPAD_TYPE_PS3,
        SDL_GAMEPAD_TYPE_PS4,
        SDL_GAMEPAD_TYPE_PS5,
        SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO,
        SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,
        SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
        SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR,
        SDL_GAMEPAD_TYPE_GAMECUBE,
        SDL_GAMEPAD_TYPE_COUNT,
    }

    public enum SDL_GamepadButton
    {
        SDL_GAMEPAD_BUTTON_INVALID = -1,
        SDL_GAMEPAD_BUTTON_SOUTH,
        SDL_GAMEPAD_BUTTON_EAST,
        SDL_GAMEPAD_BUTTON_WEST,
        SDL_GAMEPAD_BUTTON_NORTH,
        SDL_GAMEPAD_BUTTON_BACK,
        SDL_GAMEPAD_BUTTON_GUIDE,
        SDL_GAMEPAD_BUTTON_START,
        SDL_GAMEPAD_BUTTON_LEFT_STICK,
        SDL_GAMEPAD_BUTTON_RIGHT_STICK,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
        SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
        SDL_GAMEPAD_BUTTON_DPAD_UP,
        SDL_GAMEPAD_BUTTON_DPAD_DOWN,
        SDL_GAMEPAD_BUTTON_DPAD_LEFT,
        SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_MISC1,
        SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,
        SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,
        SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,
        SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,
        SDL_GAMEPAD_BUTTON_TOUCHPAD,
        SDL_GAMEPAD_BUTTON_MISC2,
        SDL_GAMEPAD_BUTTON_MISC3,
        SDL_GAMEPAD_BUTTON_MISC4,
        SDL_GAMEPAD_BUTTON_MISC5,
        SDL_GAMEPAD_BUTTON_MISC6,
        SDL_GAMEPAD_BUTTON_COUNT,
    }

    public enum SDL_GamepadButtonLabel
    {
        SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN,
        SDL_GAMEPAD_BUTTON_LABEL_A,
        SDL_GAMEPAD_BUTTON_LABEL_B,
        SDL_GAMEPAD_BUTTON_LABEL_X,
        SDL_GAMEPAD_BUTTON_LABEL_Y,
        SDL_GAMEPAD_BUTTON_LABEL_CROSS,
        SDL_GAMEPAD_BUTTON_LABEL_CIRCLE,
        SDL_GAMEPAD_BUTTON_LABEL_SQUARE,
        SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE,
    }

    public enum SDL_GamepadAxis
    {
        SDL_GAMEPAD_AXIS_INVALID = -1,
        SDL_GAMEPAD_AXIS_LEFTX,
        SDL_GAMEPAD_AXIS_LEFTY,
        SDL_GAMEPAD_AXIS_RIGHTX,
        SDL_GAMEPAD_AXIS_RIGHTY,
        SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
        SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
        SDL_GAMEPAD_AXIS_COUNT,
    }

    public enum SDL_GamepadBindingType
    {
        SDL_GAMEPAD_BINDTYPE_NONE = 0,
        SDL_GAMEPAD_BINDTYPE_BUTTON,
        SDL_GAMEPAD_BINDTYPE_AXIS,
        SDL_GAMEPAD_BINDTYPE_HAT,
    }

    public partial struct SDL_Gamepad
    {
    }

    public partial struct SDL_Joystick
    {
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HasGamepad();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_JoystickID* SDL_GetGamepads(int* count);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_IsGamepad(SDL_JoystickID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_Gamepad* SDL_OpenGamepad(SDL_JoystickID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_Gamepad* SDL_GetGamepadFromID(SDL_JoystickID instance_id);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_CloseGamepad(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial string SDL_GetGamepadName(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_GamepadType SDL_GetGamepadType(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_GamepadType SDL_GetGamepadTypeForID(SDL_JoystickID instance_id);


    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial ushort SDL_GetGamepadVendor(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial ushort SDL_GetGamepadProduct(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial ushort SDL_GetGamepadProductVersion(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial ushort SDL_GetGamepadFirmwareVersion(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_JoystickConnectionState SDL_GetGamepadConnectionState(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_PowerState SDL_GetGamepadPowerInfo(SDL_Gamepad* gamepad, int* percent);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GamepadConnected(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetGamepadPlayerIndex(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_UpdateGamepads();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GamepadHasAxis(SDL_Gamepad* gamepad, SDL_GamepadAxis axis);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial short SDL_GetGamepadAxis(SDL_Gamepad* gamepad, SDL_GamepadAxis axis);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GamepadHasButton(SDL_Gamepad* gamepad, SDL_GamepadButton button);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetGamepadButton(SDL_Gamepad* gamepad, SDL_GamepadButton button);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetNumGamepadTouchpads(SDL_Gamepad* gamepad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetNumGamepadTouchpadFingers(SDL_Gamepad* gamepad, int touchpad);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetGamepadTouchpadFinger(SDL_Gamepad* gamepad, int touchpad, int finger, SDLBool* down, float* x, float* y, float* pressure);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GamepadHasSensor(SDL_Gamepad* gamepad, SDL_SensorType type);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetGamepadSensorEnabled(SDL_Gamepad* gamepad, SDL_SensorType type, SDLBool enabled);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GamepadSensorEnabled(SDL_Gamepad* gamepad, SDL_SensorType type);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial float SDL_GetGamepadSensorDataRate(SDL_Gamepad* gamepad, SDL_SensorType type);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetGamepadSensorData(SDL_Gamepad* gamepad, SDL_SensorType type, float* data, int num_values);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_RumbleGamepad(SDL_Gamepad* gamepad, ushort low_frequency_rumble, ushort high_frequency_rumble, uint duration_ms);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_RumbleGamepadTriggers(SDL_Gamepad* gamepad, ushort left_rumble, ushort right_rumble, uint duration_ms);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetGamepadLED(SDL_Gamepad* gamepad, byte red, byte green, byte blue);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SendGamepadEffect(SDL_Gamepad* gamepad, nint data, int size);
    #endregion

    #region PowerStatus
    public enum SDL_PowerState
    {
        SDL_POWERSTATE_ERROR = -1,
        SDL_POWERSTATE_UNKNOWN,
        SDL_POWERSTATE_ON_BATTERY,
        SDL_POWERSTATE_NO_BATTERY,
        SDL_POWERSTATE_CHARGING,
        SDL_POWERSTATE_CHARGED,
    }

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_PowerState SDL_GetPowerInfo(int* seconds, int* percent);
    #endregion

    #region Extensions
    private static void logError(string? expression)
    {
        if (!string.IsNullOrEmpty(expression))
            Log.Error($"SDL error: {SDL_GetError()} at {expression}");
        else
            Log.Error($"SDL error: {SDL_GetError()}");
    }

    public static SDLBool LogErrorIfFailed(this SDLBool returnValue, [CallerArgumentExpression(nameof(returnValue))] string? expression = null)
    {
        if (!returnValue)
            logError(expression);

        return returnValue;
    }

    public static int LogErrorIfFailed(this int returnValue, [CallerArgumentExpression("returnValue")] string? expression = null)
    {
        if (returnValue == -1)
            logError(expression);

        return returnValue;
    }
    #endregion

}
