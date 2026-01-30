// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using Alimer.Input;

namespace Alimer;

internal unsafe static partial class SDL3
{
    #region Constants
    public static ReadOnlySpan<byte> SDL_HINT_APP_NAME => "SDL_APP_NAME"u8;
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

    public readonly record struct SDLBool : IEquatable<SDLBool>
    {
        private readonly byte _value;

        internal const byte FALSE_VALUE = 0;
        internal const byte TRUE_VALUE = 1;

        internal SDLBool(byte value)
        {
            _value = value;
        }

        public static implicit operator bool(SDLBool b)
        {
            return b._value != FALSE_VALUE;
        }

        public static implicit operator SDLBool(bool b)
        {
            return new SDLBool(b ? TRUE_VALUE : FALSE_VALUE);
        }

        public bool Equals(SDLBool other)
        {
            return other._value == _value;
        }

        public override int GetHashCode()
        {
            return _value.GetHashCode();
        }
    }
    #endregion

    #region Structs
    #endregion

    #region Marshallers
    [CustomMarshaller(typeof(string), MarshalMode.ManagedToUnmanagedOut, typeof(SDLOwnedStringMarshaller))]
    public static unsafe class SDLOwnedStringMarshaller
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
    public static unsafe class CallerOwnedStringMarshaller
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
            => SDL_free((IntPtr)unmanaged);
    }

    #endregion

    private const string LibraryName = "SDL3";


    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial void SDL_free(nint mem);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetHint(ReadOnlySpan<byte> name, string value);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetHint(ReadOnlySpan<byte> name, ReadOnlySpan<byte> value);

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

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(SDLOwnedStringMarshaller))]
    public static partial string SDL_GetError();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial int SDL_GetVersion();

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
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


    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    [return: MarshalUsing(typeof(SDLOwnedStringMarshaller))]
    public static partial string SDL_GetCurrentVideoDriver();

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HasClipboardText();

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetClipboardText(string text);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
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

    #region Extensions
    private static void logError(string? expression)
    {
        if (!string.IsNullOrEmpty(expression))
            Log.Error($"SDL error: {SDL_GetError()} at {expression}");
        else
            Log.Error($"SDL error: {SDL_GetError()}");
    }

    public static SDLBool LogErrorIfFailed(this SDLBool returnValue,
        [CallerArgumentExpression(nameof(returnValue))] string? expression = null)
    {
        if (!returnValue)
            logError(expression);

        return returnValue;
    }
    #endregion

}
