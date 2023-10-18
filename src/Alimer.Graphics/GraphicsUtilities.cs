// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public static class GraphicsUtilities
{
    public static ulong ComputeTextureMemorySizeInBytes(in TextureDescription descriptor)
    {
        ulong size = 0;
        uint bytesPerBlock = descriptor.Format.GetFormatBytesPerBlock();
        uint pixelsPerBlock = descriptor.Format.GetFormatHeightCompressionRatio();
        uint numBlocksX = descriptor.Width / pixelsPerBlock;
        uint numBlocksY = descriptor.Height / pixelsPerBlock;
        uint mipLevelCount = descriptor.MipLevelCount == 0 ? ImageDescription.GetMipLevelCount(descriptor.Width, descriptor.Height, descriptor.DepthOrArrayLayers) : descriptor.MipLevelCount;
        for (uint arrayLayer = 0; arrayLayer < descriptor.DepthOrArrayLayers; ++arrayLayer)
        {
            for (int mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel)
            {
                uint width = Math.Max(1u, numBlocksX >> mipLevel);
                uint height = Math.Max(1u, numBlocksY >> mipLevel);
                uint depth = Math.Max(1u, descriptor.DepthOrArrayLayers >> mipLevel);
                size += width * height * depth * bytesPerBlock;
            }
        }
        size *= (uint)descriptor.SampleCount;
        return size;
    }

    public static bool BlendEnabled(in RenderTargetBlendState state)
    {
        return
            state.ColorBlendOperation != BlendOperation.Add ||
            state.AlphaBlendOperation != BlendOperation.Add ||
            state.SourceColorBlendFactor != BlendFactor.One ||
            state.DestinationColorBlendFactor != BlendFactor.Zero ||
            state.SourceAlphaBlendFactor != BlendFactor.One ||
            state.DestinationAlphaBlendFactor != BlendFactor.Zero;
    }

    public static bool StencilTestEnabled(in DepthStencilState state)
    {
        return
            state.StencilBack.CompareFunction != CompareFunction.Always ||
            state.StencilBack.FailOperation != StencilOperation.Keep ||
            state.StencilBack.DepthFailOperation != StencilOperation.Keep ||
            state.StencilBack.PassOperation != StencilOperation.Keep ||
            state.StencilFront.CompareFunction != CompareFunction.Always ||
            state.StencilFront.FailOperation != StencilOperation.Keep ||
            state.StencilFront.DepthFailOperation != StencilOperation.Keep ||
            state.StencilFront.PassOperation != StencilOperation.Keep;
    }

    public static void Swap<T>(ref T source, ref T destination)
    {
        (destination, source) = (source, destination);
    }
}
