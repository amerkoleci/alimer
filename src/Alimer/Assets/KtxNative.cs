// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Assets;
using Alimer.Graphics;
using ktx_bool_t = Alimer.NativeBool;

#pragma warning disable CS0649

internal unsafe static partial class KtxNative
{
    public const string LibraryName = "ktx";


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
        public delegate* unmanaged<ktxTexture*, uint, nuint> GetLevelSize;
        public nint IterateLevels;
        public nint IterateLoadLevelFaces;
        public delegate* unmanaged<ktxTexture*, ktx_bool_t> NeedsTranscoding;
        public delegate* unmanaged<ktxTexture*, ktx_bool_t> IsTranscodable;
        public delegate* unmanaged<ktxTexture*, ktx_bool_t> IsHDR;
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
        KTX_TEXTURE_CREATE_NO_FLAGS = 0x00,
        /// <summary>
        /// Load the images from the KTX source.
        /// </summary>
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT = 0x01,
        /// <summary>
        /// Load the raw key-value data instead of creating a @c ktxHashList from it.
        /// </summary>
        KTX_TEXTURE_CREATE_RAW_KVDATA_BIT = 0x02,
        /// <summary>
        /// Skip any key-value data. This overrides the RAW_KVDATA_BIT.
        /// </summary>
        KTX_TEXTURE_CREATE_SKIP_KVDATA_BIT = 0x04,
        /// <summary>
        /// Load texture compatible with the rules of KHR_texture_basisu glTF extension
        /// </summary>
        KTX_TEXTURE_CREATE_CHECK_GLTF_BASISU_BIT = 0x08
    }

    public enum ktxSupercmpScheme
    {
        KTX_SS_NONE = 0,            /*!< No supercompression. */
        KTX_SS_BASIS_LZ = 1,        /*!< Basis LZ supercompression. */
        KTX_SS_ZSTD = 2,            /*!< ZStd supercompression. */
        KTX_SS_ZLIB = 3,            /*!< ZLIB supercompression. */
        KTX_SS_UASTC_HDR_6x6_INTERMEDIATE = 4,            /*!< UASTC HDR 6x6 Intermediate supercompression. */
        KTX_SS_BEGIN_RANGE = KTX_SS_NONE,
        KTX_SS_END_RANGE = KTX_SS_UASTC_HDR_6x6_INTERMEDIATE,
        KTX_SS_BEGIN_VENDOR_RANGE = 0x10000,
        KTX_SS_END_VENDOR_RANGE = 0x1ffff,
        KTX_SS_BEGIN_RESERVED = 0x20000
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
        KTX_TTF_ASTC_LDR_4x4_RGBA = 10,

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

        KTX_TTF_RGB_HALF = 24,
        /*!< 48bpp RGB half (16-bits/component, 3 components) */
        KTX_TTF_RGBA_HALF = 25,
        /*!< 64bpp RGBA half (16-bits/component, 4 components). A will
            always be  1.0 as UASTC_HDR does not support alpha. */
        KTX_TTF_RGB_9E5 = 26,
        /*!< 32bpp RGB 9E5 (shared exponent, positive only). */

        KTX_TTF_ASTC_HDR_4x4_RGBA = 29,
        /*!< HDR, RGBA unsigned. A will always be 1.0 as UASTC_HDR does
             not support alpha. */
        KTX_TTF_ASTC_HDR_6x6_RGBA = 30,
        /*!< HDR, RGBA unsigned. A will always be 1.0 as UASTC_HDR does
             not support alpha. */
        KTX_TTF_BC6HU_RGB = 31,
        /*!< HDR, RGB only, unsigned. */
        KTX_TTF_BC6HU = KTX_TTF_BC6HU_RGB,
        /*!< @deprecated Use #KTX_TTF_BC6HU_RGB. */

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
        /// <summary>
        /// PVRTC1: decode non-pow2 ETC1S texture level to the next larger power of 2 (not implemented yet). Ignored if the slice's dimensions are already a power of 2.
        /// </summary>
        KTX_TF_PVRTC_DECODE_TO_NEXT_POW2 = 2,
        /// <summary>
        /// When decoding to an opaque texture format, if the Basis data has alpha, decode the alpha slice instead of the color slice to the output texture format. Has no effect if there is no alpha data.
        /// </summary>
        KTX_TF_TRANSCODE_ALPHA_DATA_TO_OPAQUE_FORMATS = 4,
        /// <summary>
        /// Disable ETC1S to BC7 adaptive chroma filtering, for much faster transcoding to BC7.  This flag is unused by other ETC1S transcoders. 
        /// </summary>
        KTX_TF_NO_ETC1S_CHROMA_FILTERING = 64,
        /// <summary>
        /// Request higher quality transcode of UASTC to BC1, BC3, ETC2\_EAC\_R11 or ETC2\_EAC\_RG11. The flag is unused by other UASTC transcoders.
        /// </summary>
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
        public ktx_bool_t isArray;
        public ktx_bool_t isCubemap;
        public ktx_bool_t isCompressed;
        public ktx_bool_t generateMipmaps;
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
    public static partial KTX_error_code ktxTexture_CreateFromMemory(ReadOnlySpan<byte> bytes, nuint size, uint createFlags, ktxTexture** newTex);

    public static bool ktxTexture_NeedsTranscoding(ktxTexture* This)
    {
        return This->vtbl->NeedsTranscoding(This);
    }

    public static bool ktxTexture_IsTranscodable(ktxTexture* This)
    {
        return This->vtbl->IsTranscodable(This);
    }

    public static bool ktxTexture_IsHDR(ktxTexture* This)
    {
        return This->vtbl->IsHDR(This);
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

    public static nuint ktxTexture_GetLevelSize(ktxTexture* This, uint level)
    {
        return This->vtbl->GetLevelSize(This, level);
    }

    public static void ktxTexture_Destroy(ktxTexture* This)
    {
        This->vtbl->Destroy(This);
    }

    public enum ktx_rgba_astc_mapping_e
    {
        KTX_MAP_RGBA_ASTC_TO_HDR,
        /*!< map to @c VK_FORMAT_ASTC_\*_SFLOAT_BLOCK formats. */
        KTX_MAP_RGBA_ASTC_TO_LDR
        /*!< map to @c VK_FORMAT_ASTC_\*_UNORM_BLOCK formats*/
    }

    [LibraryImport(LibraryName)]
    public static partial ktx_rgba_astc_mapping_e ktxTexture1_GetRgbaAstcMapping(ktxTexture1* This);
}
