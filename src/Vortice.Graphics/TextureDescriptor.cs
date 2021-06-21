// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    public record TextureDescriptor
    {
        public TextureDescriptor(TextureDimension dimension, TextureFormat format, int width, int height, int depthOrArrayLayers, int mipLevels = 0, int sampleCount = 1, TextureUsage usage = TextureUsage.Sampled)
        {
            Dimension = dimension;
            Format = format;
            Width = width;
            Height = height;
            DepthOrArrayLayers = depthOrArrayLayers;
            MipLevels = mipLevels == 1 ? CountMipLevels(width, height, dimension == TextureDimension.Texture3D ? depthOrArrayLayers : 1) : mipLevels;
            SampleCount = sampleCount;
            Usage = usage;
        }

        public static TextureDescriptor Texture1D(
            TextureFormat format,
            int width,
            int arrayLayers = 1,
            int mipLevels = 0,
            TextureUsage usage = TextureUsage.Sampled)
        {
            return new TextureDescriptor(
                TextureDimension.Texture1D,
                format,
                width,
                1,
                arrayLayers,
                mipLevels,
                1,
                usage);
        }

        public static TextureDescriptor Texture2D(
            TextureFormat format,
            int width,
            int height,
            int arrayLayers = 1,
            int mipLevels = 0,
            int sampleCount = 1,
            TextureUsage usage = TextureUsage.Sampled)
        {
            return new TextureDescriptor(
                TextureDimension.Texture2D,
                format,
                width,
                height,
                arrayLayers,
                mipLevels,
                sampleCount,
                usage);
        }

        public static TextureDescriptor Texture3D(
            TextureFormat format,
            int width,
            int height,
            int depth = 1,
            int mipLevels = 0,
            TextureUsage usage = TextureUsage.Sampled)
        {
            return new TextureDescriptor(
                TextureDimension.Texture3D,
                format,
                width,
                height,
                depth,
                mipLevels,
                1,
                usage);
        }

        public TextureDimension Dimension { get; init; }

        public TextureFormat Format { get; init; }

        public int Width { get; init; }
        public int Height { get; init; }
        public int DepthOrArrayLayers { get; init; }
        public int MipLevels { get; init; }
        public int SampleCount { get; init; }

        public TextureUsage Usage { get; init; }

        /// <summary>
        /// Returns the number of mip levels given a texture size
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="depth"></param>
        /// <returns></returns>
        private static int CountMipLevels(int width, int height, int depth = 1)
        {
            int numMips = 0;
            int size = Math.Max(Math.Max(width, height), depth);
            while (1 << numMips <= size)
            {
                ++numMips;
            }

            if (1 << numMips < size)
                ++numMips;

            return numMips;
        }
    }
}
