// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets;

namespace Alimer.Graphics;

public static class GraphicsUtilities
{
    public static ulong ComputeTextureMemorySizeInBytes(in TextureDescription description)
    {
        ulong size = 0;
        uint bytesPerBlock = description.Format.GetFormatBytesPerBlock();
        uint pixelsPerBlock = description.Format.GetFormatHeightCompressionRatio();
        uint numBlocksX = description.Width / pixelsPerBlock;
        uint numBlocksY = description.Height / pixelsPerBlock;
        uint mipLevelCount = description.MipLevelCount == 0 ? ImageDescription.GetMipLevelCount(description.Width, description.Height, description.DepthOrArrayLayers) : description.MipLevelCount;
        for (uint arrayLayer = 0; arrayLayer < description.DepthOrArrayLayers; ++arrayLayer)
        {
            for (int mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel)
            {
                uint width = Math.Max(1u, numBlocksX >> mipLevel);
                uint height = Math.Max(1u, numBlocksY >> mipLevel);
                uint depth = Math.Max(1u, description.DepthOrArrayLayers >> mipLevel);
                size += width * height * depth * bytesPerBlock;
            }
        }
        size *= (uint)description.SampleCount;
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
