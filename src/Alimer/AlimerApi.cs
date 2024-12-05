// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using Alimer.Graphics;

namespace Alimer;

internal unsafe static partial class AlimerApi
{
    private const string LibraryName = "alimer_native";

#if ALIMER_OWN_LIBRARY_LOADING
    static AlimerApi()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), OnDllImport);
    }

    private static nint OnDllImport(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (TryResolveLibrary(libraryName, assembly, searchPath, out nint nativeLibrary))
        {
            return nativeLibrary;
        }

        return NativeLibrary.Load(libraryName, assembly, searchPath);
    }

    private static bool TryResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath, out nint nativeLibrary)
    {
        nativeLibrary = 0;
        if (libraryName is not LibraryName)
            return false;

        string rid = RuntimeInformation.RuntimeIdentifier;

        string nugetNativeLibsPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native");
        bool isNuGetRuntimeLibrariesDirectoryPresent = Directory.Exists(nugetNativeLibsPath);
        string dllName = LibraryName;

        if (OperatingSystem.IsWindows())
        {
            dllName = $"{LibraryName}.dll";

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
            dllName = $"lib{LibraryName}.so";
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            dllName = $"lib{LibraryName}.dylib";
        }

        if (isNuGetRuntimeLibrariesDirectoryPresent)
        {
            string fullPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native", dllName);

            if (NativeLibrary.TryLoad(fullPath, out nativeLibrary))
            {
                return true;
            }
        }

        if (NativeLibrary.TryLoad(LibraryName, assembly, searchPath, out nativeLibrary))
        {
            return true;
        }

        nativeLibrary = 0;
        return false;
    } 
#endif

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

    #region Image
    internal struct ImageDesc
    {
        public TextureDimension dimension;
        public PixelFormat format;
        public uint width;
        public uint height;
        public uint depthOrArrayLayers;
        public uint mipLevelCount;
    }

    [LibraryImport(LibraryName)]
    public static partial nint alimerImageCreate2D(PixelFormat format, uint width, uint height, uint arrayLayers, uint mipLevelCount);

    [LibraryImport(LibraryName)]
    public static partial nint alimerImageCreateFromMemory(void* data, nuint size);

    [LibraryImport(LibraryName)]
    public static partial void alimerImageDestroy(nint handle);

    [LibraryImport(LibraryName)]
    public static partial void alimerImageGetDesc(nint handle, ImageDesc* desc);

    [LibraryImport(LibraryName)]
    public static partial void* alimerImageGetData(nint handle, nuint* dataSize);
    #endregion

    #region Font
    [LibraryImport(LibraryName)]
    public static partial nint Alimer_FontCreateFromMemory(void* data, nuint size);

    [LibraryImport(LibraryName)]
    public static partial void Alimer_FontDestroy(nint handle);

    [LibraryImport(LibraryName)]
    public static partial void Alimer_FontGetMetrics(nint handle, out int ascent, out int descent, out int linegap);

    [LibraryImport(LibraryName)]
    public static partial int Alimer_FontGetGlyphIndex(nint handle, int codepoint);
    [LibraryImport(LibraryName)]
    public static partial float Alimer_FontGetScale(nint handle, float size);
    [LibraryImport(LibraryName)]
    public static partial float Alimer_FontGetKerning(nint handle, int glyph1, int glyph2, float scale);
    #endregion
}
