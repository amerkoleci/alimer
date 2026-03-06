// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Graphics;
using SkiaSharp;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Utilities methods for skia. <see cref="SKBitmap"/>
/// </summary>
public static class SkiaUtils
{
    public static Span<SKColor> SKColorToColor(Span<SKColor> pixels)
    {
        // ARGB --> ABGR
        foreach (ref uint pixel in MemoryMarshal.Cast<SKColor, uint>(pixels))
        {
            pixel = ((pixel >> 16) & 0x000000FF) |
                    ((pixel << 16) & 0x00FF0000) |
                    (pixel & 0xFF00FF00);
        }

        return MemoryMarshal.Cast<SKColor, SKColor>(pixels);
    }

    public static TextureAsset ToAsset(this SKBitmap bitmap)
    {
        PixelFormat format = PixelFormat.RGBA8Unorm;
        if (bitmap.ColorType == SKColorType.Rgba8888)
        {
            return TextureAsset.Create(bitmap.Width, bitmap.Height, format, bitmap.GetPixelSpan());
        }

        Span<SKColor> pixels = SKColorToColor(bitmap.Pixels);
        return TextureAsset.Create(bitmap.Width, bitmap.Height, format, pixels);
    }

    public static SKBitmap ResizeImage(this SKBitmap original, in SKSizeI size)
    {
        return ResizeImage(original, size, new SKSamplingOptions(SKCubicResampler.Mitchell));
    }

    public static SKBitmap ResizeImage(this SKBitmap original, in SKSizeI size, SKSamplingOptions sampling)
    {
        float ratioX = (float)size.Width / original.Width;
        float ratioY = (float)size.Height / original.Height;
        float ratio = Math.Min(ratioX, ratioY);

        int newWidth = (int)(original.Width * ratio);
        int newHeight = (int)(original.Height * ratio);

        return original.Resize(new SKSizeI(newWidth, newHeight), sampling);
    }

    public static List<SKBitmap> GenerateMipmaps(this SKBitmap source)
    {
        return GenerateMipmaps(source, new SKSamplingOptions(SKCubicResampler.Mitchell));
    }

    public static List<SKBitmap> GenerateMipmaps(this SKBitmap source, SKSamplingOptions sampling)
    {
        uint mipLevelsCount = ImageDescription.GetMipLevelCount((uint)source.Width, (uint)source.Height);
        List<SKBitmap> mipmaps = [source];

        SKBitmap current = source;

        while (current.Width > 1 && current.Height > 1)
        {
            int newWidth = Math.Max(1, current.Width / 2);
            int newHeight = Math.Max(1, current.Height / 2);

            // SKFilterQuality.High = new SKSamplingOptions (SKCubicResampler.Mitchell)
            SKBitmap mipmap = current.Resize(new SKSizeI(newWidth, newHeight), sampling);
            mipmaps.Add(mipmap);
            current = mipmap;
        }

        return mipmaps;
    }
}
