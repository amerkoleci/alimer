// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using System.Runtime.InteropServices;

public static partial class OpenAL
{
    private const string LibName = "OpenAL32";

    static OpenAL()
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

    [LibraryImport(LibName)]
    private static unsafe partial nint alcOpenDevice(byte* deviceName);

    public static unsafe nint alcOpenDevice(string? deviceName)
    {
        fixed (byte* pDeviceName = deviceName.GetUtf8Span())
        {
            return alcOpenDevice(pDeviceName);
        }
    }

    [LibraryImport(LibName)]
    public static partial Bool8 alcCloseDevice(nint device);

    [LibraryImport(LibName)]
    public static partial int alGetError();

    [LibraryImport(LibName)]
    public unsafe static partial Bool8 alcIsExtensionPresent(nint device, byte* extName);

    public static unsafe bool alcIsExtensionPresent(nint device, string? extName)
    {
        fixed (byte* pExtName = extName.GetUtf8Span())
        {
            return alcIsExtensionPresent(device, pExtName);
        }
    }
}
