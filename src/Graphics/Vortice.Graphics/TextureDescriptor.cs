// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    /// <summary>
    /// Structure that describes the <see cref="Texture"/>.
    /// </summary>
    public readonly struct TextureDescriptor : IEquatable<TextureDescriptor>
    {
        public TextureDescriptor(
            TextureDimension dimension,
            TextureFormat format,
            int width,
            int height,
            int depthOrArraySize,
            int mipLevels = 1,
            TextureUsage usage = TextureUsage.ShaderRead,
            TextureSampleCount sampleCount = TextureSampleCount.Count1)
        {
            Dimension = dimension;
            Format = format;
            Width = width;
            Height = height;
            DepthOrArraySize = depthOrArraySize;
            MipLevels = mipLevels == 1 ? CountMipLevels(width, height, dimension == TextureDimension.Texture3D ? depthOrArraySize : 1) : mipLevels;
            SampleCount = sampleCount;
            Usage = usage;
        }

        public static TextureDescriptor Texture1D(
            TextureFormat format,
            int width,
            int mipLevels = 1,
            int arrayLayers = 1,
            TextureUsage usage = TextureUsage.ShaderRead)
        {
            return new TextureDescriptor(
                TextureDimension.Texture1D,
                format,
                width,
                1,
                arrayLayers,
                mipLevels,
                usage,
                TextureSampleCount.Count1);
        }

        public static TextureDescriptor Texture2D(
            TextureFormat format,
            int width,
            int height,
            int mipLevels = 1,
            int arrayLayers = 1,
            TextureUsage usage = TextureUsage.ShaderRead,
            TextureSampleCount sampleCount = TextureSampleCount.Count1
            )
        {
            return new TextureDescriptor(
                TextureDimension.Texture2D,
                format,
                width,
                height,
                arrayLayers,
                mipLevels,
                usage,
                sampleCount);
        }

        public static TextureDescriptor Texture3D(
            TextureFormat format,
            int width,
            int height,
            int depth = 1,
            int mipLevels = 1,
            TextureUsage usage = TextureUsage.ShaderRead)
        {
            return new TextureDescriptor(
                TextureDimension.Texture3D,
                format,
                width,
                height,
                depth,
                mipLevels,
                usage,
                TextureSampleCount.Count1);
        }

        /// <summary>
        /// Dimension of texture.
        /// </summary>
        public TextureDimension Dimension { get; }

        public TextureFormat Format { get; }

        public int Width { get;  }
        public int Height { get; }
        public int DepthOrArraySize { get; }
        public int MipLevels { get; }
        public TextureUsage Usage { get; }
        public TextureSampleCount SampleCount { get;  }

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
                DepthOrArraySize == other.DepthOrArraySize &&
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
                hashCode.Add(DepthOrArraySize);
                hashCode.Add(MipLevels);
                hashCode.Add(SampleCount);
                hashCode.Add(Usage);
            }

            return hashCode.ToHashCode();
        }
    }
}
