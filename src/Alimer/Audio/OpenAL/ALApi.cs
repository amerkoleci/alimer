// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;

namespace Alimer.Audio.OpenAL;

internal unsafe static partial class ALApi
{
    private const string LibraryName = "openal";

    /** Context attribute: <int> Hz. */
    public const int ALC_FREQUENCY = 0x1007;
    public const int ALC_REFRESH = 0x1008;

    /** Context attribute: AL_TRUE or AL_FALSE synchronous context? */
    public const int ALC_SYNC = 0x1009;

    /** Context attribute: <int> requested Mono (3D) Sources. */
    public const int ALC_MONO_SOURCES = 0x1010;

    /** Context attribute: <int> requested Stereo Sources. */
    public const int ALC_STEREO_SOURCES = 0x1011;

    /* Source type values. */
    public const int AL_STATIC = 0x1028;
    public const int AL_STREAMING = 0x1029;
    public const int AL_UNDETERMINED = 0x1030;

    /// <summary>
    /// Unsigned 8-bit mono buffer format.
    /// </summary>
    public const int AL_FORMAT_MONO8 = 0x1100;
    /// <summary>
    /// Signed 16-bit mono buffer format.
    /// </summary>
    public const int AL_FORMAT_MONO16 = 0x1101;
    /// <summary>
    /// Unsigned 8-bit stereo buffer format.
    /// </summary>
    public const int AL_FORMAT_STEREO8 = 0x1102;
    /// <summary>
    /// Signed 16-bit stereo buffer format.
    /// </summary>
    public const int AL_FORMAT_STEREO16 = 0x1103;

    /// <summary>
    /// Buffer frequency/sample rate (query only).
    /// </summary>
    public const int AL_FREQUENCY = 0x2001;
    /// <summary>
    /// Buffer bits per sample (query only).
    /// </summary>
    public const int AL_BITS = 0x2002;
    /// <summary>
    /// Buffer channel count (query only). 
    /// </summary>
    public const int AL_CHANNELS = 0x2003;
    /// <summary>
    /// Buffer data size in bytes (query only).
    /// </summary>
    public const int AL_SIZE = 0x2004;


#if TODO
    static ALApi()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), OnDllImport);
    }

    private static nint OnDllImport(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (libraryName.Equals(LibraryName) && TryResolveOpenAL(assembly, searchPath, out nint nativeLibrary))
        {
            return nativeLibrary;
        }

        return IntPtr.Zero;
    }

    private static bool TryResolveOpenAL(Assembly assembly, DllImportSearchPath? searchPath, out IntPtr nativeLibrary)
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            if (NativeLibrary.TryLoad("OpenAL32.dll", assembly, searchPath, out nativeLibrary))
            {
                return true;
            }
        }
        else
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                if (NativeLibrary.TryLoad("libopenal.so", assembly, searchPath, out nativeLibrary))
                {
                    return true;
                }
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
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
#endif

    #region Enums
    public enum AlcError
    {
        NoError = 0,
        InvalidDevice = 0xA001,
        InvalidContext = 0xA002,
        InvalidEnum = 0xA003,
        InvalidValue = 0xA004,
        OutOfMemory = 0xA005,
    }

    public enum AlcGetInteger
    {
        /// <summary>
        /// Runtime ALC major version.
        /// </summary>
        MajorVersion = 0x1000,
        /// <summary>
        /// Runtime ALC minor version.
        /// </summary>
        MinorVersion = 0x1001,
        /// <summary>
        /// Context attribute list size.
        /// </summary>
        AttributesSize = 0x1002,

        Frequency = 0x1007,
        Refresh = 0x1008,
        Sync = 0x1009,
        MonoSources = 0x1010,
        StereoSources = 0x1011,

        /// <summary>
        /// ALC_EXT_CAPTURE - Number of sample frames available for capture.
        /// </summary>
        CaptureSamples = 0x312,
    }

    public enum AlcGetString
    {
        /// <summary>
        /// Context attribute list properties.
        /// </summary>
        AllAttributes = 0x1003,
        /// <summary>
        /// String for the default device specifier.
        /// </summary>
        DefaultDeviceSpecifier = 0x1004,
        /// <summary>
        /// Device specifier string.
        ///
        /// If device handle is NULL, it is instead a null-character separated list of strings of known device specifiers (list ends with an empty string).
        /// </summary>
        DeviceSpecifier = 0x1005,
        /// <summary>
        /// String for space-separated list of ALC extensions.
        /// </summary>
        Extensions = 0x1006,

        /// <summary>
        /// ALC_EXT_CAPTURE - Capture device specifier string.
        /// </summary>
        CaptureDeviceSpecifier = 0x310,

        /// <summary>
        /// ALC_EXT_CAPTURE - String for the default capture device specifier.
        /// </summary>
        CaptureDefaultDeviceSpecifier = 0x311,

        /// <summary>
        /// ALC_ENUMERATE_ALL_EXT - Device's extended specifier string.
        /// </summary>
        AllDevicesSpecifier = 0x1013,
    }

    public enum AlGetString
    {
        /// <summary>
        /// Context string: Vendor name.
        /// </summary>
        Vendor = 0xB001,
        /// <summary>
        /// Context string: Version.
        /// </summary>
        Version = 0xB002,
        /// <summary>
        /// Context string: Renderer name.
        /// </summary>
        Renderer = 0xB003,
        /// <summary>
        /// Context string: Space-separated extension list.
        /// </summary>
        Extensions = 0xB004
    }

    public enum AlDistanceModel
    {
        /// <summary>
        /// Bypasses all distance attenuation calculation for all Sources.
        /// </summary>
        None = 0,

        /// <summary>
        /// InverseDistance is equivalent to the IASIG I3DL2 model with the exception that SourceFloat.ReferenceDistance
        /// does not imply any clamping.
        /// </summary>
        InverseDistance = 0xD001,

        /// <summary>
        /// InverseDistanceClamped is the IASIG I3DL2 model, with SourceFloat.ReferenceDistance indicating both the
        /// reference distance and the distance below which gain will be clamped.
        /// </summary>
        InverseDistanceClamped = 0xD002,

        /// <summary>
        /// AL_EXT_LINEAR_DISTANCE extension.
        /// </summary>
        LinearDistance = 0xD003,

        /// <summary>
        /// AL_EXT_LINEAR_DISTANCE extension.
        /// </summary>
        LinearDistanceClamped = 0xD004,

        /// <summary>
        /// AL_EXT_EXPONENT_DISTANCE extension.
        /// </summary>
        ExponentDistance = 0xD005,

        /// <summary>
        /// AL_EXT_EXPONENT_DISTANCE extension.
        /// </summary>
        ExponentDistanceClamped = 0xD006
    }
    #endregion

    #region Handles
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct ALCcontext(nint handle) : IEquatable<ALCcontext>
    {
        public nint Handle { get; } = handle; public bool IsNull => Handle == 0;
        public bool IsNotNull => Handle != 0;
        public static ALCcontext Null => new(0);
        public static implicit operator ALCcontext(nint handle) => new(handle);
        public static implicit operator nint(ALCcontext handle) => handle.Handle;
        public static bool operator ==(ALCcontext left, ALCcontext right) => left.Handle == right.Handle;
        public static bool operator !=(ALCcontext left, ALCcontext right) => left.Handle != right.Handle;
        public static bool operator ==(ALCcontext left, nint right) => left.Handle == right;
        public static bool operator !=(ALCcontext left, nint right) => left.Handle != right;
        public bool Equals(ALCcontext other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is ALCcontext handle && Equals(handle);
        /// <inheritdoc/>
        public override int GetHashCode() => Handle.GetHashCode();
        private string DebuggerDisplay => $"{nameof(ALCcontext)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct ALCdevice(nint handle) : IEquatable<ALCdevice>
    {
        public nint Handle { get; } = handle;
        public bool IsNull => Handle == 0;
        public bool IsNotNull => Handle != 0;
        public static ALCdevice Null => new(0);
        public static implicit operator ALCdevice(nint handle) => new(handle);
        public static implicit operator nint(ALCdevice handle) => handle.Handle;
        public static bool operator ==(ALCdevice left, ALCdevice right) => left.Handle == right.Handle;
        public static bool operator !=(ALCdevice left, ALCdevice right) => left.Handle != right.Handle;
        public static bool operator ==(ALCdevice left, nint right) => left.Handle == right;
        public static bool operator !=(ALCdevice left, nint right) => left.Handle != right;
        public bool Equals(ALCdevice other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is ALCdevice handle && Equals(handle);
        /// <inheritdoc/>
        public override int GetHashCode() => Handle.GetHashCode();
        private string DebuggerDisplay => $"{nameof(ALCdevice)} [0x{Handle:X}]";
    }

    #endregion

    #region Alc
    [LibraryImport(LibraryName)]
    public static unsafe partial ALCdevice alcOpenDevice(ReadOnlySpan<byte> deviceName);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static unsafe partial ALCdevice alcOpenDevice(string? deviceName);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alcCloseDevice(ALCdevice device);

    [LibraryImport(LibraryName)]
    public static partial AlcError alcGetError(ALCdevice device);

    [LibraryImport(LibraryName, EntryPoint = nameof(alcGetString))]
    private static partial byte* alcGetStringUnsafe(ALCdevice device, int param);

    public static string? alcGetString(ALCdevice device, int param)
    {
        return Utf8StringMarshaller.ConvertToManaged(alcGetStringUnsafe(device, param));
    }

    public static string? alcGetString(ALCdevice device, AlcGetString param)
    {
        return Utf8StringMarshaller.ConvertToManaged(alcGetStringUnsafe(device, (int)param));
    }

    [LibraryImport(LibraryName)]
    public static partial void alcGetIntegerv(ALCdevice device, AlcGetInteger param, int size, int* values);

    public static int alcGetInteger(ALCdevice device, AlcGetInteger param)
    {
        int value = default;
        alcGetIntegerv(device, param, 1, &value);
        return value;
    }

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alcIsExtensionPresent(ALCdevice device, ReadOnlySpan<byte> extName);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alcIsExtensionPresent(ALCdevice device, string extName);

    [LibraryImport(LibraryName)]
    public static partial ALCcontext alcCreateContext(ALCdevice device, int* attrlist);

    [LibraryImport(LibraryName)]
    public static partial ALCcontext alcCreateContext(ALCdevice device, ReadOnlySpan<int> attrlist);

    public static ALCcontext alcCreateContext(ALCdevice device) => alcCreateContext(device, (int*)null);


    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alcMakeContextCurrent(ALCcontext context);

    [LibraryImport(LibraryName)]
    public static partial void alcProcessContext(ALCcontext context);

    [LibraryImport(LibraryName)]
    public static partial void alcSuspendContext(ALCcontext context);

    [LibraryImport(LibraryName)]
    public static partial void alcDestroyContext(ALCcontext context);

    [LibraryImport(LibraryName)]
    public static partial ALCdevice alcGetContextsDevice(ALCcontext context);

    [LibraryImport(LibraryName)]
    public static partial ALCcontext alcGetCurrentContext();
    #endregion

    #region AL
    [LibraryImport(LibraryName)]
    public static partial void alEnable(int capability);

    [LibraryImport(LibraryName)]
    public static partial void alDisable(int capability);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alIsEnabled(int capability);

    [LibraryImport(LibraryName)]
    public static partial int alGetError();

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static unsafe partial bool alIsExtensionPresent(ReadOnlySpan<byte> extName);

    [LibraryImport(LibraryName)]
    public static partial void alDopplerFactor(float value);

    [LibraryImport(LibraryName)]
    public static partial void alDopplerVelocity(float value);

    [LibraryImport(LibraryName)]
    public static partial void alSpeedOfSound(float value);

    [LibraryImport(LibraryName)]
    public static partial void alDistanceModel(AlDistanceModel value);

    [LibraryImport(LibraryName)]
    public static partial int alGetEnumValue(ReadOnlySpan<byte> name);

    [LibraryImport(LibraryName, EntryPoint = nameof(alGetString))]
    private static partial byte* alGetStringUnsafe(int param);

    public static string? alGetString(AlGetString param) => Utf8StringMarshaller.ConvertToManaged(alGetStringUnsafe((int)param));
    #endregion
}
