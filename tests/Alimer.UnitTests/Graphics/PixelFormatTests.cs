// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;
using static Alimer.Graphics.PixelFormatUtils;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(PixelFormat))]
public class PixelFormatTests
{
    [TestCase(PixelFormat.R8Unorm, FormatKind.UNorm, DxgiFormat.R8Unorm)]
    [TestCase(PixelFormat.R8Snorm, FormatKind.SNorm, DxgiFormat.R8Snorm)]
    [TestCase(PixelFormat.R8Uint, FormatKind.UInt, DxgiFormat.R8Uint)]
    [TestCase(PixelFormat.R8Sint, FormatKind.SInt, DxgiFormat.R8Sint)]
    public void Test8Bit(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();

        Assert.That(() => formatInfo.BytesPerBlock, Is.EqualTo(1u));
        Assert.That(() => formatInfo.BlockWidth, Is.EqualTo(1u));
        Assert.That(() => formatInfo.BlockHeight, Is.EqualTo(1u));
        Assert.That(() => formatInfo.Kind, Is.EqualTo(kind));

        Assert.That(() => format.IsSrgb(), Is.False);
        Assert.That(() => format.IsCompressedFormat(), Is.False);
        Assert.That(() => format.IsDepthFormat(), Is.False);
        Assert.That(() => format.IsStencilFormat(), Is.False);
        Assert.That(() => format.IsDepthStencilFormat(), Is.False);
        Assert.That(() => format.IsDepthOnlyFormat(), Is.False);

        Assert.That(() => format.SrgbToLinearFormat(), Is.EqualTo(format));
        Assert.That(() => format.LinearToSrgbFormat(), Is.EqualTo(format));
        Assert.That(() => format.BitsPerPixel(), Is.EqualTo(8u));

        Assert.That(() => format.ToDxgiFormat(), Is.EqualTo(dxgiFormat));

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);

        Assert.That(() => rowPitch, Is.EqualTo(width));
        Assert.That(() => slicePitch, Is.EqualTo(height * rowPitch));
        Assert.That(() => width, Is.EqualTo(widthCount));
        Assert.That(() => height, Is.EqualTo(heightCount));
    }

    [TestCase(PixelFormat.R16Unorm, FormatKind.UNorm, DxgiFormat.R16Unorm)]
    [TestCase(PixelFormat.R16Snorm, FormatKind.SNorm, DxgiFormat.R16Snorm)]
    [TestCase(PixelFormat.R16Uint, FormatKind.UInt, DxgiFormat.R16Uint)]
    [TestCase(PixelFormat.R16Sint, FormatKind.SInt, DxgiFormat.R16Sint)]
    [TestCase(PixelFormat.R16Float, FormatKind.Float, DxgiFormat.R16Float)]
    [TestCase(PixelFormat.RG8Unorm, FormatKind.UNorm, DxgiFormat.R8G8Unorm)]
    [TestCase(PixelFormat.RG8Snorm, FormatKind.SNorm, DxgiFormat.R8G8Snorm)]
    [TestCase(PixelFormat.RG8Uint, FormatKind.UInt, DxgiFormat.R8G8Uint)]
    [TestCase(PixelFormat.RG8Sint, FormatKind.SInt, DxgiFormat.R8G8Sint)]
    public void Test16Bit(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();

        Assert.That(() => formatInfo.BytesPerBlock, Is.EqualTo(2u));
        Assert.That(() => formatInfo.BlockWidth, Is.EqualTo(1u));
        Assert.That(() => formatInfo.BlockHeight, Is.EqualTo(1u));
        Assert.That(() => formatInfo.Kind, Is.EqualTo(kind));
        Assert.That(() => format.IsSrgb(), Is.False);
        Assert.That(() => format.IsCompressedFormat(), Is.False);
        Assert.That(() => format.IsDepthFormat(), Is.False);
        Assert.That(() => format.IsStencilFormat(), Is.False);
        Assert.That(() => format.IsDepthStencilFormat(), Is.False);
        Assert.That(() => format.IsDepthOnlyFormat(), Is.False);

        Assert.That(() => format.SrgbToLinearFormat(), Is.EqualTo(format));
        Assert.That(() => format.LinearToSrgbFormat(), Is.EqualTo(format));
        Assert.That(() => format.BitsPerPixel(), Is.EqualTo(16u));
        Assert.That(() => format.ToDxgiFormat(), Is.EqualTo(dxgiFormat));

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.That(() => rowPitch, Is.EqualTo(width * 2));
        Assert.That(() => slicePitch, Is.EqualTo(height * rowPitch));
        Assert.That(() => width, Is.EqualTo(widthCount));
        Assert.That(() => height, Is.EqualTo(heightCount));
    }

    [TestCase(PixelFormat.BGRA4Unorm, FormatKind.UNorm, DxgiFormat.B4G4R4A4Unorm)]
    [TestCase(PixelFormat.B5G6R5Unorm, FormatKind.UNorm, DxgiFormat.B5G6R5Unorm)]
    [TestCase(PixelFormat.BGR5A1Unorm, FormatKind.UNorm, DxgiFormat.B5G5R5A1Unorm)]
    public void Test16BitPacked(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();

        Assert.That(() => formatInfo.BytesPerBlock, Is.EqualTo(2u));
        Assert.That(() => formatInfo.BlockWidth, Is.EqualTo(1u));
        Assert.That(() => formatInfo.BlockHeight, Is.EqualTo(1u));
        Assert.That(() => formatInfo.Kind, Is.EqualTo(kind));

        Assert.That(() => format.IsSrgb(), Is.False);
        Assert.That(() => format.IsCompressedFormat(), Is.False);
        Assert.That(() => format.IsDepthFormat(), Is.False);
        Assert.That(() => format.IsStencilFormat(), Is.False);
        Assert.That(() => format.IsDepthStencilFormat(), Is.False);
        Assert.That(() => format.IsDepthOnlyFormat(), Is.False);

        Assert.That(() => format.SrgbToLinearFormat(), Is.EqualTo(format));
        Assert.That(() => format.LinearToSrgbFormat(), Is.EqualTo(format));
        Assert.That(() => format.BitsPerPixel(), Is.EqualTo(16u));
        Assert.That(() => format.ToDxgiFormat(), Is.EqualTo(dxgiFormat));

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.That(() => rowPitch, Is.EqualTo(width * 2));
        Assert.That(() => slicePitch, Is.EqualTo(height * rowPitch));
        Assert.That(() => width, Is.EqualTo(widthCount));
        Assert.That(() => height, Is.EqualTo(heightCount));
    }


#if TODO
    [Theory]
    [InlineData(PixelFormat.R32Uint, FormatKind.Uint, DxgiFormat.R32Uint)]
    [InlineData(PixelFormat.R32Sint, FormatKind.Sint, DxgiFormat.R32Sint)]
    [InlineData(PixelFormat.R32Float, FormatKind.Float, DxgiFormat.R32Float)]
    [InlineData(PixelFormat.RG16Unorm, FormatKind.Unorm, DxgiFormat.R16G16Unorm)]
    [InlineData(PixelFormat.RG16Snorm, FormatKind.Snorm, DxgiFormat.R16G16Snorm)]
    [InlineData(PixelFormat.RG16Uint, FormatKind.Uint, DxgiFormat.R16G16Uint)]
    [InlineData(PixelFormat.RG16Sint, FormatKind.Sint, DxgiFormat.R16G16Sint)]
    [InlineData(PixelFormat.RG16Float, FormatKind.Float, DxgiFormat.R16G16Float)]
    [InlineData(PixelFormat.RGBA8Unorm, FormatKind.Unorm, DxgiFormat.R8G8B8A8Unorm, PixelFormat.RGBA8UnormSrgb)]
    [InlineData(PixelFormat.RGBA8UnormSrgb, FormatKind.UnormSrgb, DxgiFormat.R8G8B8A8UnormSrgb, PixelFormat.RGBA8Unorm)]
    public void Test32Bit(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat, PixelFormat srgbFormat = PixelFormat.Undefined)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();
        Assert.Equal(4u, formatInfo.BytesPerBlock);
        Assert.Equal(1u, formatInfo.BlockWidth);
        Assert.Equal(1u, formatInfo.BlockHeight);
        Assert.Equal(kind, formatInfo.Kind);

        Assert.False(format.IsCompressedFormat());
        Assert.False(format.IsDepthFormat());
        Assert.False(format.IsStencilFormat());
        Assert.False(format.IsDepthStencilFormat());
        Assert.False(format.IsDepthOnlyFormat());
        if (srgbFormat == PixelFormat.Undefined)
        {
            Assert.False(format.IsSrgb());
            Assert.Equal(format, format.SrgbToLinearFormat());
            Assert.Equal(format, format.LinearToSrgbFormat());
        }
        else if (format.IsSrgb())
        {
            Assert.Equal(srgbFormat, format.SrgbToLinearFormat());
            Assert.Equal(format, srgbFormat.LinearToSrgbFormat());
        }
        else
        {
            Assert.Equal(format, srgbFormat.SrgbToLinearFormat());
            Assert.Equal(srgbFormat, format.LinearToSrgbFormat());
        }

        Assert.Equal(32u, format.BitsPerPixel());
        Assert.Equal(dxgiFormat, format.ToDxgiFormat());

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.Equal(width * 4, rowPitch);
        Assert.Equal(height * rowPitch, slicePitch);
        Assert.Equal(width, widthCount);
        Assert.Equal(height, heightCount);
    }

    [Theory]
    [InlineData(PixelFormat.RGBA8UnormSrgb, PixelFormat.RGBA8Unorm)]
    [InlineData(PixelFormat.BGRA8UnormSrgb, PixelFormat.BGRA8Unorm)]
    [InlineData(PixelFormat.Bc1RgbaUnormSrgb, PixelFormat.Bc1RgbaUnorm)]
    [InlineData(PixelFormat.Bc2RgbaUnormSrgb, PixelFormat.Bc2RgbaUnorm)]
    [InlineData(PixelFormat.Bc3RgbaUnormSrgb, PixelFormat.Bc3RgbaUnorm)]
    [InlineData(PixelFormat.Bc7RgbaUnormSrgb, PixelFormat.Bc7RgbaUnorm)]
    [InlineData(PixelFormat.Etc2Rgb8UnormSrgb, PixelFormat.Etc2Rgb8Unorm)]
    [InlineData(PixelFormat.Etc2Rgb8A1UnormSrgb, PixelFormat.Etc2Rgb8A1Unorm)]
    [InlineData(PixelFormat.Etc2Rgba8UnormSrgb, PixelFormat.Etc2Rgba8Unorm)]
    public void TestSrgbToLinearFormat(PixelFormat srgbFormat, PixelFormat linearFormat)
    {
        Assert.True(srgbFormat.IsSrgb());
        Assert.Equal(linearFormat, srgbFormat.SrgbToLinearFormat());
        Assert.Equal(srgbFormat, linearFormat.LinearToSrgbFormat());
    }

    [Theory]
    [InlineData(PixelFormat.Bc1RgbaUnorm, 8, FormatKind.Unorm, DxgiFormat.BC1Unorm)]
    [InlineData(PixelFormat.Bc1RgbaUnormSrgb, 8, FormatKind.UnormSrgb, DxgiFormat.BC1UnormSrgb)]
    [InlineData(PixelFormat.Bc2RgbaUnorm, 16, FormatKind.Unorm, DxgiFormat.BC2Unorm)]
    [InlineData(PixelFormat.Bc2RgbaUnormSrgb, 16, FormatKind.UnormSrgb, DxgiFormat.BC2UnormSrgb)]
    [InlineData(PixelFormat.Bc3RgbaUnorm, 16, FormatKind.Unorm, DxgiFormat.BC3Unorm)]
    [InlineData(PixelFormat.Bc3RgbaUnormSrgb, 16, FormatKind.UnormSrgb, DxgiFormat.BC3UnormSrgb)]
    [InlineData(PixelFormat.Bc4RUnorm, 8, FormatKind.Unorm, DxgiFormat.BC4Unorm)]
    [InlineData(PixelFormat.Bc4RSnorm, 8, FormatKind.Snorm, DxgiFormat.BC4Snorm)]
    [InlineData(PixelFormat.Bc5RgUnorm, 16, FormatKind.Unorm, DxgiFormat.BC5Unorm)]
    [InlineData(PixelFormat.Bc5RgSnorm, 16, FormatKind.Snorm, DxgiFormat.BC5Snorm)]
    [InlineData(PixelFormat.Bc6hRgbUfloat, 16, FormatKind.Float, DxgiFormat.BC6HUF16)]
    [InlineData(PixelFormat.Bc6hRgbFloat, 16, FormatKind.Float, DxgiFormat.BC6HSF16)]
    [InlineData(PixelFormat.Bc7RgbaUnorm, 16, FormatKind.Unorm, DxgiFormat.BC7Unorm)]
    [InlineData(PixelFormat.Bc7RgbaUnormSrgb, 16, FormatKind.UnormSrgb, DxgiFormat.BC7UnormSrgb)]
    public void TestBCCompressed(PixelFormat format, uint bytesPerBlock, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();
        Assert.Equal(bytesPerBlock, formatInfo.BytesPerBlock);
        Assert.Equal(4u, formatInfo.BlockWidth);
        Assert.Equal(4u, formatInfo.BlockHeight);
        Assert.Equal(kind, formatInfo.Kind);

        Assert.True(format.IsCompressedFormat());
        Assert.True(format.IsBcCompressedFormat());
        Assert.False(format.IsAstcCompressedFormat());
        Assert.False(format.IsDepthFormat());
        Assert.False(format.IsStencilFormat());
        Assert.False(format.IsDepthStencilFormat());
        Assert.False(format.IsDepthOnlyFormat());

        Assert.Equal(bytesPerBlock / 2, format.BitsPerPixel());
        Assert.Equal(dxgiFormat, format.ToDxgiFormat());

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.Equal(width / 4, widthCount);
        Assert.Equal(height / 4, heightCount);
        Assert.Equal(widthCount * bytesPerBlock, rowPitch);
        Assert.Equal(heightCount * rowPitch, slicePitch);
    }

    [Theory]
    [InlineData(PixelFormat.Etc2Rgb8Unorm, 8, FormatKind.Unorm)]
    [InlineData(PixelFormat.Etc2Rgb8UnormSrgb, 8, FormatKind.UnormSrgb)]
    [InlineData(PixelFormat.Etc2Rgb8A1Unorm, 16, FormatKind.Unorm)]
    [InlineData(PixelFormat.Etc2Rgb8A1UnormSrgb, 16, FormatKind.UnormSrgb)]
    [InlineData(PixelFormat.Etc2Rgba8Unorm, 16, FormatKind.Unorm)]
    [InlineData(PixelFormat.Etc2Rgba8UnormSrgb, 16, FormatKind.UnormSrgb)]
    [InlineData(PixelFormat.EacR11Unorm, 8, FormatKind.Unorm)]
    [InlineData(PixelFormat.EacR11Snorm, 8, FormatKind.Snorm)]
    [InlineData(PixelFormat.EacRg11Unorm, 16, FormatKind.Unorm)]
    [InlineData(PixelFormat.EacRg11Snorm, 16, FormatKind.Snorm)]
    public void TestETC2_EAC_Compressed(PixelFormat format, uint bytesPerBlock, FormatKind kind)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();
        Assert.Equal(bytesPerBlock, formatInfo.BytesPerBlock);
        Assert.Equal(4u, formatInfo.BlockWidth);
        Assert.Equal(4u, formatInfo.BlockHeight);
        Assert.Equal(kind, formatInfo.Kind);

        Assert.True(format.IsCompressedFormat());
        Assert.False(format.IsBcCompressedFormat());
        Assert.False(format.IsAstcCompressedFormat());
        Assert.False(format.IsDepthFormat());
        Assert.False(format.IsStencilFormat());
        Assert.False(format.IsDepthStencilFormat());
        Assert.False(format.IsDepthOnlyFormat());

        Assert.Equal(bytesPerBlock / 2, format.BitsPerPixel());

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.Equal(width / 4, widthCount);
        Assert.Equal(height / 4, heightCount);
        Assert.Equal(widthCount * bytesPerBlock, rowPitch);
        Assert.Equal(heightCount * rowPitch, slicePitch);
    } 
#endif
}
