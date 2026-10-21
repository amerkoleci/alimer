// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers.Binary;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using Alimer.Serialization;
using Alimer.Utilities;
// FIX VkFormat usage when building without vulkan backend
using Vortice.Vulkan;
using static Alimer.AlimerApi;
using static KtxNative;
using static KtxNative.KTX_error_code;
using static KtxNative.ktx_rgba_astc_mapping_e;
using static Vortice.Vulkan.Vulkan;
using GLenum = System.UInt32;

namespace Alimer.Assets;

public sealed unsafe class Image : Asset, IBinarySerializable<Image>
{
    private readonly MipMapDescription[] _mipmaps;
    private readonly ImageData[] _levels;
    private readonly byte* _data;

    public Image(in ImageDescription description)
    {
        ArgumentOutOfRangeException.ThrowIfEqual(description.Format, PixelFormat.Undefined);
        ArgumentOutOfRangeException.ThrowIfLessThan(description.Width, 1u);
        ArgumentOutOfRangeException.ThrowIfLessThan(description.Height, 1u);
        ArgumentOutOfRangeException.ThrowIfLessThan(description.DepthOrArrayLayers, 1u);

        Description = description;
        MipLevelCount = description.MipLevelCount == 0 ? ImageDescription.GetMipLevelCount(Width, Height, Dimension == TextureDimension.Texture3D ? Depth : 1u) : description.MipLevelCount;
        _mipmaps = new MipMapDescription[MipLevelCount];

        uint levelsCount = 0;
        switch (Description.Dimension)
        {
            case TextureDimension.Texture1D:
            case TextureDimension.Texture2D:
            case TextureDimension.TextureCube:
                for (uint arrayIndex = 0; arrayIndex < ActualArrayLayers; ++arrayIndex)
                {
                    uint mipWidth = Width;
                    uint mipHeight = Height;

                    for (uint level = 0; level < MipLevelCount; ++level)
                    {
                        PixelFormatUtils.GetSurfaceInfo(Format, mipWidth, mipHeight, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);

                        _mipmaps[level] = new MipMapDescription(
                           mipWidth,
                           mipHeight,
                           1,
                           rowPitch,
                           slicePitch,
                           widthCount,
                           heightCount
                           );

                        SizeInBytes += slicePitch;
                        levelsCount++;

                        if (mipHeight > 1)
                            mipHeight >>= 1;

                        if (mipWidth > 1)
                            mipWidth >>= 1;
                    }
                }
                break;

            case TextureDimension.Texture3D:
            {
                uint mipWidth = Width;
                uint mipHeight = Height;
                uint mipDepth = Depth;

                for (uint level = 0; level < MipLevelCount; ++level)
                {
                    PixelFormatUtils.GetSurfaceInfo(Format, mipWidth, mipHeight, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);

                    _mipmaps[level] = new MipMapDescription(
                       mipWidth,
                       mipHeight,
                       mipDepth,
                       rowPitch,
                       slicePitch,
                       widthCount,
                       heightCount
                       );

                    for (uint slice = 0; slice < mipDepth; ++slice)
                    {
                        SizeInBytes += slicePitch;
                        levelsCount++;
                    }

                    if (mipWidth > 1)
                        mipWidth >>= 1;

                    if (mipHeight > 1)
                        mipHeight >>= 1;

                    if (mipDepth > 1)
                        mipDepth >>= 1;
                }
            }
            break;

            default:
                throw new InvalidOperationException("Invalid Image dimension");
        }

        _data = (byte*)NativeMemory.AllocZeroed(SizeInBytes);
        _levels = new ImageData[levelsCount];
        SetupImageArray(_data, SizeInBytes, description, _levels);
    }

    private Image(in ImageDescription description, Span<byte> source)
        : this(in description)
    {
        source.CopyTo(new Span<byte>(_data, source.Length));
    }

    private Image(in ImageDescription description, ReadOnlySpan<byte> source)
        : this(in description)
    {
        source.CopyTo(new Span<byte>(_data, source.Length));
    }

    /// <summary>
    /// Gets the image description.
    /// </summary>
    public ImageDescription Description { get; }

    /// <summary>
    /// Gets the image dimension.
    /// </summary>
    public TextureDimension Dimension => Description.Dimension;

    /// <summary>
    /// Gets the data format.
    /// </summary>
    public PixelFormat Format => Description.Format;

    /// <summary>
    /// Get the width of the image.
    /// </summary>
    public uint Width => Description.Width;

    /// <summary>
    /// Get the height of the image.
    /// </summary>
    public uint Height => Description.Height;

    /// <summary>
    /// Get the depth of the image.
    /// </summary>
    public uint Depth => Description.Dimension == TextureDimension.Texture3D ? Description.DepthOrArrayLayers : 1;

    /// <summary>
    /// Get the array layers of the image.
    /// </summary>
    public uint ArrayLayers => Description.Dimension != TextureDimension.Texture3D ? Description.DepthOrArrayLayers : 1;

    /// <summary>
    /// Actual number of array layers (taking care of cubemap textures)
    /// </summary>
    internal uint ActualArrayLayers => (Description.Dimension == TextureDimension.TextureCube) ? 6 * ArrayLayers : ArrayLayers;

    /// <summary>
    /// Get the number of mipmap levels in the.
    /// </summary>
    public uint MipLevelCount { get; }

    /// <summary>
    /// Get the data size in bytes.
    /// </summary>
    public uint SizeInBytes { get; }

    /// <summary>
    /// Get the image data.
    /// </summary>
    public ReadOnlySpan<byte> Data => new(_data, (int)SizeInBytes);

    /// <summary>
    /// Gets a pointer to the image buffer in memory.
    /// </summary>
    public nint DataPointer => (nint)_data;

    /// <summary>
    /// Get the image data.
    /// </summary>
    public ReadOnlySpan<T> GetData<T>()
        where T : unmanaged
    {
        return MemoryMarshal.Cast<byte, T>(Data);
    }

    /// <summary>
    /// Get a mip-level width.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetWidth(uint mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Width >> (int)mipLevel) : 0;
    }

    /// <summary>
    /// Get a mip-level height.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetHeight(uint mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Height >> (int)mipLevel) : 0;
    }

    /// <summary>
    /// Get a mip-level depth.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetDepth(uint mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Depth >> (int)mipLevel) : 0;
    }

    /// <summary>
    /// Gets the mipmap description of this instance for the specified mipmap level.
    /// </summary>
    /// <param name="level">The mipmap level to get information.</param>
    /// <returns>A description of a particular mipmap for this texture.</returns>
    public MipMapDescription GetMipMapDescription(uint level)
    {
        Debug.Assert(level >= 0);
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(level, MipLevelCount);

        return _mipmaps[level];
    }

    public ImageData GetLevel(uint mipLevel, uint arrayOrDepthSlice = 0u)
    {
        if (mipLevel >= Description.MipLevelCount)
            return default;

        uint index = 0;

        switch (Description.Dimension)
        {
            case TextureDimension.Texture1D:
            case TextureDimension.Texture2D:
            case TextureDimension.TextureCube:
            {
                if (arrayOrDepthSlice >= ActualArrayLayers)
                    return default;

                index = arrayOrDepthSlice * MipLevelCount + mipLevel;
                break;
            }

            case TextureDimension.Texture3D:
            {
                uint mipDepth = Description.DepthOrArrayLayers;

                for (uint level = 0; level < mipLevel; ++level)
                {
                    index += mipDepth;
                    if (mipDepth > 1)
                        mipDepth >>= 1;
                }

                if (arrayOrDepthSlice >= mipDepth)
                    return default;

                index += arrayOrDepthSlice;
                break;
            }

            default:
                return default;
        }

        return _levels[(int)index];
    }

    public static ImageFileType DetectFileType(ReadOnlySpan<byte> data)
    {
        if (data.Length < 2)
            return ImageFileType.Unknown;


        if (data.Length >= 8 && data[..8].SequenceEqual(new byte[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A }))
            return ImageFileType.Png;

        if (data.Length >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF)
            return ImageFileType.Jpeg;

        if (data.Length >= 6 && (data[..6].SequenceEqual("GIF87a"u8) || data[..6].SequenceEqual("GIF89a"u8)))
            return ImageFileType.Gif;

        if (data.Length >= 12 && data[..4].SequenceEqual("RIFF"u8) && data[8..12].SequenceEqual("WEBP"u8))
            return ImageFileType.Webp;

        if (data.Length >= 4 && data[..4].SequenceEqual("8BPS"u8))
            return ImageFileType.Psd;

        if (data.Length >= 4 && data[..4].SequenceEqual("DDS "u8))
            return ImageFileType.Dds;

        if (data.Length >= 8 && data[0] == 0x76 && data[1] == 0x2F && data[2] == 0x31 && data[3] == 0x01)
            return ImageFileType.Exr;

        if (IsRadianceHdr(data))
            return ImageFileType.Hdr;

        if (IsTga(data))
            return ImageFileType.Tga;

        if (IsIco(data))
            return ImageFileType.Ico;

        if (IsPnm(data))
            return ImageFileType.Pnm;

        if (IsBmp(data))
            return ImageFileType.Bmp;

        if (IsKTX1(data))
            return ImageFileType.Ktx1;

        if (IsKTX2(data))
            return ImageFileType.Ktx2;

        return ImageFileType.Unknown;
    }

    public static Image FromFile(string filePath, int channels = 4, bool srgb = true)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(stream, channels, srgb);
    }

    public static Image FromStream(Stream stream, int channels = 4, bool srgb = true)
    {
        Span<byte> data = stream.Length < 2048 ? stackalloc byte[(int)stream.Length] : new byte[(int)stream.Length];
        stream.ReadExactly(data);
        return FromMemory(data, channels, srgb);
    }

    public static Image FromMemory(ReadOnlySpan<byte> data, int channels = 4, bool srgb = true)
    {
        // TODO: Add DDS, ASTC, KTX1 and KTX2 loading
        ImageFileType fileType = DetectFileType(data);

        Image result;

        if (fileType == ImageFileType.Ktx1 || fileType == ImageFileType.Ktx2)
        {
            ktxTexture* ktxTexture;
            KTX_error_code ktx_result = ktxTexture_CreateFromMemory(data, (uint)data.Length,
                (uint)ktxTextureCreateFlags.KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                &ktxTexture);

            // Not ktx texture.
            if (ktx_result != KTX_SUCCESS)
            {
                //return false;
            }

            // We detect format from VkFormat
            PixelFormat format = PixelFormat.Undefined;
            bool needsTranscoding = ktxTexture_NeedsTranscoding(ktxTexture);
            bool isTranscodable = ktxTexture_IsTranscodable(ktxTexture);
            bool isHDR = ktxTexture_IsHDR(ktxTexture);
            if (ktxTexture->classId == class_id.ktxTexture2_c)
            {
                ktxTexture2* ktx_texture2 = (ktxTexture2*)ktxTexture;
                if (needsTranscoding)
                {
                    // Once transcoded, the ktxTexture object contains the texture data in a native GPU format (e.g. BC7)
                    // Handle other formats (textureCompressionBC) See: https://raw.githubusercontent.com/KhronosGroup/Vulkan-Samples/main/samples/performance/texture_compression_basisu/texture_compression_basisu.cpp
                    ktx_result = ktxTexture2_TranscodeBasis(ktx_texture2, ktx_transcode_fmt.KTX_TTF_BC7_RGBA, 0);
                }

                format = Alimer.Graphics.Vulkan.VulkanUtils.FromVkFormat((Vortice.Vulkan.VkFormat)ktx_texture2->vkFormat);
            }
            else
            {
                // KTX1
                ktxTexture1* ktx_texture1 = (ktxTexture1*)ktxTexture;

                VkFormat vkFormat = vkGetFormatFromOpenGLInternalFormat(ktx_texture1->glInternalformat, ktxTexture1_GetRgbaAstcMapping(ktx_texture1));
                format = Alimer.Graphics.Vulkan.VulkanUtils.FromVkFormat(vkFormat);

                // KTX-1 files don't contain color space information. Color data is normally
                // in sRGB, but the format we get back won't report that, so this will adjust it
                // if necessary.
                format = format.LinearToSrgbFormat();
            }

            ImageDescription imageDescription = default;
            if (ktxTexture->baseDepth > 1)
            {
                imageDescription = ImageDescription.Image3D(
                    format,
                    ktxTexture->baseWidth,
                    ktxTexture->baseHeight,
                    ktxTexture->baseDepth,
                    ktxTexture->numLevels
                );
            }
            else if (ktxTexture->isCubemap && !ktxTexture->isArray)
            {
                imageDescription = ImageDescription.ImageCube(
                    format,
                    ktxTexture->baseWidth,
                    ktxTexture->numLevels,
                    ktxTexture->numFaces / 6u
                );
            }
            else
            {
                imageDescription = ImageDescription.Image2D(
                    format,
                    ktxTexture->baseWidth,
                    ktxTexture->baseHeight,
                    ktxTexture->numLevels,
                    ktxTexture->isArray ? ktxTexture->numLayers : 1u
                );
            }

            result = new(in imageDescription);

            // If the texture contains more than one layer, then populate the offsets otherwise take the mipmap level offsets
            if (ktxTexture->isCubemap || ktxTexture->isArray)
            {
                uint layerCount = ktxTexture->isCubemap ? ktxTexture->numFaces : ktxTexture->numLayers;

                for (uint layer = 0; layer < layerCount; layer++)
                {
                    for (uint miplevel = 0; miplevel < ktxTexture->numLevels; miplevel++)
                    {
                        nuint offset;
                        if (ktxTexture->isCubemap)
                        {
                            ktx_result = ktxTexture_GetImageOffset(ktxTexture, miplevel, 0, layer, &offset);
                        }
                        else
                        {
                            ktx_result = ktxTexture_GetImageOffset(ktxTexture, miplevel, layer, 0, &offset);
                        }

                        if (ktx_result != KTX_SUCCESS)
                        {
                            //LOGF("Error loading KTX texture");
                        }

                        nuint levelSize = ktxTexture_GetImageSize(ktxTexture, miplevel);

                        ImageData imageData = result.GetLevel(miplevel, layer);
                        Debug.Assert(imageData.SlicePitch == levelSize);
                        NativeMemory.Copy(ktxTexture->pData + offset, imageData.DataPointer.ToPointer(), levelSize);
                    }
                }
            }
            else
            {
                for (uint miplevel = 0; miplevel < ktxTexture->numLevels; miplevel++)
                {
                    nuint offset;
                    ktx_result = ktxTexture_GetImageOffset(ktxTexture, miplevel, 0, 0, &offset);
                    if (ktx_result != KTX_SUCCESS)
                    {
                        //LOGF("Error loading KTX texture");
                    }

                    nuint levelSize = ktxTexture_GetImageSize(ktxTexture, miplevel);
                    ImageData imageData = result.GetLevel(miplevel);
                    Debug.Assert(imageData.SlicePitch == levelSize);
                    NativeMemory.Copy(ktxTexture->pData + offset, imageData.DataPointer.ToPointer(), levelSize);
                }
            }

            ktxTexture_Destroy(ktxTexture);
        }
        else if (fileType == ImageFileType.Exr)
        {
            Debug.Assert(alimerIsEXRFromMemory(data, (uint)data.Length) == TINYEXR_SUCCESS);

            float* pixelData;
            int width, height;
            byte* err = null;
            int ret = alimerLoadEXRFromMemory(&pixelData, &width, &height, data, (uint)data.Length, &err);
            if (ret != TINYEXR_SUCCESS)
            {
                if (err != null)
                {
                    string errMsg = Utf8CustomMarshaller.ConvertToManaged(err)!;
                    Log.Error($"Could not load EXR file: {errMsg}");
                    //std::cerr << "Could not load EXR file '" << path << "': " << err << std::endl;
                    alimerFreeEXRErrorMessage(err); // release memory of error message.
                }

                return default;
            }

            // TODO: Allow conversion  to 16-bit (https://eliemichel.github.io/LearnWebGPU/advanced-techniques/hdr-textures.html)
            ImageDescription imageDescription = ImageDescription.Image2D(
                PixelFormat.RGBA32Float,
                (uint)width,
                (uint)height,
                1u,
                1u
                );
            result = new(in imageDescription);
        }
        else
        {
            PixelFormat format = PixelFormat.RGBA8Unorm;
            int width, height, loaded_channels;
            void* imageData = null;
            if (stbi_is_16_bit_from_memory(data, data.Length) == 1)
            {
                imageData = stbi_load_16_from_memory(data, data.Length, &width, &height, &channels, 0);
                switch (channels)
                {
                    case 1:
                        format = PixelFormat.R16Uint;
                        break;
                    case 2:
                        format = PixelFormat.RG16Uint;
                        break;
                    case 4:
                        format = PixelFormat.RGBA16Uint;
                        break;
                    default:
                        throw new InvalidOperationException("Unsupported number of channels");
                }
            }
            else if (stbi_is_hdr_from_memory(data, data.Length) == 1)
            {
                // TODO: Allow conversion  to 16-bit (https://eliemichel.github.io/LearnWebGPU/advanced-techniques/hdr-textures.html)
                imageData = stbi_loadf_from_memory(data, data.Length, &width, &height, &channels, 4);
                format = PixelFormat.RGBA32Float;
            }
            else
            {
                imageData = stbi_load_from_memory(data, data.Length, &width, &height, &loaded_channels, channels);
                format = PixelFormat.RGBA8Unorm;
            }

            ImageDescription imageDescription = ImageDescription.Image2D(srgb ?
                format.LinearToSrgbFormat() : format,
                (uint)width,
                (uint)height,
                1u,
                1u
                );

            result = new(in imageDescription);
            ImageData resultImageData = result.GetLevel(0);
            NativeMemory.Copy(imageData, resultImageData.DataPointer.ToPointer(), (nuint)resultImageData.SlicePitch);
        }

        return result;
    }

    public bool SavePng(string filePath)
    {
        using FileStream stream = new(filePath, FileMode.Create);
        return SavePng(stream);
    }

    public bool SavePng(Stream stream)
    {
        GCHandle handle = GCHandle.Alloc(stream, GCHandleType.Normal);
        try
        {
            ImageData imageData = GetLevel(0);
            int result = stbi_write_png_to_func(&StbSaveCallback,
                (nint)handle,
                (int)imageData.Width,
                (int)imageData.Height,
                4,
                imageData.DataPointer.ToPointer(),
                (int)imageData.RowPitch);
            return result != 0;
        }
        finally
        {
            handle.Free();
        }
    }

    [UnmanagedCallersOnly]
    private static void StbSaveCallback(nint context, void* data, int size)
    {
        MemoryStream stream = (MemoryStream)GCHandle.FromIntPtr(context).Target!;
        stream.Write(new ReadOnlySpan<byte>(data, size));
    }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        if (_data != null)
        {
            NativeMemory.Free(_data);
        }
    }

    private static void SetupImageArray(byte* pixels, nuint memorySize, in ImageDescription description, ImageData[] levels)
    {
        Debug.Assert(pixels != null);
        Debug.Assert(memorySize > 0);
        Debug.Assert(levels.Length > 0);

        int index = 0;
        byte* pEndBits = pixels + memorySize;

        switch (description.Dimension)
        {
            case TextureDimension.Texture1D:
            case TextureDimension.Texture2D:
            case TextureDimension.TextureCube:
            {
                uint actualArrayLayers = (description.Dimension == TextureDimension.TextureCube) ? 6 * description.DepthOrArrayLayers : description.DepthOrArrayLayers;
                for (uint item = 0; item < actualArrayLayers; ++item)
                {
                    uint mipWidth = description.Width;
                    uint mipHeight = description.Height;

                    for (uint level = 0; level < description.MipLevelCount; ++level)
                    {
                        PixelFormatUtils.GetSurfaceInfo(description.Format, mipWidth, mipHeight, out uint rowPitch, out uint slicePitch, out uint _, out uint _);

                        levels[index] = new(mipWidth, mipHeight, description.Format, rowPitch, slicePitch, (IntPtr)pixels);
                        ++index;

                        pixels += slicePitch;
                        Debug.Assert(pixels <= pEndBits);

                        if (mipWidth > 1)
                            mipWidth >>= 1;

                        if (mipHeight > 1)
                            mipHeight >>= 1;
                    }
                }
            }
            break;

            case TextureDimension.Texture3D:
            {
                uint mipWidth = description.Width;
                uint mipHeight = description.Height;
                uint mipDepth = description.DepthOrArrayLayers;

                for (uint level = 0; level < description.MipLevelCount; ++level)
                {
                    PixelFormatUtils.GetSurfaceInfo(description.Format, mipWidth, mipHeight, out uint rowPitch, out uint slicePitch, out uint _, out uint _);

                    for (uint slice = 0; slice < mipDepth; ++slice)
                    {
                        Debug.Assert(index < levels.Length);

                        // We use the same memory organization that Direct3D 11 needs for D3D11_SUBRESOURCE_DATA
                        // with all slices of a given miplevel being continuous in memory
                        levels[index] = new(mipWidth, mipHeight, description.Format, rowPitch, slicePitch, (IntPtr)pixels);
                        ++index;

                        pixels += slicePitch;
                        Debug.Assert(pixels <= pEndBits);
                    }

                    if (mipWidth > 1)
                        mipWidth >>= 1;

                    if (mipHeight > 1)
                        mipHeight >>= 1;

                    if (mipDepth > 1)
                        mipDepth >>= 1;
                }
            }
            break;
        }
    }

    private static bool IsIco(ReadOnlySpan<byte> data)
    {
        // Reserved zero, type 1 for an icon and 2 for a cursor, and a non-zero entry count,
        // without which this matches far too much.
        if (data.Length < 6 || data[0] != 0 || data[1] != 0 || data[3] != 0)
            return false;
        if (data[2] is not (1 or 2))
            return false;
        return BinaryPrimitives.ReadUInt16LittleEndian(data[4..]) != 0;
    }

    private static bool IsPnm(ReadOnlySpan<byte> data)
    {
        // "P1".."P7" and whitespace, which keeps text files starting with a P out.
        if (data.Length < 3 || data[0] != (byte)'P')
            return false;
        if (data[1] is < (byte)'1' or > (byte)'7')
            return false;
        return data[2] is (byte)' ' or (byte)'\t' or (byte)'\r' or (byte)'\n' or (byte)'#';
    }

    private static bool IsBmp(ReadOnlySpan<byte> data)
    {
        // A two byte signature is weak, so the declared file size is checked too.
        if (data.Length < 14 || data[0] != (byte)'B' || data[1] != (byte)'M')
            return false;
        uint declared = BinaryPrimitives.ReadUInt32LittleEndian(data[2..]);
        uint offset = BinaryPrimitives.ReadUInt32LittleEndian(data[10..]);
        return declared >= 14 && offset >= 14;
    }

    private static bool IsKTX1(ReadOnlySpan<byte> data)
    {
        if (data.Length <= 12)
        {
            return false;
        }

        return data[..12].SequenceEqual(new byte[] { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A });
    }

    private static bool IsKTX2(ReadOnlySpan<byte> data)
    {
        if (data.Length <= 12)
        {
            return false;
        }

        return data[..12].SequenceEqual(new byte[] { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A });
    }

    private static bool IsRadianceHdr(ReadOnlySpan<byte> data)
    {
        if (data.Length <= 6)
        {
            return false;
        }

        ReadOnlySpan<byte> signature = "#?RADIANCE"u8;
        for (int i = 0; i < signature.Length; i++)
        {
            if (data[i] != signature[i])
            {
                // Try alt signature
                ReadOnlySpan<byte> altSignature = "#?RGBE"u8;
                for (int j = 0; j < altSignature.Length; j++)
                {
                    if (data[j] != altSignature[i])
                    {
                        return false;
                    }
                }

                return false;
            }
        }

        return true;
    }

    private static bool IsRaw(ReadOnlySpan<byte> data)
    {
        if (data.Length <= 15)
        {
            return false;
        }

        return data[..15].SequenceEqual("FUJIFILMCCD-RAW"u8);
    }

    private static bool IsTga(ReadOnlySpan<byte> data)
    {
        if (data.Length <= 18)
        {
            return false;
        }

        return data[2] == 0 && data[3] == 0 && data[4] == 2;
    }

    public static Image Read(ref ReadByteStream stream, Image? existingInstance)
    {
        // TODO: Magic number and version
        Debug.Assert((int)PixelFormat.Count <= byte.MaxValue);

        ImageDescription description = stream.ReadSerializable<ImageDescription>();
        //ImageDescription description = stream.ReadByteSerializable<ImageDescription>();
        Image result = new(in description);
        return result;
    }

    public static void Write(ref WriteByteStream stream, Image value)
    {
        stream.Write(value.Description);
    }

    private static VkFormat vkGetFormatFromOpenGLInternalFormat(GLenum internalFormat, ktx_rgba_astc_mapping_e mapping)
    {
        const GLenum GL_R8 = 0x8229;
        const GLenum GL_RG8 = 0x822B;
        const GLenum GL_RGB8 = 0x8051;
        const GLenum GL_RGBA8 = 0x8058;

        const GLenum GL_R8_SNORM = 0x8F94;
        const GLenum GL_RG8_SNORM = 0x8F95;
        const GLenum GL_RGB8_SNORM = 0x8F96;
        const GLenum GL_RGBA8_SNORM = 0x8F97;

        const GLenum GL_R8UI = 0x8232;
        const GLenum GL_RG8UI = 0x8238;
        const GLenum GL_RGB8UI = 0x8D7D;
        const GLenum GL_RGBA8UI = 0x8D7C;

        const GLenum GL_R8I = 0x8231;
        const GLenum GL_RG8I = 0x8237;
        const GLenum GL_RGB8I = 0x8D8F;
        const GLenum GL_RGBA8I = 0x8D8E;

        const GLenum GL_SR8 = 0x8FBD;
        const GLenum GL_SRG8 = 0x8FBE;
        const GLenum GL_SRGB8 = 0x8C41;
        const GLenum GL_SRGB8_ALPHA8 = 0x8C43;

        const GLenum GL_R16 = 0x822A;
        const GLenum GL_RG16 = 0x822C;
        const GLenum GL_RGB16 = 0x8054;
        const GLenum GL_RGBA16 = 0x805B;

        const GLenum GL_R16_SNORM = 0x8F98;
        const GLenum GL_RG16_SNORM = 0x8F99;
        const GLenum GL_RGB16_SNORM = 0x8F9A;
        const GLenum GL_RGBA16_SNORM = 0x8F9B;

        const GLenum GL_R16UI = 0x8234;
        const GLenum GL_RG16UI = 0x823A;
        const GLenum GL_RGB16UI = 0x8D77;
        const GLenum GL_RGBA16UI = 0x8D76;

        const GLenum GL_R16I = 0x8233;
        const GLenum GL_RG16I = 0x8239;
        const GLenum GL_RGB16I = 0x8D89;
        const GLenum GL_RGBA16I = 0x8D88;

        const GLenum GL_R16F = 0x822D;
        const GLenum GL_RG16F = 0x822F;
        const GLenum GL_RGB16F = 0x881B;
        const GLenum GL_RGBA16F = 0x881A;

        const GLenum GL_R32UI = 0x8236;
        const GLenum GL_RG32UI = 0x823C;
        const GLenum GL_RGB32UI = 0x8D71;
        const GLenum GL_RGBA32UI = 0x8D70;

        const GLenum GL_R32I = 0x8235;
        const GLenum GL_RG32I = 0x823B;
        const GLenum GL_RGB32I = 0x8D83;
        const GLenum GL_RGBA32I = 0x8D82;

        const GLenum GL_R32F = 0x822E;
        const GLenum GL_RG32F = 0x8230;
        const GLenum GL_RGB32F = 0x8815;
        const GLenum GL_RGBA32F = 0x8814;

        const GLenum GL_R3_G3_B2 = 0x2A10;
        const GLenum GL_RGB4 = 0x804F;
        const GLenum GL_RGB5 = 0x8050;
        const GLenum GL_RGB565 = 0x8D62;
        const GLenum GL_RGB10 = 0x8052;
        const GLenum GL_RGB12 = 0x8053;
        const GLenum GL_RGBA2 = 0x8055;
        const GLenum GL_RGBA4 = 0x8056;
        const GLenum GL_RGBA12 = 0x805A;
        const GLenum GL_RGB5_A1 = 0x8057;
        const GLenum GL_RGB10_A2 = 0x8059;
        const GLenum GL_RGB10_A2UI = 0x906F;
        const GLenum GL_R11F_G11F_B10F = 0x8C3A;
        const GLenum GL_RGB9_E5 = 0x8C3D;

        const GLenum GL_COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0;
        const GLenum GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
        const GLenum GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
        const GLenum GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;
        const GLenum GL_COMPRESSED_SRGB_S3TC_DXT1_EXT = 0x8C4C;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F;

        const GLenum GL_COMPRESSED_LUMINANCE_LATC1_EXT = 0x8C70;
        const GLenum GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT = 0x8C72;
        const GLenum GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT = 0x8C71;
        const GLenum GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT = 0x8C73;

        const GLenum GL_COMPRESSED_RED_RGTC1 = 0x8DBB;
        const GLenum GL_COMPRESSED_RG_RGTC2 = 0x8DBD;
        const GLenum GL_COMPRESSED_SIGNED_RED_RGTC1 = 0x8DBC;
        const GLenum GL_COMPRESSED_SIGNED_RG_RGTC2 = 0x8DBE;

        const GLenum GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F;
        const GLenum GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E;
        const GLenum GL_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D;

        const GLenum GL_ETC1_RGB8_OES = 0x8D64;

        const GLenum GL_COMPRESSED_RGB8_ETC2 = 0x9274;
        const GLenum GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276;
        const GLenum GL_COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
        const GLenum GL_COMPRESSED_SRGB8_ETC2 = 0x9275;
        const GLenum GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;

        const GLenum GL_COMPRESSED_R11_EAC = 0x9270;
        const GLenum GL_COMPRESSED_RG11_EAC = 0x9272;
        const GLenum GL_COMPRESSED_SIGNED_R11_EAC = 0x9271;
        const GLenum GL_COMPRESSED_SIGNED_RG11_EAC = 0x9273;

        const GLenum GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG = 0x8C01;
        const GLenum GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG = 0x8C00;
        const GLenum GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG = 0x8C03;
        const GLenum GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 0x8C02;
        const GLenum GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG = 0x9137;
        const GLenum GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG = 0x9138;

        const GLenum GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT = 0x8A54;
        const GLenum GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT = 0x8A55;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT = 0x8A56;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT = 0x8A57;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG = 0x93F0;
        const GLenum GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG = 0x93F1;

        const GLenum GL_COMPRESSED_RGBA_ASTC_4x4_KHR = 0x93B0;
        const GLenum GL_COMPRESSED_RGBA_ASTC_5x4_KHR = 0x93B1;
        const GLenum GL_COMPRESSED_RGBA_ASTC_5x5_KHR = 0x93B2;
        const GLenum GL_COMPRESSED_RGBA_ASTC_6x5_KHR = 0x93B3;
        const GLenum GL_COMPRESSED_RGBA_ASTC_6x6_KHR = 0x93B4;
        const GLenum GL_COMPRESSED_RGBA_ASTC_8x5_KHR = 0x93B5;
        const GLenum GL_COMPRESSED_RGBA_ASTC_8x6_KHR = 0x93B6;
        const GLenum GL_COMPRESSED_RGBA_ASTC_8x8_KHR = 0x93B7;
        const GLenum GL_COMPRESSED_RGBA_ASTC_10x5_KHR = 0x93B8;
        const GLenum GL_COMPRESSED_RGBA_ASTC_10x6_KHR = 0x93B9;
        const GLenum GL_COMPRESSED_RGBA_ASTC_10x8_KHR = 0x93BA;
        const GLenum GL_COMPRESSED_RGBA_ASTC_10x10_KHR = 0x93BB;
        const GLenum GL_COMPRESSED_RGBA_ASTC_12x10_KHR = 0x93BC;
        const GLenum GL_COMPRESSED_RGBA_ASTC_12x12_KHR = 0x93BD;

        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR = 0x93D0;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR = 0x93D1;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR = 0x93D2;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR = 0x93D3;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR = 0x93D4;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR = 0x93D5;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR = 0x93D6;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR = 0x93D7;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR = 0x93D8;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR = 0x93D9;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR = 0x93DA;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR = 0x93DB;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR = 0x93DC;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR = 0x93DD;

        const GLenum GL_COMPRESSED_RGBA_ASTC_3x3x3_OES = 0x93C0;
        const GLenum GL_COMPRESSED_RGBA_ASTC_4x3x3_OES = 0x93C1;
        const GLenum GL_COMPRESSED_RGBA_ASTC_4x4x3_OES = 0x93C2;
        const GLenum GL_COMPRESSED_RGBA_ASTC_4x4x4_OES = 0x93C3;
        const GLenum GL_COMPRESSED_RGBA_ASTC_5x4x4_OES = 0x93C4;
        const GLenum GL_COMPRESSED_RGBA_ASTC_5x5x4_OES = 0x93C5;
        const GLenum GL_COMPRESSED_RGBA_ASTC_5x5x5_OES = 0x93C6;
        const GLenum GL_COMPRESSED_RGBA_ASTC_6x5x5_OES = 0x93C7;
        const GLenum GL_COMPRESSED_RGBA_ASTC_6x6x5_OES = 0x93C8;
        const GLenum GL_COMPRESSED_RGBA_ASTC_6x6x6_OES = 0x93C9;

        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES = 0x93E0;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES = 0x93E1;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES = 0x93E2;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES = 0x93E3;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES = 0x93E4;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES = 0x93E5;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES = 0x93E6;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES = 0x93E7;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES = 0x93E8;
        const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES = 0x93E9;

        const GLenum GL_ATC_RGB_AMD = 0x8C92;
        const GLenum GL_ATC_RGBA_EXPLICIT_ALPHA_AMD = 0x8C93;
        const GLenum GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD = 0x87EE;

        const GLenum GL_PALETTE4_RGB8_OES = 0x8B90;
        const GLenum GL_PALETTE4_RGBA8_OES = 0x8B91;
        const GLenum GL_PALETTE4_R5_G6_B5_OES = 0x8B92;
        const GLenum GL_PALETTE4_RGBA4_OES = 0x8B93;
        const GLenum GL_PALETTE4_RGB5_A1_OES = 0x8B94;
        const GLenum GL_PALETTE8_RGB8_OES = 0x8B95;
        const GLenum GL_PALETTE8_RGBA8_OES = 0x8B96;
        const GLenum GL_PALETTE8_R5_G6_B5_OES = 0x8B97;
        const GLenum GL_PALETTE8_RGBA4_OES = 0x8B98;
        const GLenum GL_PALETTE8_RGB5_A1_OES = 0x8B99;

        const GLenum GL_DEPTH_COMPONENT16 = 0x81A5;
        const GLenum GL_DEPTH_COMPONENT24 = 0x81A6;
        const GLenum GL_DEPTH_COMPONENT32 = 0x81A7;
        const GLenum GL_DEPTH_COMPONENT32F = 0x8CAC;
        const GLenum GL_DEPTH_COMPONENT32F_NV = 0x8DAB;
        const GLenum GL_STENCIL_INDEX1 = 0x8D46;
        const GLenum GL_STENCIL_INDEX4 = 0x8D47;
        const GLenum GL_STENCIL_INDEX8 = 0x8D48;
        const GLenum GL_STENCIL_INDEX16 = 0x8D49;
        const GLenum GL_DEPTH24_STENCIL8 = 0x88F0;
        const GLenum GL_DEPTH32F_STENCIL8 = 0x8CAD;
        const GLenum GL_DEPTH32F_STENCIL8_NV = 0x8DAC;


        switch (internalFormat)
        {
            //
            // 8 bits per component
            //
            case GL_R8: return VK_FORMAT_R8_UNORM;                  // 1-component, 8-bit unsigned normalized
            case GL_RG8: return VK_FORMAT_R8G8_UNORM;               // 2-component, 8-bit unsigned normalized
            case GL_RGB8: return VK_FORMAT_R8G8B8_UNORM;                // 3-component, 8-bit unsigned normalized
            case GL_RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;         // 4-component, 8-bit unsigned normalized

            case GL_R8_SNORM: return VK_FORMAT_R8_SNORM;                    // 1-component, 8-bit signed normalized
            case GL_RG8_SNORM: return VK_FORMAT_R8G8_SNORM;             // 2-component, 8-bit signed normalized
            case GL_RGB8_SNORM: return VK_FORMAT_R8G8B8_SNORM;              // 3-component, 8-bit signed normalized
            case GL_RGBA8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;           // 4-component, 8-bit signed normalized

            case GL_R8UI: return VK_FORMAT_R8_UINT;                 // 1-component, 8-bit unsigned integer
            case GL_RG8UI: return VK_FORMAT_R8G8_UINT;                  // 2-component, 8-bit unsigned integer
            case GL_RGB8UI: return VK_FORMAT_R8G8B8_UINT;               // 3-component, 8-bit unsigned integer
            case GL_RGBA8UI: return VK_FORMAT_R8G8B8A8_UINT;                // 4-component, 8-bit unsigned integer

            case GL_R8I: return VK_FORMAT_R8_SINT;                  // 1-component, 8-bit signed integer
            case GL_RG8I: return VK_FORMAT_R8G8_SINT;                   // 2-component, 8-bit signed integer
            case GL_RGB8I: return VK_FORMAT_R8G8B8_SINT;                // 3-component, 8-bit signed integer
            case GL_RGBA8I: return VK_FORMAT_R8G8B8A8_SINT;             // 4-component, 8-bit signed integer

            case GL_SR8: return VK_FORMAT_R8_SRGB;                  // 1-component, 8-bit sRGB
            case GL_SRG8: return VK_FORMAT_R8G8_SRGB;                   // 2-component, 8-bit sRGB
            case GL_SRGB8: return VK_FORMAT_R8G8B8_SRGB;                // 3-component, 8-bit sRGB
            case GL_SRGB8_ALPHA8: return VK_FORMAT_R8G8B8A8_SRGB;               // 4-component, 8-bit sRGB

            //
            // 16 bits per component
            //
            case GL_R16: return VK_FORMAT_R16_UNORM;                    // 1-component, 16-bit unsigned normalized
            case GL_RG16: return VK_FORMAT_R16G16_UNORM;                // 2-component, 16-bit unsigned normalized
            case GL_RGB16: return VK_FORMAT_R16G16B16_UNORM;            // 3-component, 16-bit unsigned normalized
            case GL_RGBA16: return VK_FORMAT_R16G16B16A16_UNORM;        // 4-component, 16-bit unsigned normalized

            case GL_R16_SNORM: return VK_FORMAT_R16_SNORM;                  // 1-component, 16-bit signed normalized
            case GL_RG16_SNORM: return VK_FORMAT_R16G16_SNORM;              // 2-component, 16-bit signed normalized
            case GL_RGB16_SNORM: return VK_FORMAT_R16G16B16_SNORM;          // 3-component, 16-bit signed normalized
            case GL_RGBA16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;      // 4-component, 16-bit signed normalized

            case GL_R16UI: return VK_FORMAT_R16_UINT;                   // 1-component, 16-bit unsigned integer
            case GL_RG16UI: return VK_FORMAT_R16G16_UINT;               // 2-component, 16-bit unsigned integer
            case GL_RGB16UI: return VK_FORMAT_R16G16B16_UINT;           // 3-component, 16-bit unsigned integer
            case GL_RGBA16UI: return VK_FORMAT_R16G16B16A16_UINT;           // 4-component, 16-bit unsigned integer

            case GL_R16I: return VK_FORMAT_R16_SINT;                    // 1-component, 16-bit signed integer
            case GL_RG16I: return VK_FORMAT_R16G16_SINT;                // 2-component, 16-bit signed integer
            case GL_RGB16I: return VK_FORMAT_R16G16B16_SINT;            // 3-component, 16-bit signed integer
            case GL_RGBA16I: return VK_FORMAT_R16G16B16A16_SINT;            // 4-component, 16-bit signed integer

            case GL_R16F: return VK_FORMAT_R16_SFLOAT;              // 1-component, 16-bit floating-point
            case GL_RG16F: return VK_FORMAT_R16G16_SFLOAT;              // 2-component, 16-bit floating-point
            case GL_RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;          // 3-component, 16-bit floating-point
            case GL_RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;      // 4-component, 16-bit floating-point

            //
            // 32 bits per component
            //
            case GL_R32UI: return VK_FORMAT_R32_UINT;                   // 1-component, 32-bit unsigned integer
            case GL_RG32UI: return VK_FORMAT_R32G32_UINT;               // 2-component, 32-bit unsigned integer
            case GL_RGB32UI: return VK_FORMAT_R32G32B32_UINT;           // 3-component, 32-bit unsigned integer
            case GL_RGBA32UI: return VK_FORMAT_R32G32B32A32_UINT;           // 4-component, 32-bit unsigned integer

            case GL_R32I: return VK_FORMAT_R32_SINT;                    // 1-component, 32-bit signed integer
            case GL_RG32I: return VK_FORMAT_R32G32_SINT;                // 2-component, 32-bit signed integer
            case GL_RGB32I: return VK_FORMAT_R32G32B32_SINT;            // 3-component, 32-bit signed integer
            case GL_RGBA32I: return VK_FORMAT_R32G32B32A32_SINT;            // 4-component, 32-bit signed integer

            case GL_R32F: return VK_FORMAT_R32_SFLOAT;              // 1-component, 32-bit floating-point
            case GL_RG32F: return VK_FORMAT_R32G32_SFLOAT;              // 2-component, 32-bit floating-point
            case GL_RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;          // 3-component, 32-bit floating-point
            case GL_RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;      // 4-component, 32-bit floating-point

            //
            // Packed
            //
            case GL_R3_G3_B2: return VK_FORMAT_UNDEFINED;                   // 3-component 3:3:2,       unsigned normalized
            case GL_RGB4: return VK_FORMAT_UNDEFINED;                   // 3-component 4:4:4,       unsigned normalized
            case GL_RGB5: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;       // 3-component 5:5:5,       unsigned normalized
            case GL_RGB565: return VK_FORMAT_R5G6B5_UNORM_PACK16;       // 3-component 5:6:5,       unsigned normalized
            case GL_RGB10: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;   // 3-component 10:10:10,    unsigned normalized
            case GL_RGB12: return VK_FORMAT_UNDEFINED;                  // 3-component 12:12:12,    unsigned normalized
            case GL_RGBA2: return VK_FORMAT_UNDEFINED;                  // 4-component 2:2:2:2,     unsigned normalized
            case GL_RGBA4: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;      // 4-component 4:4:4:4,     unsigned normalized
            case GL_RGBA12: return VK_FORMAT_UNDEFINED;                 // 4-component 12:12:12:12, unsigned normalized
            case GL_RGB5_A1: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;        // 4-component 5:5:5:1,     unsigned normalized
            case GL_RGB10_A2: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;    // 4-component 10:10:10:2,  unsigned normalized
            case GL_RGB10_A2UI: return VK_FORMAT_A2R10G10B10_UINT_PACK32;   // 4-component 10:10:10:2,  unsigned integer
            case GL_R11F_G11F_B10F: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;   // 3-component 11:11:10,    floating-point
            case GL_RGB9_E5: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;   // 3-component/exp 9:9:9/5, floating-point

            //
            // S3TC/DXT/BC
            //

            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;     // line through 3D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;       // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: return VK_FORMAT_BC2_UNORM_BLOCK;            // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return VK_FORMAT_BC3_UNORM_BLOCK;            // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;     // line through 3D space, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;      // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: return VK_FORMAT_BC2_SRGB_BLOCK;           // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: return VK_FORMAT_BC3_SRGB_BLOCK;           // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

            case GL_COMPRESSED_LUMINANCE_LATC1_EXT: return VK_FORMAT_BC4_UNORM_BLOCK;           // line through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT: return VK_FORMAT_BC5_UNORM_BLOCK;         // two lines through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT: return VK_FORMAT_BC4_SNORM_BLOCK;            // line through 1D space, 4x4 blocks, signed normalized
            case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT: return VK_FORMAT_BC5_SNORM_BLOCK;          // two lines through 1D space, 4x4 blocks, signed normalized

            case GL_COMPRESSED_RED_RGTC1: return VK_FORMAT_BC4_UNORM_BLOCK;         // line through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RG_RGTC2: return VK_FORMAT_BC5_UNORM_BLOCK;          // two lines through 1D space, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SIGNED_RED_RGTC1: return VK_FORMAT_BC4_SNORM_BLOCK;          // line through 1D space, 4x4 blocks, signed normalized
            case GL_COMPRESSED_SIGNED_RG_RGTC2: return VK_FORMAT_BC5_SNORM_BLOCK;           // two lines through 1D space, 4x4 blocks, signed normalized

            case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: return VK_FORMAT_BC6H_UFLOAT_BLOCK;         // 3-component, 4x4 blocks, unsigned floating-point
            case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: return VK_FORMAT_BC6H_SFLOAT_BLOCK;           // 3-component, 4x4 blocks, signed floating-point
            case GL_COMPRESSED_RGBA_BPTC_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;           // 4-component, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: return VK_FORMAT_BC7_SRGB_BLOCK;          // 4-component, 4x4 blocks, sRGB

            //
            // ETC
            //
            case GL_ETC1_RGB8_OES: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;    // 3-component ETC1, 4x4 blocks, unsigned normalized

            case GL_COMPRESSED_RGB8_ETC2: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK; // 3-component ETC2, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;   // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA8_ETC2_EAC: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;  // 4-component ETC2, 4x4 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB8_ETC2: return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK; // 3-component ETC2, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;   // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;    // 4-component ETC2, 4x4 blocks, sRGB

            case GL_COMPRESSED_R11_EAC: return VK_FORMAT_EAC_R11_UNORM_BLOCK;       // 1-component ETC, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_RG11_EAC: return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;   // 2-component ETC, 4x4 blocks, unsigned normalized
            case GL_COMPRESSED_SIGNED_R11_EAC: return VK_FORMAT_EAC_R11_SNORM_BLOCK;        // 1-component ETC, 4x4 blocks, signed normalized
            case GL_COMPRESSED_SIGNED_RG11_EAC: return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;    // 2-component ETC, 4x4 blocks, signed normalized

            //
            // PVRTC
            //
            case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;  // 3-component PVRTC, 16x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;  // 3-component PVRTC,  8x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG; // 4-component PVRTC, 16x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG; // 4-component PVRTC,  8x8 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG: return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG; // 4-component PVRTC,  8x4 blocks, unsigned normalized
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG: return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG; // 4-component PVRTC,  4x4 blocks, unsigned normalized

            case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT: return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;  // 3-component PVRTC, 16x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT: return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;  // 3-component PVRTC,  8x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT: return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;    // 4-component PVRTC, 16x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT: return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;    // 4-component PVRTC,  8x8 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG: return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;    // 4-component PVRTC,  8x4 blocks, sRGB
            case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG: return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;    // 4-component PVRTC,  4x4 blocks, sRGB

            //
            // ASTC
            //
            case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK : VK_FORMAT_ASTC_4x4_UNORM_BLOCK;                    // 4-component ASTC, 4x4 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK : VK_FORMAT_ASTC_5x4_UNORM_BLOCK;                    // 4-component ASTC, 5x4 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK : VK_FORMAT_ASTC_5x5_UNORM_BLOCK;                    // 4-component ASTC, 5x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK : VK_FORMAT_ASTC_6x5_UNORM_BLOCK;                    // 4-component ASTC, 6x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK : VK_FORMAT_ASTC_6x6_UNORM_BLOCK;                    // 4-component ASTC, 6x6 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK : VK_FORMAT_ASTC_8x5_UNORM_BLOCK;                    // 4-component ASTC, 8x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK : VK_FORMAT_ASTC_8x6_UNORM_BLOCK;                    // 4-component ASTC, 8x6 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK : VK_FORMAT_ASTC_8x8_UNORM_BLOCK;                    // 4-component ASTC, 8x8 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK : VK_FORMAT_ASTC_10x5_UNORM_BLOCK;                  // 4-component ASTC, 10x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK : VK_FORMAT_ASTC_10x6_UNORM_BLOCK;                  // 4-component ASTC, 10x6 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK : VK_FORMAT_ASTC_10x8_UNORM_BLOCK;                  // 4-component ASTC, 10x8 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK : VK_FORMAT_ASTC_10x10_UNORM_BLOCK;                // 4-component ASTC, 10x10 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK : VK_FORMAT_ASTC_12x10_UNORM_BLOCK;                // 4-component ASTC, 12x10 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR) ? VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK : VK_FORMAT_ASTC_12x12_UNORM_BLOCK;                // 4-component ASTC, 12x12 blocks, unsigned normalized or signed float

            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;     // 4-component ASTC, 4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;     // 4-component ASTC, 5x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;     // 4-component ASTC, 5x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;     // 4-component ASTC, 6x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;     // 4-component ASTC, 6x6 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;     // 4-component ASTC, 8x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;     // 4-component ASTC, 8x6 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;     // 4-component ASTC, 8x8 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;       // 4-component ASTC, 10x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;       // 4-component ASTC, 10x6 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;       // 4-component ASTC, 10x8 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;     // 4-component ASTC, 10x10 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;     // 4-component ASTC, 12x10 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;     // 4-component ASTC, 12x12 blocks, sRGB

            case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_3x3x3_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_3x3x3_UNORM_BLOCK_EXT);            // 4-component ASTC, 3x3x3 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_4x3x3_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_4x3x3_UNORM_BLOCK_EXT);            // 4-component ASTC, 4x3x3 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_4x4x3_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_4x4x3_UNORM_BLOCK_EXT);            // 4-component ASTC, 4x4x3 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_4x4x4_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_4x4x4_UNORM_BLOCK_EXT);            // 4-component ASTC, 4x4x4 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_5x4x4_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_5x4x4_UNORM_BLOCK_EXT);            // 4-component ASTC, 5x4x4 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_5x5x4_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_5x5x4_UNORM_BLOCK_EXT);            // 4-component ASTC, 5x5x4 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_5x5x5_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_5x5x5_UNORM_BLOCK_EXT);            // 4-component ASTC, 5x5x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_6x5x5_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_6x5x5_UNORM_BLOCK_EXT);            // 4-component ASTC, 6x5x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_6x6x5_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_6x6x5_UNORM_BLOCK_EXT);            // 4-component ASTC, 6x6x5 blocks, unsigned normalized or signed float
            case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES: return (mapping == KTX_MAP_RGBA_ASTC_TO_HDR ? VK_FORMAT_ASTC_6x6x6_SFLOAT_BLOCK_EXT : VK_FORMAT_ASTC_6x6x6_UNORM_BLOCK_EXT);            // 4-component ASTC, 6x6x6 blocks, unsigned normalized or signed float

            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES: return VK_FORMAT_ASTC_3x3x3_SRGB_BLOCK_EXT;     // 4-component ASTC, 3x3x3 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES: return VK_FORMAT_ASTC_4x3x3_SRGB_BLOCK_EXT;     // 4-component ASTC, 4x3x3 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES: return VK_FORMAT_ASTC_4x4x3_SRGB_BLOCK_EXT;     // 4-component ASTC, 4x4x3 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES: return VK_FORMAT_ASTC_4x4x4_SRGB_BLOCK_EXT;     // 4-component ASTC, 4x4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES: return VK_FORMAT_ASTC_5x4x4_SRGB_BLOCK_EXT;     // 4-component ASTC, 5x4x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES: return VK_FORMAT_ASTC_5x5x4_SRGB_BLOCK_EXT;     // 4-component ASTC, 5x5x4 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES: return VK_FORMAT_ASTC_5x5x5_SRGB_BLOCK_EXT;     // 4-component ASTC, 5x5x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES: return VK_FORMAT_ASTC_6x5x5_SRGB_BLOCK_EXT;     // 4-component ASTC, 6x5x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES: return VK_FORMAT_ASTC_6x6x5_SRGB_BLOCK_EXT;     // 4-component ASTC, 6x6x5 blocks, sRGB
            case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES: return VK_FORMAT_ASTC_6x6x6_SRGB_BLOCK_EXT;     // 4-component ASTC, 6x6x6 blocks, sRGB

            //
            // ATC
            //
            case GL_ATC_RGB_AMD: return VK_FORMAT_UNDEFINED;                    // 3-component, 4x4 blocks, unsigned normalized
            case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD: return VK_FORMAT_UNDEFINED;                    // 4-component, 4x4 blocks, unsigned normalized
            case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD: return VK_FORMAT_UNDEFINED;                    // 4-component, 4x4 blocks, unsigned normalized

            //
            // Palletized
            //
            case GL_PALETTE4_RGB8_OES: return VK_FORMAT_UNDEFINED;                  // 3-component 8:8:8,   4-bit palette, unsigned normalized
            case GL_PALETTE4_RGBA8_OES: return VK_FORMAT_UNDEFINED;                 // 4-component 8:8:8:8, 4-bit palette, unsigned normalized
            case GL_PALETTE4_R5_G6_B5_OES: return VK_FORMAT_UNDEFINED;                  // 3-component 5:6:5,   4-bit palette, unsigned normalized
            case GL_PALETTE4_RGBA4_OES: return VK_FORMAT_UNDEFINED;                 // 4-component 4:4:4:4, 4-bit palette, unsigned normalized
            case GL_PALETTE4_RGB5_A1_OES: return VK_FORMAT_UNDEFINED;                   // 4-component 5:5:5:1, 4-bit palette, unsigned normalized
            case GL_PALETTE8_RGB8_OES: return VK_FORMAT_UNDEFINED;                  // 3-component 8:8:8,   8-bit palette, unsigned normalized
            case GL_PALETTE8_RGBA8_OES: return VK_FORMAT_UNDEFINED;                 // 4-component 8:8:8:8, 8-bit palette, unsigned normalized
            case GL_PALETTE8_R5_G6_B5_OES: return VK_FORMAT_UNDEFINED;                  // 3-component 5:6:5,   8-bit palette, unsigned normalized
            case GL_PALETTE8_RGBA4_OES: return VK_FORMAT_UNDEFINED;                 // 4-component 4:4:4:4, 8-bit palette, unsigned normalized
            case GL_PALETTE8_RGB5_A1_OES: return VK_FORMAT_UNDEFINED;                   // 4-component 5:5:5:1, 8-bit palette, unsigned normalized

            //
            // Depth/stencil
            //
            case GL_DEPTH_COMPONENT16: return VK_FORMAT_D16_UNORM;
            case GL_DEPTH_COMPONENT24: return VK_FORMAT_X8_D24_UNORM_PACK32;
            case GL_DEPTH_COMPONENT32: return VK_FORMAT_UNDEFINED;
            case GL_DEPTH_COMPONENT32F: return VK_FORMAT_D32_SFLOAT;
            case GL_DEPTH_COMPONENT32F_NV: return VK_FORMAT_D32_SFLOAT;
            case GL_STENCIL_INDEX1: return VK_FORMAT_UNDEFINED;
            case GL_STENCIL_INDEX4: return VK_FORMAT_UNDEFINED;
            case GL_STENCIL_INDEX8: return VK_FORMAT_S8_UINT;
            case GL_STENCIL_INDEX16: return VK_FORMAT_UNDEFINED;
            case GL_DEPTH24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
            case GL_DEPTH32F_STENCIL8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case GL_DEPTH32F_STENCIL8_NV: return VK_FORMAT_D32_SFLOAT_S8_UINT;

            default: return VK_FORMAT_UNDEFINED;
        }
    }
}
