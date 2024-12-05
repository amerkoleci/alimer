// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using System.Runtime.InteropServices;

namespace Alimer.Bindings.OpenAL;

public static partial class OpenAL
{
    private const string LibraryName = "openal";

    /** Context attribute: <int> Hz. */
    public const int ALC_FREQUENCY = 0x1007;
    public const int ALC_REFRESH = 0x1008;

    static OpenAL()
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

    #region Alc
    [LibraryImport(LibraryName)]
    private static unsafe partial ALCdevice alcOpenDevice(byte* deviceName);

    public static unsafe ALCdevice alcOpenDevice(string? deviceName)
    {
        fixed (byte* pDeviceName = deviceName.GetUtf8Span())
        {
            return alcOpenDevice(pDeviceName);
        }
    }

    [LibraryImport(LibraryName)]
    public static partial Bool8 alcCloseDevice(ALCdevice device);

    [LibraryImport(LibraryName)]
    public static partial AlcError alcGetError(ALCdevice device);

    [LibraryImport(LibraryName, EntryPoint = nameof(alcGetString))]
    private static unsafe partial byte* alcGetStringPrivate(ALCdevice device, int param);

    public static unsafe string alcGetString(ALCdevice device, int param)
    {
        return Interop.GetStringOrEmpty(alcGetStringPrivate(device, param));
    }

    public static string alcGetString(ALCdevice device, AlcError param) => alcGetString(device, (int)param);

    public static string alcGetString(ALCdevice device, AlcGetString param) => alcGetString(device, (int)param);
    public static string alcGetString(AlcGetString param) => alcGetString(ALCdevice.Null, (int)param);

    [LibraryImport(LibraryName)]
    public static unsafe partial void alcGetIntegerv(ALCdevice device, AlcGetInteger param, int size, int* values);

    public static unsafe int alcGetInteger(ALCdevice device, AlcGetInteger param)
    {
        int value = default;
        alcGetIntegerv(device, param, 1, &value);
        return value;
    }

    [LibraryImport(LibraryName)]
    public static unsafe partial Bool8 alcIsExtensionPresent(ALCdevice device, byte* extName);

    public static unsafe bool alcIsExtensionPresent(string? extName)
    {
        fixed (byte* pExtName = extName.GetUtf8Span())
        {
            return alcIsExtensionPresent(ALCdevice.Null, pExtName);
        }
    }

    public static unsafe bool alcIsExtensionPresent(ALCdevice device, string? extName)
    {
        fixed (byte* pExtName = extName.GetUtf8Span())
        {
            return alcIsExtensionPresent(device, pExtName);
        }
    }

    [LibraryImport(LibraryName)]
    public static unsafe partial ALCcontext alcCreateContext(ALCdevice device, int* attrlist);

    public static unsafe ALCcontext alcCreateContext(ALCdevice device, Span<int> attributes)
    {
        fixed (int* attrlist = attributes)
            return alcCreateContext(device, attrlist);
    }

    public static unsafe ALCcontext alcCreateContext(ALCdevice device, int[] attributes)
    {
        fixed (int* attrlist = attributes)
            return alcCreateContext(device, attrlist);
    }

    [LibraryImport(LibraryName)]
    public static partial Bool8 alcMakeContextCurrent(ALCcontext context);

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
    public static partial Bool8 alIsEnabled(int capability);

    [LibraryImport(LibraryName)]
    public static partial int alGetError();

    [LibraryImport(LibraryName)]
    public static unsafe partial Bool8 alIsExtensionPresent(byte* extName);

    public static unsafe bool alIsExtensionPresent(string? extName)
    {
        fixed (byte* pExtName = extName.GetUtf8Span())
        {
            return alIsExtensionPresent(pExtName);
        }
    }

    [LibraryImport(LibraryName)]
    public static partial void alDopplerFactor(float value);

    [LibraryImport(LibraryName)]
    public static partial void alDopplerVelocity(float value);

    [LibraryImport(LibraryName)]
    public static partial void alSpeedOfSound(float value);

    [LibraryImport(LibraryName)]
    public static partial void alDistanceModel(AlDistanceModel distanceModel);

    [LibraryImport(LibraryName, EntryPoint = nameof(alGetString))]
    private static unsafe partial byte* alGetStringPrivate(int param);

    public static unsafe string alGetString(int param)
    {
        return Interop.GetStringOrEmpty(alGetStringPrivate(param));
    }

    public static string alGetString(AlGetString param) => alGetString((int)param);
    #endregion
}
