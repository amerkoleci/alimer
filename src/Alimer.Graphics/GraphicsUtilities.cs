// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public static class GraphicsUtilities
{
    public static uint GetMipLevelCount(uint width, uint height, uint depth = 1u)
    {
        uint mipLevelCount = 1;
        while (width > 1u || height > 1u || depth > 1u)
        {
            width = Math.Max(1, width >> 1);
            height = Math.Max(1, height >> 1);
            depth = Math.Max(1, depth >> 1);
            mipLevelCount++;
        }

        return mipLevelCount;
    }

    public static ulong ComputeTextureMemorySizeInBytes(in TextureDescription descriptor)
    {
        ulong size = 0;
        uint bytes_per_block = descriptor.Format.GetFormatBytesPerBlock();
        uint pixels_per_block = descriptor.Format.GetFormatHeightCompressionRatio();
        uint num_blocks_x = descriptor.Width / pixels_per_block;
        uint num_blocks_y = descriptor.Height / pixels_per_block;
        uint mipLevelCount = descriptor.MipLevelCount == 0 ? GetMipLevelCount(descriptor.Width, descriptor.Height, descriptor.DepthOrArrayLayers) : descriptor.MipLevelCount;
        for (uint arrayLayer = 0; arrayLayer < descriptor.DepthOrArrayLayers; ++arrayLayer)
        {
            for (int mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel)
            {
                uint width = Math.Max(1u, num_blocks_x >> mipLevel);
                uint height = Math.Max(1u, num_blocks_y >> mipLevel);
                uint depth = Math.Max(1u, descriptor.DepthOrArrayLayers >> mipLevel);
                size += width * height * depth * bytes_per_block;
            }
        }
        size *= (uint)descriptor.SampleCount;
        return size;
    }

    public static unsafe void CopyTo(this IntPtr source, IntPtr destination, int sizeInBytesToCopy)
    {
        CopyTo(new ReadOnlySpan<byte>(source.ToPointer(), sizeInBytesToCopy), destination);
    }

    public static unsafe void CopyTo(void* source, void* destination, int sizeInBytesToCopy)
    {
        CopyTo(new ReadOnlySpan<byte>(source, sizeInBytesToCopy), destination);
    }

    public static unsafe void CopyTo<T>(this Span<T> source, IntPtr destination) where T : unmanaged
    {
        source.CopyTo(new Span<T>(destination.ToPointer(), source.Length));
    }

    public static unsafe void CopyTo<T>(this Span<T> source, void* destination) where T : unmanaged
    {
        source.CopyTo(new Span<T>(destination, source.Length));
    }

    public static unsafe void CopyTo<T>(this ReadOnlySpan<T> source, IntPtr destination) where T : unmanaged
    {
        source.CopyTo(new Span<T>(destination.ToPointer(), source.Length));
    }

    public static unsafe void CopyTo<T>(this ReadOnlySpan<T> source, void* destination) where T : unmanaged
    {
        source.CopyTo(new Span<T>(destination, source.Length));
    }

    public static unsafe void CopyTo<T>(this IntPtr source, Span<T> destination) where T : unmanaged
    {
        new Span<T>(source.ToPointer(), destination.Length).CopyTo(destination);
    }
}
