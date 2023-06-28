// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using ALCenum = System.Int32;

namespace Alimer.Audio.OpenAL;

internal static unsafe partial class OpenALNative
{
    private const string LibName = "openal32";

    static OpenALNative()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), OnDllImport);
    }

    private static nint OnDllImport(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (libraryName.Equals(LibName) && TryResolveOpenAL(assembly, searchPath, out nint nativeLibrary))
        {
            return nativeLibrary;
        }

        return IntPtr.Zero;
    }

    private static bool TryResolveOpenAL(Assembly assembly, DllImportSearchPath? searchPath, out IntPtr nativeLibrary)
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            if (NativeLibrary.TryLoad("openal32.dll", assembly, searchPath, out nativeLibrary))
            {
                return true;
            }

            if (NativeLibrary.TryLoad("soft_oal.dll", assembly, searchPath, out nativeLibrary))
            {
                return true;
            }
        }
        else
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                if (NativeLibrary.TryLoad("libopenal.so.1", assembly, searchPath, out nativeLibrary))
                {
                    return true;
                }

                if (NativeLibrary.TryLoad("libopenal.so", assembly, searchPath, out nativeLibrary))
                {
                    return true;
                }
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                if (NativeLibrary.TryLoad("/System/Library/Frameworks/OpenAL.framework/OpenAL", assembly, searchPath, out nativeLibrary))
                {
                    return true;
                }

                if (NativeLibrary.TryLoad("libopenal.dylib", assembly, searchPath, out nativeLibrary))
                {
                    return true;
                }
            }

            if (NativeLibrary.TryLoad("libopenal", assembly, searchPath, out nativeLibrary))
            {
                return true;
            }
        }

        return false;
    }

    /** Context attribute: <int> Hz. */
    public const int ALC_FREQUENCY = 0x1007;

    /** Context attribute: <int> Hz. */
    public const int ALC_REFRESH = 0x1008;

    /** Context attribute: AL_TRUE or AL_FALSE synchronous context? */
    public const int ALC_SYNC = 0x1009;

    /** Context attribute: <int> requested Mono (3D) Sources. */
    public const int ALC_MONO_SOURCES = 0x1010;

    /** Context attribute: <int> requested Stereo Sources. */
    public const int ALC_STEREO_SOURCES = 0x1011;

    /** No error. */
    public const int ALC_NO_ERROR = 0;

    /** Invalid device handle. */
    public const int ALC_INVALID_DEVICE = 0xA001;

    /** Invalid context handle. */
    public const int ALC_INVALID_CONTEXT = 0xA002;

    /** Invalid enumeration passed to an ALC call. */
    public const int ALC_INVALID_ENUM = 0xA003;

    /** Invalid value passed to an ALC call. */
    public const int ALC_INVALID_VALUE = 0xA004;

    /** Out of memory. */
    public const int ALC_OUT_OF_MEMORY = 0xA005;


    /** Runtime ALC major version. */
    public const int ALC_MAJOR_VERSION = 0x1000;
    /** Runtime ALC minor version. */
    public const int ALC_MINOR_VERSION = 0x1001;

    /** Context attribute list size. */
    public const int ALC_ATTRIBUTES_SIZE = 0x1002;
    /** Context attribute list properties. */
    public const int ALC_ALL_ATTRIBUTES = 0x1003;

    /** String for the default device specifier. */
    public const int ALC_DEFAULT_DEVICE_SPECIFIER = 0x1004;
    /**
     * Device specifier string.
     *
     * If device handle is NULL, it is instead a null-character separated list of
     * strings of known device specifiers (list ends with an empty string).
     */
    public const int ALC_DEVICE_SPECIFIER = 0x1005;
    /** String for space-separated list of ALC extensions. */
    public const int ALC_EXTENSIONS = 0x1006;

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct ALCdevice : IEquatable<ALCdevice>
    {
        public ALCdevice(nint handle) { Handle = handle; }
        public nint Handle { get; }
        public bool IsNull => Handle == 0;
        public bool IsNotNull => Handle != 0;
        public static ALCdevice Null => new(0);
        public static implicit operator ALCdevice(nint handle) => new(handle);
        public static bool operator ==(ALCdevice left, ALCdevice right) => left.Handle == right.Handle;
        public static bool operator !=(ALCdevice left, ALCdevice right) => left.Handle != right.Handle;
        public static bool operator ==(ALCdevice left, nint right) => left.Handle == right;
        public static bool operator !=(ALCdevice left, nint right) => left.Handle != right;
        public bool Equals(ALCdevice other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is ALCdevice handle && Equals(handle);
        /// <inheritdoc/>
        public override int GetHashCode() => Handle.GetHashCode();
        private string DebuggerDisplay => $"{nameof(ALCdevice)} [0x{Handle.ToString("X")}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct ALCcontext : IEquatable<ALCcontext>
    {
        public ALCcontext(nint handle) { Handle = handle; }
        public nint Handle { get; }
        public bool IsNull => Handle == 0;
        public bool IsNotNull => Handle != 0;
        public static ALCcontext Null => new(0);
        public static implicit operator ALCcontext(nint handle) => new(handle);
        public static bool operator ==(ALCcontext left, ALCcontext right) => left.Handle == right.Handle;
        public static bool operator !=(ALCcontext left, ALCcontext right) => left.Handle != right.Handle;
        public static bool operator ==(ALCcontext left, nint right) => left.Handle == right;
        public static bool operator !=(ALCcontext left, nint right) => left.Handle != right;
        public bool Equals(ALCcontext other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is ALCcontext handle && Equals(handle);
        /// <inheritdoc/>
        public override int GetHashCode() => Handle.GetHashCode();
        private string DebuggerDisplay => $"{nameof(ALCcontext)} [0x{Handle.ToString("X")}]";
    }

    [LibraryImport(LibName)]
    public static partial ALCdevice alcOpenDevice(sbyte* name);

    [LibraryImport(LibName)]
    public static partial byte alcCloseDevice(ALCdevice handle);

    [LibraryImport(LibName)]
    public static partial ALCenum alcGetError(ALCdevice handle);

    [LibraryImport(LibName)]
    public static partial byte alcIsExtensionPresent(ALCdevice handle, sbyte* name);

    [LibraryImport(LibName)]
    public static partial nint alcGetProcAddress(ALCdevice handle, sbyte* funcName);

    [LibraryImport(LibName)]
    public static partial int alcGetEnumValue(ALCdevice handle, sbyte* funcName);

    [LibraryImport(LibName)]
    public static partial sbyte* alcGetString(ALCdevice handle, ALCenum param);

    [LibraryImport(LibName)]
    public static partial void alcGetIntegerv(ALCdevice handle, ALCenum param, int size, int* values);

    [LibraryImport(LibName)]
    public static partial ALCcontext alcCreateContext(ALCdevice device, int* attrlist);

    [LibraryImport(LibName)]
    public static partial byte alcMakeContextCurrent(ALCcontext context);

    [LibraryImport(LibName)]
    public static partial void alcProcessContext(ALCcontext context);

    [LibraryImport(LibName)]
    public static partial void alcSuspendContext(ALCcontext context);

    [LibraryImport(LibName)]
    public static partial void alcDestroyContext(ALCcontext context);

    [LibraryImport(LibName)]
    public static partial ALCcontext alcGetCurrentContext();

    [LibraryImport(LibName)]
    public static partial ALCdevice alcGetContextsDevice(ALCcontext context);

    [LibraryImport(LibName)]
    public static partial ALCdevice alcCaptureOpenDevice(sbyte* name, uint frequency, ALCenum format, int buffersize);

    [LibraryImport(LibName)]
    public static partial byte alcCaptureCloseDevice(ALCdevice device);

    [LibraryImport(LibName)]
    public static partial void alcCaptureStart(ALCdevice device);

    [LibraryImport(LibName)]
    public static partial void alcCaptureStop(ALCdevice device);

    [LibraryImport(LibName)]
    public static partial void alcCaptureSamples(ALCdevice device, void* buffer, int samples);
}
