// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;

namespace Vortice.Graphics;

public readonly struct SwapChainDescriptor : IEquatable<SwapChainDescriptor>
{
    public Size Size { get; init; }
    public TextureFormat ColorFormat { get; init; }
    public PresentMode PresentMode { get; init; }
    public bool IsFullscreen { get; init; }

    public string? Label { get; init; }

    public SwapChainDescriptor(Size size,
        TextureFormat colorFormat = TextureFormat.BGRA8UNormSrgb,
        PresentMode presentMode = PresentMode.Fifo,
        bool isFullscreen = false,
        string? label = default)
    {
        Size = size;
        ColorFormat = colorFormat;
        PresentMode = presentMode;
        IsFullscreen = isFullscreen;
        Label = label;
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
            PresentMode == other.PresentMode &&
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
            hashCode.Add(PresentMode);
            hashCode.Add(IsFullscreen);
        }

        return hashCode.ToHashCode();
    }
}
