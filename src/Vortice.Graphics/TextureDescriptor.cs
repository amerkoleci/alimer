// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    public readonly struct TextureDescriptor : IEquatable<TextureDescriptor>
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

        public TextureDimension Dimension { get; }

        public TextureFormat Format { get; }

        public int Width { get; }
        public int Height { get; }
        public int DepthOrArrayLayers { get; }
        public int MipLevels { get; }
        public int SampleCount { get; }

        public TextureUsage Usage { get; }

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

        /// <summary>
        /// Compares two <see cref="TextureDescriptor"/> objects for equality.
        /// </summary>
        /// <param name="left">The <see cref="TextureDescriptor"/> on the left hand of the operand.</param>
        /// <param name="right">The <see cref="TextureDescriptor"/> on the right hand of the operand.</param>
        /// <returns>
        /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
        /// </returns>
        public static bool operator ==(TextureDescriptor left, TextureDescriptor right) => left.Equals(right);

        /// <summary>
        /// Compares two <see cref="TextureDescriptor"/> objects for inequality.
        /// </summary>
        /// <param name="left">The <see cref="TextureDescriptor"/> on the left hand of the operand.</param>
        /// <param name="right">The <see cref="TextureDescriptor"/> on the right hand of the operand.</param>
        /// <returns>
        /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
        /// </returns>
        public static bool operator !=(TextureDescriptor left, TextureDescriptor right) => !left.Equals(right);

        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is TextureDescriptor other && Equals(other);

        /// <inheritdoc/>
        public bool Equals(TextureDescriptor other)
        {
            return
                Dimension == other.Dimension &&
                Format == other.Format &&
                Width == other.Width &&
                Height == other.Height &&
                DepthOrArrayLayers == other.DepthOrArrayLayers &&
                MipLevels == other.MipLevels &&
                SampleCount == other.SampleCount &&
                Usage == other.Usage;
        }

        /// <summary>
        /// Returns the hash code for this instance.
        /// </summary>
        /// <returns>A 32-bit signed integer that is the hash code for this instance.</returns>
        public override int GetHashCode()
        {
            var hashCode = new HashCode();
            {
                hashCode.Add(Dimension);
                hashCode.Add(Format);
                hashCode.Add(Width);
                hashCode.Add(Height);
                hashCode.Add(DepthOrArrayLayers);
                hashCode.Add(MipLevels);
                hashCode.Add(SampleCount);
                hashCode.Add(Usage);
            }

            return hashCode.ToHashCode();
        }
    }
}
