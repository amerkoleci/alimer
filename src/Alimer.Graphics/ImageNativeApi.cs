// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Graphics;

internal static unsafe partial class ImageNativeApi
{
    private const string LibName = "alimer_image";

#if NET6_0_OR_GREATER
    static ImageNativeApi()
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

    private static bool TryResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath, out IntPtr nativeLibrary)
    {
        nativeLibrary = IntPtr.Zero;
        if (libraryName is not LibName)
            return false;

        string rid = RuntimeInformation.RuntimeIdentifier;

        string nugetNativeLibsPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native");
        bool isNuGetRuntimeLibrariesDirectoryPresent = Directory.Exists(nugetNativeLibsPath);
        string dllName = LibName;

        if (OperatingSystem.IsWindows())
        {
            dllName = $"{LibName}.dll";

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
            dllName = $"lib{LibName}.so";
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            dllName = $"lib{LibName}.dylib";
        }

        if (isNuGetRuntimeLibrariesDirectoryPresent)
        {
            string fullPath = Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native", dllName);

            if (NativeLibrary.TryLoad(fullPath, out nativeLibrary))
            {
                return true;
            }
        }

        if (NativeLibrary.TryLoad(LibName, assembly, searchPath, out nativeLibrary))
        {
            return true;
        }

        nativeLibrary = IntPtr.Zero;
        return false;
    }
#endif

    public enum ImageFormat
    {
        ImageFormat_Undefined = 0,
        // 8-bit formats
        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,
        // 16-bit formats
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,
        R16Float,
        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,
        // Packed 16-Bit formats
        BGRA4Unorm,
        B5G6R5Unorm,
        BGR5A1Unorm,
        // 32-bit formats
        R32Uint,
        R32Sint,
        R32Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,
        RG16Float,
        RGBA8Unorm,
        RGBA8UnormSrgb,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        BGRA8Unorm,
        BGRA8UnormSrgb,
        // Packed 32-Bit Pixel Formats
        RGB10A2Unorm,
        RGB10A2Uint,
        RG11B10Ufloat,
        RGB9E5Ufloat,
        // 64-bit formats
        RG32Uint,
        RG32Sint,
        RG32Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        // 128-bit formats
        RGBA32Uint,
        RGBA32Sint,
        RGBA32Float,
        // Depth-stencil formats
        Depth16Unorm,
        Depth24UnormStencil8,
        Depth32Float,
        Depth32FloatStencil8,
        // Bc compressed formats
        BC1RGBAUnorm,
        BC1RGBAUnormSrgb,
        BC2RGBAUnorm,
        BC2RGBAUnormSrgb,
        BC3RGBAUnorm,
        BC3RGBAUnormSrgb,
        BC4RUnorm,
        BC4RSnorm,
        BC5RGUnorm,
        BC5RGSnorm,
        BC6HRGBUfloat,
        BC6HRGBFloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        // Etc2/Eac compressed formats
        ETC2RGB8Unorm,
        ETC2RGB8UnormSrgb,
        ETC2RGB8A1Unorm,
        ETC2RGB8A1UnormSrgb,
        ETC2RGBA8Unorm,
        ETC2RGBA8UnormSrgb,
        EACR11Unorm,
        EACR11Snorm,
        EACRG11Unorm,
        EACRG11Snorm,
        // Astc compressed formats
        ASTC4x4Unorm,
        ASTC4x4UnormSrgb,
        ASTC5x4Unorm,
        ASTC5x4UnormSrgb,
        ASTC5x5Unorm,
        ASTC5x5UnormSrgb,
        ASTC6x5Unorm,
        ASTC6x5UnormSrgb,
        ASTC6x6Unorm,
        ASTC6x6UnormSrgb,
        ASTC8x5Unorm,
        ASTC8x5UnormSrgb,
        ASTC8x6Unorm,
        ASTC8x6UnormSrgb,
        ASTC8x8Unorm,
        ASTC8x8UnormSrgb,
        ASTC10x5Unorm,
        ASTC10x5UnormSrgb,
        ASTC10x6Unorm,
        ASTC10x6UnormSrgb,
        ASTC10x8Unorm,
        ASTC10x8UnormSrgb,
        ASTC10x10Unorm,
        ASTC10x10UnormSrgb,
        ASTC12x10Unorm,
        ASTC12x10UnormSrgb,
        ASTC12x12Unorm,
        ASTC12x12UnormSrgb,
    }

    [LibraryImport(LibName)]
    public static partial nint alimerImageCreateFromMemory(void* data, nuint size);

    [LibraryImport(LibName)]
    public static partial void alimerImageDestroy(nint image);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    public static extern TextureDimension alimerImageGetDimension(nint image);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    public static extern ImageFormat alimerImageGetFormat(nint image);

    [LibraryImport(LibName)]
    public static partial uint alimerImageGetWidth(nint image, uint level);

    [LibraryImport(LibName)]
    public static partial uint alimerImageGetHeight(nint image, uint level);

    [LibraryImport(LibName)]
    public static partial uint alimerImageGetDepth(nint image, uint level);

    [LibraryImport(LibName)]
    public static partial uint alimerImageGetLayerCount(nint image);

    [LibraryImport(LibName)]
    public static partial uint alimerImageGetLevelCount(nint image);

    [LibraryImport(LibName)]
    public static partial int alimerImageIsArray(nint image);

    [LibraryImport(LibName)]
    public static partial int alimerImageIsCubemap(nint image);

    [LibraryImport(LibName)]
    public static partial uint alimerImageGetDataSize(nint image);

    [LibraryImport(LibName)]
    public static partial void* alimerImageGetData(nint image);

#if !WINDOWS_UWP
    [LibraryImport(LibName)]
    public static partial int alimerImageSavePngMemory(nint image, delegate* unmanaged<void*, uint, void> callback);
#endif
}
