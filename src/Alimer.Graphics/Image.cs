// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Alimer.Graphics.ImageNativeApi;

namespace Alimer.Graphics;

public sealed class Image
{
    public Image(uint width, uint height, PixelFormat format = PixelFormat.Rgba8Unorm)
    {
        Width = width;
        Height = height;
        Format = format;
        BytesPerPixel = format.GetFormatBytesPerBlock();
        RowPitch = format.GetFormatRowPitch(width);

        if (format.IsCompressedFormat())
        {
            uint blockSizeY = format.GetFormatHeightCompressionRatio();
            Debug.Assert(height % blockSizeY == 0); // Should divide evenly
            SizeInBytes = RowPitch * (height / blockSizeY);
        }
        else
        {
            SizeInBytes = height * RowPitch;
        }

        Data = new byte[SizeInBytes];
    }

    private Image(uint width, uint height, PixelFormat format, Memory<byte> data)
    {
        Width = width;
        Height = height;
        Format = format;
        Data = data;
        BytesPerPixel = format.GetFormatBytesPerBlock();
        RowPitch = format.GetFormatRowPitch(width);

        if (format.IsCompressedFormat())
        {
            uint blockSizeY = format.GetFormatHeightCompressionRatio();
            Debug.Assert(height % blockSizeY == 0); // Should divide evenly
            SizeInBytes = RowPitch * (height / blockSizeY);
        }
        else
        {
            SizeInBytes = height * RowPitch;
        }
    }

    /// <summary>
    /// Get the width of the image.
    /// </summary>
    public uint Width { get; }

    /// <summary>
    /// Get the height of the image.
    /// </summary>
    public uint Height { get; }

    /// <summary>
    /// Gets the data format.
    /// </summary>
    public PixelFormat Format { get; }

    /// <summary>
    /// Get the number of bytes per single pixel.
    /// </summary>
    public uint BytesPerPixel { get; }

    /// <summary>
    /// Get the row pitch in bytes. For compressed formats this corresponds to one row of blocks, not pixels.
    /// </summary>
    public uint RowPitch { get; }

    /// <summary>
    /// Get the data size in bytes.
    /// </summary>
    public uint SizeInBytes { get; }

    /// <summary>
    /// Get the image data.
    /// </summary>
    public Memory<byte> Data { get; }

    public static Image FromFile(string filePath, bool srgb = true)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(stream, srgb);
    }

    public static Image FromStream(Stream stream, bool srgb = true)
    {
        byte[] data = new byte[stream.Length];
        stream.Read(data, 0, (int)stream.Length);
        return FromMemory(data, srgb);
    }

    public static unsafe Image FromMemory(byte[] data, bool srgb = true)
    {
        fixed (byte* dataPtr = data)
        {
            nint handle = alimerImageFromMemory(dataPtr, (uint)data.Length);

            TextureDimension dimension = alimerImageGetDimension(handle);
            ImageFormat imageFormat = alimerImageGetFormat(handle);
            PixelFormat format = FromImageFormat(imageFormat);
            uint width = alimerImageGetWidth(handle, 0);
            uint height = alimerImageGetHeight(handle, 0);
            uint dataSize = alimerImageGetDataSize(handle);
            void* pData = alimerImageGetData(handle);

            byte[] imageData = new byte[dataSize];
            fixed (byte* destDataPtr = imageData)
            {
                Unsafe.CopyBlockUnaligned(destDataPtr, pData, dataSize);
            }

            //var result = image_save_png_memory(handle, &SaveCallback) == 1;

            alimerImageDestroy(handle);

            return new Image(width, height, srgb ? format.LinearToSrgbFormat() : format, imageData);
        }
    }

    private static PixelFormat FromImageFormat(ImageFormat format)
    {
        switch (format)
        {
            case ImageFormat.R8:
                return PixelFormat.R8Unorm;
            case ImageFormat.RG8:
                return PixelFormat.Rg8Unorm;
            case ImageFormat.RGBA8:
                return PixelFormat.Rgba8Unorm;
            case ImageFormat.R16:
                return PixelFormat.R16Unorm;
            case ImageFormat.RG16:
                return PixelFormat.Rg16Unorm;
            case ImageFormat.RGBA16:
                return PixelFormat.Rgba16Unorm;
            case ImageFormat.R16F:
                return PixelFormat.R16Float;
            case ImageFormat.RG16F:
                return PixelFormat.Rg16Float;
            case ImageFormat.RGBA16F:
                return PixelFormat.Rgba16Float;
            case ImageFormat.R32F:
                return PixelFormat.R32Float;
            case ImageFormat.RG32F:
                return PixelFormat.Rg32Float;
            case ImageFormat.RGBA32F:
                return PixelFormat.Rgba32Float;
            default:
                return PixelFormat.Undefined;
        }
    }

#if !WINDOWS_UWP
    [UnmanagedCallersOnly]
    private static unsafe void SaveCallback(void* pData, uint dataSize)
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

    private static bool IsKTX1(byte[] data)
    {
        if (data.Length <= 12)
        {
            return false;
        }

        Span<byte> id = stackalloc byte[] { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
        for (int i = 0; i < 12; i++)
        {
            if (data[i] != id[i])
                return false;
        }

        return true;
    }

    private static bool IsKTX2(byte[] data)
    {
        if (data.Length <= 12)
        {
            return false;
        }

        Span<byte> id = stackalloc byte[] { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
        for (int i = 0; i < 12; i++)
        {
            if (data[i] != id[i])
                return false;
        }

        return true;
    }
}
