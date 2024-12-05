// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Alimer.Utilities;
using CommunityToolkit.Diagnostics;
using static Alimer.AlimerApi;

namespace Alimer.Graphics;

public sealed unsafe class Image : DisposableObject
{
    private readonly MipMapDescription[] _mipmaps;
    private readonly ImageData[] _levels;
    private readonly byte* _data;

    public Image(in ImageDescription description)
    {
        Guard.IsTrue(description.Format != PixelFormat.Undefined);
        Guard.IsGreaterThanOrEqualTo(description.Width, 1);
        Guard.IsGreaterThanOrEqualTo(description.Height, 1);
        Guard.IsGreaterThanOrEqualTo(description.DepthOrArrayLayers, 1);

        Description = description;
        _mipmaps = new MipMapDescription[MipLevelCount];

        uint levelsCount = 0;
        switch (Description.Dimension)
        {
            case TextureDimension.Texture1D:
            case TextureDimension.Texture2D:
            case TextureDimension.TextureCube:
                for (uint arrayIndex = 0; arrayIndex < ArrayLayers; ++arrayIndex)
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

        _data = (byte*)NativeMemory.AllocZeroed((nuint)SizeInBytes);
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

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            if (_data != null)
            {
                NativeMemory.Free(_data);
            }
        }
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
    /// Get the number of mipmap levels in the.
    /// </summary>
    public uint MipLevelCount => Description.MipLevelCount;

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
        Guard.IsGreaterThan(level, 0);
        Guard.IsLessThan(level, MipLevelCount);
        return _mipmaps[level];
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

    public static unsafe Image FromMemory(Span<byte> data, int channels = 4, bool srgb = true)
    {
        // TODO: Add DDS, ASTC, KTX1 and KTX2 loading
        if (IsKTX1(data) || IsKTX2(data))
        {
            throw new NotImplementedException("KTX1/KTX2 loading is not supported");
        }
        else
        {
            fixed (byte* dataPtr = data)
            {
                nint handle = alimerImageCreateFromMemory(dataPtr, (uint)data.Length);

                ImageDesc imageDesc;
                nuint dataSize;

                alimerImageGetDesc(handle, &imageDesc);
                void* pData = alimerImageGetData(handle, &dataSize);

                byte[] imageData = new byte[dataSize];
                fixed (byte* destDataPtr = imageData)
                {
                    NativeMemory.Copy(pData, destDataPtr, dataSize);
                }

                alimerImageDestroy(handle);

                ImageDescription imageDescription = ImageDescription.Image2D(srgb ?
                    imageDesc.format.LinearToSrgbFormat() : imageDesc.format,
                    imageDesc.width,
                    imageDesc.height);
                return new(imageDescription, imageData);
            }
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
                for (uint item = 0; item < description.DepthOrArrayLayers; ++item)
                {
                    uint mipWidth = description.Width;
                    uint mipHeight = description.Height;

                    for (uint level = 0; level < description.MipLevelCount; ++level)
                    {
                        PixelFormatUtils.GetSurfaceInfo(description.Format, mipWidth, mipHeight, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);

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
                    PixelFormatUtils.GetSurfaceInfo(description.Format, mipWidth, mipHeight, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);

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
}
