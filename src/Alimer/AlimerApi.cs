// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Assets;
using Alimer.Graphics;

#pragma warning disable CS0649

namespace Alimer;

internal unsafe static partial class AlimerApi
{
    public const string LibraryName = "alimer_native";

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

    [LibraryImport(LibraryName)]
    public static partial LogLevel alimerGetLogLevel();
    [LibraryImport(LibraryName)]
    public static partial void alimerSetLogLevel(LogLevel level);

    [LibraryImport(LibraryName)]
    public static partial void alimerSetLogCallback(delegate* unmanaged<LogCategory, LogLevel, byte*, nint, void> callback, nint userdata);

    #region Image
    public enum ImageType
    {
        Type2D = 0,
        Type1D,
        Type3D,
        TypeCube,
    }

    public struct ImageDesc
    {
        public ImageType type;
        public PixelFormat format;
        public uint width;
        public uint height;
        public uint depthOrArrayLayers;
        public uint mipLevelCount;
    }

    public struct ImageLevel
    {
        public uint width;
        public uint height;
        public PixelFormat format;
        public uint rowPitch;
        public uint slicePitch;
        public byte* pixels;
    }

    public enum class_id
    {
        ktxTexture1_c = 1,
        ktxTexture2_c = 2,
    }

    public enum ktxOrientationX
    {
        KTX_ORIENT_X_LEFT = 108,
        KTX_ORIENT_X_RIGHT = 114,
    }

    public enum ktxOrientationY
    {
        KTX_ORIENT_Y_UP = 117,
        KTX_ORIENT_Y_DOWN = 100,
    }

    public enum ktxOrientationZ
    {
        KTX_ORIENT_Z_IN = 105,
        KTX_ORIENT_Z_OUT = 111,
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ktxTexture_vtbl
    {
        public delegate* unmanaged<ktxTexture*, void> Destroy;
        public delegate* unmanaged<ktxTexture*, uint, uint, uint, nuint*, KTX_error_code> GetImageOffset;
        public delegate* unmanaged<ktxTexture*, nuint> GetDataSizeUncompressed;
        public delegate* unmanaged<ktxTexture*, uint, nuint> GetImageSize;
        public IntPtr GetLevelSize;
        public IntPtr IterateLevels;
        public IntPtr IterateLoadLevelFaces;
        public delegate* unmanaged<ktxTexture*, Bool8> NeedsTranscoding;
        public IntPtr LoadImageData;
        public IntPtr SetImageFromMemory;
        public IntPtr SetImageFromStdioStream;
        public IntPtr WriteToStdioStream;
        public IntPtr WriteToNamedFile;
        public IntPtr WriteToMemory;
        public IntPtr WriteToStream;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ktxOrientation
    {

        /// <summary>
        /// Orientation in X 
        /// </summary>
        public ktxOrientationX x;

        /// <summary>
        /// Orientation in Y 
        /// </summary>
        public ktxOrientationY y;

        /// <summary>
        /// Orientation in Z 
        /// </summary>
        public ktxOrientationZ z;
    }

    public enum KTX_error_code
    {

        /// <summary>
        /// Operation was successful. 
        /// </summary>
        KTX_SUCCESS = 0,

        /// <summary>
        /// The data in the file is inconsistent with the spec. 
        /// </summary>
        KTX_FILE_DATA_ERROR = 1,

        /// <summary>
        /// The file is a pipe or named pipe. 
        /// </summary>
        KTX_FILE_ISPIPE = 2,

        /// <summary>
        /// The target file could not be opened. 
        /// </summary>
        KTX_FILE_OPEN_FAILED = 3,

        /// <summary>
        /// The operation would exceed the max file size. 
        /// </summary>
        KTX_FILE_OVERFLOW = 4,

        /// <summary>
        /// An error occurred while reading from the file. 
        /// </summary>
        KTX_FILE_READ_ERROR = 5,

        /// <summary>
        /// An error occurred while seeking in the file. 
        /// </summary>
        KTX_FILE_SEEK_ERROR = 6,

        /// <summary>
        /// File does not have enough data to satisfy request. 
        /// </summary>
        KTX_FILE_UNEXPECTED_EOF = 7,

        /// <summary>
        /// An error occurred while writing to the file. 
        /// </summary>
        KTX_FILE_WRITE_ERROR = 8,

        /// <summary>
        /// GL operations resulted in an error. 
        /// </summary>
        KTX_GL_ERROR = 9,

        /// <summary>
        /// The operation is not allowed in the current state. 
        /// </summary>
        KTX_INVALID_OPERATION = 10,

        /// <summary>
        /// A parameter value was not valid. 
        /// </summary>
        KTX_INVALID_VALUE = 11,

        /// <summary>
        /// Requested metadata key or required dynamically loaded GPU function was not found. 
        /// </summary>
        KTX_NOT_FOUND = 12,

        /// <summary>
        /// Not enough memory to complete the operation. 
        /// </summary>
        KTX_OUT_OF_MEMORY = 13,

        /// <summary>
        /// Transcoding of block compressed texture failed. 
        /// </summary>
        KTX_TRANSCODE_FAILED = 14,

        /// <summary>
        /// The file not a KTX file 
        /// </summary>
        KTX_UNKNOWN_FILE_FORMAT = 15,

        /// <summary>
        /// The KTX file specifies an unsupported texture type. 
        /// </summary>
        KTX_UNSUPPORTED_TEXTURE_TYPE = 16,

        /// <summary>
        /// Feature not included in in-use library or not yet implemented. 
        /// </summary>
        KTX_UNSUPPORTED_FEATURE = 17,

        /// <summary>
        /// Library dependency (OpenGL or Vulkan) not linked into application. 
        /// </summary>
        KTX_LIBRARY_NOT_LINKED = 18,

        /// <summary>
        /// Decompressed byte count does not match expected byte size 
        /// </summary>
        KTX_DECOMPRESS_LENGTH_ERROR = 19,

        /// <summary>
        /// Checksum mismatch when decompressing 
        /// </summary>
        KTX_DECOMPRESS_CHECKSUM_ERROR = 20,

        /// <summary>
        /// For safety checks. 
        /// </summary>
        KTX_ERROR_MAX_ENUM = 20,
    }

    [Flags]
    public enum ktxTextureCreateFlags
    {
        KTX_TEXTURE_CREATE_NO_FLAGS = 0,
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT = 1,
        KTX_TEXTURE_CREATE_RAW_KVDATA_BIT = 2,
        KTX_TEXTURE_CREATE_SKIP_KVDATA_BIT = 4,
        KTX_TEXTURE_CREATE_CHECK_GLTF_BASISU_BIT = 8,
    }

    public enum ktxSupercmpScheme
    {

        /// <summary>
        /// No supercompression. 
        /// </summary>
        KTX_SS_NONE = 0,

        /// <summary>
        /// Basis LZ supercompression. 
        /// </summary>
        KTX_SS_BASIS_LZ = 1,

        /// <summary>
        /// ZStd supercompression. 
        /// </summary>
        KTX_SS_ZSTD = 2,

        /// <summary>
        /// ZLIB supercompression. 
        /// </summary>
        KTX_SS_ZLIB = 3,
        KTX_SS_BEGIN_RANGE = 0,
        KTX_SS_END_RANGE = 3,
        KTX_SS_BEGIN_VENDOR_RANGE = 65536,
        KTX_SS_END_VENDOR_RANGE = 131071,
        KTX_SS_BEGIN_RESERVED = 131072,
    }

    public enum ktx_transcode_fmt
    {
        /// <summary>
        /// ETC1-2
        /// </summary>
        KTX_TTF_ETC1_RGB = 0,
        KTX_TTF_ETC2_RGBA = 1,

        /// <summary>
        /// BC1-5, BC7 (desktop, some mobile devices)
        /// </summary>
        KTX_TTF_BC1_RGB = 2,
        KTX_TTF_BC3_RGBA = 3,
        KTX_TTF_BC4_R = 4,
        KTX_TTF_BC5_RG = 5,
        KTX_TTF_BC7_RGBA = 6,

        /// <summary>
        /// PVRTC1 4bpp (mobile, PowerVR devices)
        /// </summary>
        KTX_TTF_PVRTC1_4_RGB = 8,
        KTX_TTF_PVRTC1_4_RGBA = 9,

        /// <summary>
        /// ASTC (mobile, Intel devices, hopefully all desktop GPU's one day)
        /// </summary>
        KTX_TTF_ASTC_4x4_RGBA = 10,

        /// <summary>
        /// ATC and FXT1 formats are not supported by KTX2 as there
        /// are no equivalent VkFormats.
        /// </summary>
        KTX_TTF_PVRTC2_4_RGB = 18,
        KTX_TTF_PVRTC2_4_RGBA = 19,
        KTX_TTF_ETC2_EAC_R11 = 20,
        KTX_TTF_ETC2_EAC_RG11 = 21,

        /// <summary>
        /// Uncompressed (raw pixel) formats
        /// </summary>
        KTX_TTF_RGBA32 = 13,
        KTX_TTF_RGB565 = 14,
        KTX_TTF_BGR565 = 15,
        KTX_TTF_RGBA4444 = 16,

        /// <summary>
        /// Values for automatic selection of RGB or RGBA depending if alpha
        /// present.
        /// </summary>
        KTX_TTF_ETC = 22,
        KTX_TTF_BC1_OR_3 = 23,
        KTX_TTF_NOSELECTION = 2147483647,

        /// <summary>
        /// Old enums for compatibility with code compiled against previous
        /// versions of libktx.
        /// </summary>
        KTX_TF_ETC1 = 0,
        KTX_TF_ETC2 = 22,
        KTX_TF_BC1 = 2,
        KTX_TF_BC3 = 3,
        KTX_TF_BC4 = 4,
        KTX_TF_BC5 = 5,
        KTX_TTF_BC7_M6_RGB = 6,
        KTX_TTF_BC7_M5_RGBA = 6,
        KTX_TF_BC7_M6_OPAQUE_ONLY = 6,
        KTX_TF_PVRTC1_4_OPAQUE_ONLY = 8,
    }

    [Flags]
    public enum ktx_transcode_flag_bits
    {
        KTX_TF_PVRTC_DECODE_TO_NEXT_POW2 = 2,
        KTX_TF_TRANSCODE_ALPHA_DATA_TO_OPAQUE_FORMATS = 4,
        KTX_TF_HIGH_QUALITY = 32,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ktxHashListEntry { IntPtr ptr; }
    [StructLayout(LayoutKind.Sequential)]
    public struct ktxHashList { ktxHashListEntry entry; }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ktxTexture
    {
        public class_id classId;
        public ktxTexture_vtbl* vtbl;
        public IntPtr vvtbl;
        public IntPtr _protected;
        public Bool8 isArray;
        public Bool8 isCubemap;
        public Bool8 isCompressed;
        public Bool8 generateMipmaps;
        public UInt32 baseWidth;
        public UInt32 baseHeight;
        public UInt32 baseDepth;
        public UInt32 numDimensions;
        public UInt32 numLevels;
        public UInt32 numLayers;
        public UInt32 numFaces;
        public ktxOrientation orientation;
        public ktxHashList kvDataHead;
        public UInt32 kvDataLen;
        public byte* kvData;
        public UIntPtr dataSize;
        public byte* pData;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ktxTexture1
    {
        public class_id classId;
        public ktxTexture_vtbl* vtbl;
        public IntPtr vvtbl;
        public IntPtr _protected;
        public byte isArray;
        public byte isCubemap;
        public byte isCompressed;
        public byte generateMipmaps;
        public UInt32 baseWidth;
        public UInt32 baseHeight;
        public UInt32 baseDepth;
        public UInt32 numDimensions;
        public UInt32 numLevels;
        public UInt32 numLayers;
        public UInt32 numFaces;
        public ktxOrientation orientation;
        public ktxHashList kvDataHead;
        public UInt32 kvDataLen;
        public byte* kvData;
        public UIntPtr dataSize;
        public byte* pData;

        /// <summary>
        /// Format of the texture data, e.g., GL_RGB. 
        /// </summary>
        public UInt32 glFormat;

        /// <summary>
        /// Internal format of the texture data,
        /// e.g., GL_RGB8. 
        /// </summary>
        public UInt32 glInternalformat;

        /// <summary>
        /// Base format of the texture data,
        /// e.g., GL_RGB. 
        /// </summary>
        public UInt32 glBaseInternalformat;

        /// <summary>
        /// Type of the texture data, e.g, GL_UNSIGNED_BYTE.
        /// </summary>
        public UInt32 glType;

        /// <summary>
        /// Private data. 
        /// </summary>
        public IntPtr _private;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ktxTexture2
    {
        public class_id classId;
        public ktxTexture_vtbl* vtbl;
        public IntPtr vvtbl;
        public IntPtr _protected;
        public byte isArray;
        public byte isCubemap;
        public byte isCompressed;
        public byte generateMipmaps;
        public UInt32 baseWidth;
        public UInt32 baseHeight;
        public UInt32 baseDepth;
        public UInt32 numDimensions;
        public UInt32 numLevels;
        public UInt32 numLayers;
        public UInt32 numFaces;
        public ktxOrientation orientation;
        public ktxHashList kvDataHead;
        public UInt32 kvDataLen;
        public byte* kvData;
        public UIntPtr dataSize;
        public byte* pData;
        public UInt32 vkFormat;
        public UInt32* pDfd;
        public ktxSupercmpScheme supercompressionScheme;
        public byte isVideo;
        public UInt32 duration;
        public UInt32 timescale;
        public UInt32 loopcount;

        /// <summary>
        /// Private data. 
        /// </summary>
        public IntPtr _private;
    }

    [LibraryImport(LibraryName)]
    public static partial ImageFileType alimerImageDetectFileType(void* data, nuint size);

    [LibraryImport(LibraryName)]
    public static partial nint alimerImageCreate2D(PixelFormat format, uint width, uint height, uint arrayLayers, uint mipLevelCount);

    [LibraryImport(LibraryName)]
    public static partial nint alimerImageCreateFromMemory(void* data, nuint size);

    [LibraryImport(LibraryName)]
    public static partial void alimerImageDestroy(nint handle);

    [LibraryImport(LibraryName)]
    public static partial void alimerImageGetDesc(nint handle, ImageDesc* desc);

    [LibraryImport(LibraryName)]
    public static partial byte* alimerImageGetPixels(nint handle, nuint* dataSize);

    [LibraryImport(LibraryName)]
    public static partial ImageLevel* alimerImageGetLevel(nint handle, uint mipLevel, uint arrayOrDepthSlice);

    [LibraryImport(LibraryName)]
    public static partial KTX_error_code ktxTexture_CreateFromMemory(byte* bytes, nuint size, uint createFlags, ktxTexture** newTex);

    public static bool ktxTexture_NeedsTranscoding(ktxTexture* This)
    {
        return This->vtbl->NeedsTranscoding(This);
    }

    [LibraryImport(LibraryName)]
    public static partial KTX_error_code ktxTexture2_TranscodeBasis(ktxTexture2* This, ktx_transcode_fmt fmt, uint transcodeFlags);

    public static KTX_error_code ktxTexture_GetImageOffset(ktxTexture* This, uint level, uint layer, uint faceSlice, nuint* pOffset)
    {
        return This->vtbl->GetImageOffset(This, level, layer, faceSlice, pOffset);
    }

    public static nuint ktxTexture_GetImageSize(ktxTexture* This, uint level)
    {
        return This->vtbl->GetImageSize(This, level);
    }

    public static void ktxTexture_Destroy(ktxTexture* This)
    {
        This->vtbl->Destroy(This);
    }

    [LibraryImport(LibraryName)]
    public static partial uint alimerVkFormatFromOpenGLInternalFormat(uint glInternalformat);
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
