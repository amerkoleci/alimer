// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using Alimer.Graphics;

namespace Alimer;

internal unsafe static partial class AlimerApi
{
#if (IOS || TVOS || WEBGL)
    private const string Library = "__Internal";
#else
    private const string Library = "alimer_native";
#endif

    static AlimerApi()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), OnDllImport);
    }

    private static IntPtr OnDllImport(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (TryResolveLibrary(libraryName, assembly, searchPath, out IntPtr nativeLibrary))
        {
            return nativeLibrary;
        }

        return NativeLibrary.Load(libraryName, assembly, searchPath);
    }

    private static bool TryResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath, out nint nativeLibrary)
    {
        nativeLibrary = IntPtr.Zero;
        if (libraryName is not Library)
            return false;

        string rid = RuntimeInformation.RuntimeIdentifier;

        string nugetNativeLibsPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native");
        bool isNuGetRuntimeLibrariesDirectoryPresent = Directory.Exists(nugetNativeLibsPath);
        string dllName = Library;

        if (OperatingSystem.IsWindows())
        {
            dllName = $"{Library}.dll";

            if (!isNuGetRuntimeLibrariesDirectoryPresent)
            {
                rid = RuntimeInformation.ProcessArchitecture switch
                {
                    Architecture.X64 => "win-x64",
                    Architecture.Arm64 => "win-arm64",
                    _ => "win-x64"
                };

                nugetNativeLibsPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native");
                isNuGetRuntimeLibrariesDirectoryPresent = Directory.Exists(nugetNativeLibsPath);
            }
        }
        else if (OperatingSystem.IsLinux())
        {
            dllName = $"lib{Library}.so";
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            dllName = $"lib{Library}.dylib";
        }

        if (isNuGetRuntimeLibrariesDirectoryPresent)
        {
            string fullPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native", dllName);

            if (NativeLibrary.TryLoad(fullPath, out nativeLibrary))
            {
                return true;
            }
        }

        if (NativeLibrary.TryLoad(Library, assembly, searchPath, out nativeLibrary))
        {
            return true;
        }

        nativeLibrary = 0;
        return false;
    }

    //[LibraryImport(Library)]
    //public static partial void Alimer_FreeString(byte* memory);

    public static string ParseUTF8(byte* str)
    {
        byte* ptr = str;
        while (*ptr != 0)
        {
            ptr++;
        }

        return Encoding.UTF8.GetString(str, (int)(ptr - str));
    }

    //public static string ParseUTF8AndFree(byte* str)
    //{
    //    string result = ParseUTF8(str);
    //    Alimer_FreeString(str);
    //    return result;
    //}

    public static byte* ToUTF8(in string str)
    {
        int count = Encoding.UTF8.GetByteCount(str) + 1;
        byte* ptr = (byte*)NativeMemory.Alloc((nuint)count);
        var span = new Span<byte>(ptr, count);
        Encoding.UTF8.GetBytes(str, span);
        span[^1] = 0;
        return ptr;
    }

    public static void FreeUTF8(void* ptr)
    {
        NativeMemory.Free(ptr);
    }

    public static void FreeUTF8(nint ptr)
    {
        NativeMemory.Free(ptr.ToPointer());
    }

    [LibraryImport(Library)]
    public static partial void Alimer_GetVersion(int* major, int* minor, int* patch);

    #region Image
    [LibraryImport(Library)]
    public static partial nint Alimer_ImageCreateFromMemory(void* data, nuint size);

    [LibraryImport(Library)]
    public static partial void Alimer_ImageDestroy(nint image);

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    public static extern TextureDimension Alimer_ImageGetDimension(nint image);

    [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
    public static extern PixelFormat Alimer_ImageGetFormat(nint handle);

    [LibraryImport(Library)]
    public static partial uint Alimer_ImageGetWidth(nint handle, uint level);

    [LibraryImport(Library)]
    public static partial uint Alimer_ImageGetHeight(nint handle, uint level);

    [LibraryImport(Library)]
    public static partial uint Alimer_ImageGetDepth(nint handle, uint level);

    [LibraryImport(Library)]
    public static partial uint Alimer_ImageGetArraySize(nint handle);

    [LibraryImport(Library)]
    public static partial uint Alimer_ImageGetMipLevels(nint handle);

    [LibraryImport(Library)]
    public static partial Bool32 Alimer_ImageIsCubemap(nint handle);

    [LibraryImport(Library)]
    public static partial void* Alimer_ImageGetData(nint handle, out nuint size);

    [LibraryImport(Library)]
    public static partial int Alimer_ImageSaveBmp(nint handle, delegate* unmanaged<nint, void*, uint, void> callback);

    [LibraryImport(Library)]
    public static partial int Alimer_ImageSavePng(nint handle, delegate* unmanaged<nint, void*, uint, void> callback);
    #endregion

    #region Font
    [LibraryImport(Library)]
    public static partial nint Alimer_FontCreateFromMemory(void* data, nuint size);

    [LibraryImport(Library)]
    public static partial void Alimer_FontDestroy(nint handle);

    [LibraryImport(Library)]
    public static partial void Alimer_FontGetMetrics(nint handle, out int ascent, out int descent, out int linegap);

    [LibraryImport(Library)]
    public static partial int Alimer_FontGetGlyphIndex(nint handle, int codepoint);
    [LibraryImport(Library)]
    public static partial float Alimer_FontGetScale(nint handle, float size);
    [LibraryImport(Library)]
    public static partial float Alimer_FontGetKerning(nint handle, int glyph1, int glyph2, float scale);
    #endregion

    #region Audio
    [LibraryImport(Library)]
    public static partial Bool32 Alimer_AudioInit();

    [LibraryImport(Library)]
    public static partial void Alimer_AudioShutdown();

    [LibraryImport(Library)]
    public static partial Bool32 Alimer_AudioStart();

    [LibraryImport(Library)]
    public static partial Bool32 Alimer_AudioStop();

    [LibraryImport(Library)]
    public static partial void Alimer_AudioSetMasterVolume(float value);

    [LibraryImport(Library)]
    public static partial float Alimer_AudioGetMasterVolume();

    [LibraryImport(Library)]
    public static partial uint Alimer_AudioGetOutputChannels();

    [LibraryImport(Library)]
    public static partial uint Alimer_AudioGetOutputSampleRate();
    #endregion
}
