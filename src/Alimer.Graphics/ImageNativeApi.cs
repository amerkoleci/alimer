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
        R8,
        RG8,
        RGBA8,
        R16,
        RG16,
        RGBA16,
        R16F,
        RG16F,
        RGBA16F,
        R32F,
        RG32F,
        RGBA32F,
    }

    [LibraryImport(LibName)]
    public static partial nint alimerImageFromMemory(void* data, nuint size);

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
    public static partial int image_save_png_memory(nint image, delegate* unmanaged<void*, uint, void> callback);
#endif
}

#if TODO
/// <summary>
/// A dispatchable handle.
/// </summary>
[DebuggerDisplay("{DebuggerDisplay,nq}")]
public readonly unsafe partial struct KtxTexture : IEquatable<KtxTexture>
{
    public KtxTexture(nint handle) { Handle = handle; }
    public nint Handle { get; }
    public bool IsNull => Handle == 0;
    public static KtxTexture Null => new KtxTexture(0);

    public uint BaseWidth => NativeApi.ktx_get_baseWidth(Handle);
    public uint BaseHeight => NativeApi.ktx_get_baseHeight(Handle);
    public uint NumDimensions => NativeApi.ktx_get_numDimensions(Handle);
    public uint NumLevels => NativeApi.ktx_get_numLevels(Handle);
    public uint NumLayers => NativeApi.ktx_get_numLayers(Handle);
    public uint NumFaces => NativeApi.ktx_get_numFaces(Handle);
    public bool IsArray => NativeApi.ktx_get_isArray(Handle) == 1;

    public static implicit operator KtxTexture(nint handle) => new KtxTexture(handle);
    public static bool operator ==(KtxTexture left, KtxTexture right) => left.Handle == right.Handle;
    public static bool operator !=(KtxTexture left, KtxTexture right) => left.Handle != right.Handle;
    public static bool operator ==(KtxTexture left, nint right) => left.Handle == right;
    public static bool operator !=(KtxTexture left, nint right) => left.Handle != right;
    public bool Equals(KtxTexture other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is KtxTexture handle && Equals(handle);
    /// <inheritdoc/>
    public override int GetHashCode() => Handle.GetHashCode();
    private string DebuggerDisplay => string.Format("KtxTexture [0x{0}]", Handle.ToString("X"));
}

internal enum KtxErrorCode
{
    /// <summary>
    /// Operation was successful.
    /// </summary>
    Success = 0,
    KTX_FILE_DATA_ERROR,     /*!< The data in the file is inconsistent with the spec. */
    KTX_FILE_ISPIPE,         /*!< The file is a pipe or named pipe. */
    KTX_FILE_OPEN_FAILED,    /*!< The target file could not be opened. */
    KTX_FILE_OVERFLOW,       /*!< The operation would exceed the max file size. */
    KTX_FILE_READ_ERROR,     /*!< An error occurred while reading from the file. */
    KTX_FILE_SEEK_ERROR,     /*!< An error occurred while seeking in the file. */
    KTX_FILE_UNEXPECTED_EOF, /*!< File does not have enough data to satisfy request. */
    KTX_FILE_WRITE_ERROR,    /*!< An error occurred while writing to the file. */
    KTX_GL_ERROR,            /*!< GL operations resulted in an error. */
    KTX_INVALID_OPERATION,   /*!< The operation is not allowed in the current state. */
    KTX_INVALID_VALUE,       /*!< A parameter value was not valid */
    KTX_NOT_FOUND,           /*!< Requested key was not found */
    KTX_OUT_OF_MEMORY,       /*!< Not enough memory to complete the operation. */
    KTX_TRANSCODE_FAILED,    /*!< Transcoding of block compressed texture failed. */
    KTX_UNKNOWN_FILE_FORMAT, /*!< The file not a KTX file */
    KTX_UNSUPPORTED_TEXTURE_TYPE, /*!< The KTX file specifies an unsupported texture type. */
    KTX_UNSUPPORTED_FEATURE,  /*!< Feature not included in in-use library or not yet implemented. */
    KTX_LIBRARY_NOT_LINKED,  /*!< Library dependency (OpenGL or Vulkan) not linked into application. */
    KTX_ERROR_MAX_ENUM = KTX_LIBRARY_NOT_LINKED /*!< For safety checks. */
}

#endif
