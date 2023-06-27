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

    public static ulong ComputeTextureMemorySizeInBytes(in TextureDescription description)
    {
        ulong size = 0;
        uint bytes_per_block = description.Format.GetFormatBytesPerBlock();
        uint pixels_per_block = description.Format.GetFormatHeightCompressionRatio();
        uint num_blocks_x = description.Width / pixels_per_block;
        uint num_blocks_y = description.Height / pixels_per_block;
        uint mipLevelCount = description.MipLevelCount == 0 ? GetMipLevelCount(description.Width, description.Height, description.DepthOrArrayLayers) : description.MipLevelCount;
        for (uint arrayLayer = 0; arrayLayer < description.DepthOrArrayLayers; ++arrayLayer)
        {
            for (int mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel)
            {
                uint width = Math.Max(1u, num_blocks_x >> mipLevel);
                uint height = Math.Max(1u, num_blocks_y >> mipLevel);
                uint depth = Math.Max(1u, description.DepthOrArrayLayers >> mipLevel);
                size += width * height * depth * bytes_per_block;
            }
        }
        size *= (uint)description.SampleCount;
        return size;
    }
}
