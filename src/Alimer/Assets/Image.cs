// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using Alimer.Serialization;
using static Alimer.AlimerApi;
using static Alimer.AlimerApi.KTX_error_code;
namespace Alimer.Assets;

public sealed unsafe class Image : Asset, IBinarySerializable
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

    public static Image FromFile(string filePath, int channels = 4, bool srgb = true)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(stream, channels, srgb);
    }

    public static Image FromStream(Stream stream, int channels = 4, bool srgb = true)
    {
        Span<byte> data = stackalloc byte[(int)stream.Length];
        stream.ReadExactly(data);
        return FromMemory(data, channels, srgb);
    }

    public static Image FromMemory(Span<byte> data, int channels = 4, bool srgb = true)
    {
        // TODO: Add DDS, ASTC, KTX1 and KTX2 loading
        fixed (byte* dataPtr = data)
        {
            ImageFileType fileType = alimerImageDetectFileType(dataPtr, (uint)data.Length);
            Image result;

            if (IsKTX1(data) || IsKTX2(data))
            {
                ktxTexture* ktxTexture;
                KTX_error_code ktx_result = ktxTexture_CreateFromMemory(dataPtr, (uint)data.Length,
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

                    format = Alimer.Graphics.Vulkan.VulkanUtils.FromVkFormat((Vortice.Vulkan.VkFormat)alimerVkFormatFromOpenGLInternalFormat(ktx_texture1->glInternalformat));

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
                        NativeMemory.Copy(ktxTexture->pData + offset, imageData.DataPointer.ToPointer(), levelSize);
                    }
                }

                ktxTexture_Destroy(ktxTexture);
            }
            else
            {
                nint handle = alimerImageCreateFromMemory(dataPtr, (uint)data.Length);

                ImageDesc imageDesc;
                nuint dataSize;

                alimerImageGetDesc(handle, &imageDesc);
                ImageDescription imageDescription = ImageDescription.Image2D(srgb ?
                    imageDesc.format.LinearToSrgbFormat() : imageDesc.format,
                    imageDesc.width,
                    imageDesc.height,
                    imageDesc.mipLevelCount,
                    imageDesc.depthOrArrayLayers
                    );

                if (imageDesc.mipLevelCount == 1)
                {
                    byte* pData = alimerImageGetPixels(handle, &dataSize);

                    byte[] imageData = new byte[dataSize];
                    fixed (byte* destDataPtr = imageData)
                    {
                        NativeMemory.Copy(pData, destDataPtr, dataSize);
                    }

                    alimerImageDestroy(handle);

                    result = new(imageDescription, imageData);
                }
                else
                {
                    result = new(in imageDescription);

                    for (uint miplevel = 0; miplevel < imageDescription.MipLevelCount; miplevel++)
                    {
                        ImageLevel* levelData = alimerImageGetLevel(handle, miplevel, 0u);
                        ImageData imageData = result.GetLevel(miplevel);
                        NativeMemory.Copy(levelData->pixels, imageData.DataPointer.ToPointer(), (nuint)imageData.RowPitch);
                        //memcpy(levelData->pixels, ktxTexture->pData + offset, levelSize);
                    }
                }
            }


            return result;
        }
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

#if TODO_SAVE
    private static Stream _bmpStream;

    [UnmanagedCallersOnly]
    private static unsafe void SaveBmpCallback(nint image, void* pData, uint dataSize)
    {
        ReadOnlySpan<byte> imageData = new byte[dataSize];
        fixed (byte* destDataPtr = imageData)
        {
            Unsafe.CopyBlockUnaligned(destDataPtr, pData, dataSize);
        }

        _bmpStream.Write(imageData);
    }

    [UnmanagedCallersOnly]
    private static unsafe void SaveCallback(nint image, void* pData, uint dataSize)
    {
        string assetsPath = Path.Combine(AppContext.BaseDirectory, "Test.png");

        byte[] imageData = new byte[dataSize];
        fixed (byte* destDataPtr = imageData)
        {
            Unsafe.CopyBlockUnaligned(destDataPtr, pData, dataSize);
        }

        File.WriteAllBytes(assetsPath, imageData);
    } 
#endif

    private static bool IsKTX1(Span<byte> data)
    {
        if (data.Length <= 12)
        {
            return false;
        }

        Span<byte> id = [0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A];
        for (int i = 0; i < 12; i++)
        {
            if (data[i] != id[i])
                return false;
        }

        return true;
    }

    private static bool IsHDR(Span<byte> data)
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

    private static bool IsKTX2(ReadOnlySpan<byte> data)
    {
        if (data.Length <= 12)
        {
            return false;
        }

        Span<byte> id = [0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A];
        for (int i = 0; i < 12; i++)
        {
            if (data[i] != id[i])
                return false;
        }

        return true;
    }

    public void Serialize(BinarySerializer serializer)
    {
        // TODO: Magic number and version
        Debug.Assert((int)PixelFormat.Count <= byte.MaxValue);

        if (serializer.IsReading)
        {
        }
        else
        {
        }

        throw new NotImplementedException();
    }
}
