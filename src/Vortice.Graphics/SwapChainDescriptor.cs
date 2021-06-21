// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Drawing;

namespace Vortice.Graphics
{
    public readonly struct SwapChainDescriptor : IEquatable<SwapChainDescriptor>
    {
        public Size Size { get; }
        public TextureFormat ColorFormat { get; }
        //public TextureFormat DepthStencilFormat { get; }
        public bool EnableVerticalSync { get; }
        public bool IsFullscreen { get; }

        public SwapChainDescriptor(Size size,
            TextureFormat colorFormat = TextureFormat.BGRA8UNormSrgb,
            bool enableVerticalSync = true,
            bool isFullscreen = false)
        {
            Size = size;
            ColorFormat = colorFormat;
            EnableVerticalSync = enableVerticalSync;
            IsFullscreen = isFullscreen;
        }

        /// <summary>
        /// Compares two <see cref="SwapChainDescriptor"/> objects for equality.
        /// </summary>
        /// <param name="left">The <see cref="SwapChainDescriptor"/> on the left hand of the operand.</param>
        /// <param name="right">The <see cref="SwapChainDescriptor"/> on the right hand of the operand.</param>
        /// <returns>
        /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
        /// </returns>
        public static bool operator ==(SwapChainDescriptor left, SwapChainDescriptor right) => left.Equals(right);

        /// <summary>
        /// Compares two <see cref="SwapChainDescriptor"/> objects for inequality.
        /// </summary>
        /// <param name="left">The <see cref="SwapChainDescriptor"/> on the left hand of the operand.</param>
        /// <param name="right">The <see cref="SwapChainDescriptor"/> on the right hand of the operand.</param>
        /// <returns>
        /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
        /// </returns>
        public static bool operator !=(SwapChainDescriptor left, SwapChainDescriptor right) => !left.Equals(right);

        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is SwapChainDescriptor other && Equals(other);

        /// <inheritdoc/>
        public bool Equals(SwapChainDescriptor other)
        {
            return
                Size == other.Size &&
                ColorFormat == other.ColorFormat &&
                EnableVerticalSync == other.EnableVerticalSync &&
                IsFullscreen == other.IsFullscreen;
        }

        /// <summary>
        /// Returns the hash code for this instance.
        /// </summary>
        /// <returns>A 32-bit signed integer that is the hash code for this instance.</returns>
        public override int GetHashCode()
        {
            var hashCode = new HashCode();
            {
                hashCode.Add(Size);
                hashCode.Add(ColorFormat);
                hashCode.Add(EnableVerticalSync);
                hashCode.Add(IsFullscreen);
            }

            return hashCode.ToHashCode();
        }
    }
}
