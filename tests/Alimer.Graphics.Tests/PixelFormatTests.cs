// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Xunit;
using static Alimer.Graphics.PixelFormatUtils;

namespace Alimer.Graphics.Tests;

[Trait("Graphics", "PixelFormat")]
public class PixelFormatTests
{
    [Theory]
    [InlineData(PixelFormat.R8Unorm, FormatKind.Unorm, DxgiFormat.R8Unorm)]
    [InlineData(PixelFormat.R8Snorm, FormatKind.Snorm, DxgiFormat.R8Snorm)]
    [InlineData(PixelFormat.R8Uint, FormatKind.Uint, DxgiFormat.R8Uint)]
    [InlineData(PixelFormat.R8Sint, FormatKind.Sint, DxgiFormat.R8Sint)]
    public void Test8Bit(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();
        Assert.Equal(1u, formatInfo.BytesPerBlock);
        Assert.Equal(1u, formatInfo.BlockWidth);
        Assert.Equal(1u, formatInfo.BlockHeight);
        Assert.Equal(kind, formatInfo.Kind);
        Assert.False(format.IsSrgb());
        Assert.False(format.IsCompressedFormat());
        Assert.False(format.IsDepthFormat());
        Assert.False(format.IsStencilFormat());
        Assert.False(format.IsDepthStencilFormat());
        Assert.False(format.IsDepthOnlyFormat());
        Assert.Equal(format, format.SrgbToLinearFormat());
        Assert.Equal(format, format.LinearToSrgbFormat());
        Assert.Equal(8u, format.BitsPerPixel());
        Assert.Equal(dxgiFormat, format.ToDxgiFormat());

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.Equal(width, rowPitch);
        Assert.Equal(height * rowPitch, slicePitch);
        Assert.Equal(width, widthCount);
        Assert.Equal(height, heightCount);
    }

    [Theory]
    [InlineData(PixelFormat.R16Unorm, FormatKind.Unorm, DxgiFormat.R16Unorm)]
    [InlineData(PixelFormat.R16Snorm, FormatKind.Snorm, DxgiFormat.R16Snorm)]
    [InlineData(PixelFormat.R16Uint, FormatKind.Uint, DxgiFormat.R16Uint)]
    [InlineData(PixelFormat.R16Sint, FormatKind.Sint, DxgiFormat.R16Sint)]
    [InlineData(PixelFormat.R16Float, FormatKind.Float, DxgiFormat.R16Float)]
    [InlineData(PixelFormat.RG8Unorm, FormatKind.Unorm, DxgiFormat.R8G8Unorm)]
    [InlineData(PixelFormat.RG8Snorm, FormatKind.Snorm, DxgiFormat.R8G8Snorm)]
    [InlineData(PixelFormat.RG8Uint, FormatKind.Uint, DxgiFormat.R8G8Uint)]
    [InlineData(PixelFormat.RG8Sint, FormatKind.Sint, DxgiFormat.R8G8Sint)]
    public void Test16Bit(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();
        Assert.Equal(2u, formatInfo.BytesPerBlock);
        Assert.Equal(1u, formatInfo.BlockWidth);
        Assert.Equal(1u, formatInfo.BlockHeight);
        Assert.Equal(kind, formatInfo.Kind);
        Assert.False(format.IsSrgb());
        Assert.False(format.IsCompressedFormat());
        Assert.False(format.IsDepthFormat());
        Assert.False(format.IsStencilFormat());
        Assert.False(format.IsDepthStencilFormat());
        Assert.False(format.IsDepthOnlyFormat());
        Assert.Equal(format, format.SrgbToLinearFormat());
        Assert.Equal(format, format.LinearToSrgbFormat());
        Assert.Equal(16u, format.BitsPerPixel());
        Assert.Equal(dxgiFormat, format.ToDxgiFormat());

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.Equal(width * 2, rowPitch);
        Assert.Equal(height * rowPitch, slicePitch);
        Assert.Equal(width, widthCount);
        Assert.Equal(height, heightCount);
    }

    [Theory]
    [InlineData(PixelFormat.BGRA4Unorm, FormatKind.Unorm, DxgiFormat.B4G4R4A4Unorm)]
    [InlineData(PixelFormat.B5G6R5Unorm, FormatKind.Unorm, DxgiFormat.B5G6R5Unorm)]
    [InlineData(PixelFormat.BGR5A1Unorm, FormatKind.Unorm, DxgiFormat.B5G5R5A1Unorm)]
    public void Test16BitPacked(PixelFormat format, FormatKind kind, DxgiFormat dxgiFormat)
    {
        uint width = 1024;
        uint height = 512;
        PixelFormatInfo formatInfo = format.GetFormatInfo();
        Assert.Equal(2u, formatInfo.BytesPerBlock);
        Assert.Equal(1u, formatInfo.BlockWidth);
        Assert.Equal(1u, formatInfo.BlockHeight);
        Assert.Equal(kind, formatInfo.Kind);
        Assert.False(format.IsSrgb());
        Assert.False(format.IsCompressedFormat());
        Assert.False(format.IsDepthFormat());
        Assert.False(format.IsStencilFormat());
        Assert.False(format.IsDepthStencilFormat());
        Assert.False(format.IsDepthOnlyFormat());
        Assert.Equal(format, format.SrgbToLinearFormat());
        Assert.Equal(format, format.LinearToSrgbFormat());
        Assert.Equal(16u, format.BitsPerPixel());
        Assert.Equal(dxgiFormat, format.ToDxgiFormat());

        GetSurfaceInfo(format, width, height, out uint rowPitch, out uint slicePitch, out uint widthCount, out uint heightCount);
        Assert.Equal(width * 2, rowPitch);
        Assert.Equal(height * rowPitch, slicePitch);
        Assert.Equal(width, widthCount);
        Assert.Equal(height, heightCount);
    }

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
        else if(format.IsSrgb())
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
    [InlineData(PixelFormat.BC1RGBAUnormSrgb, PixelFormat.BC1RGBAUnorm)]
    [InlineData(PixelFormat.BC2RGBAUnormSrgb, PixelFormat.BC2RGBAUnorm)]
    [InlineData(PixelFormat.BC3RGBAUnormSrgb, PixelFormat.BC3RGBAUnorm)]
    [InlineData(PixelFormat.BC7RGBAUnormSrgb, PixelFormat.BC7RGBAUnorm)]
    [InlineData(PixelFormat.ETC2RGB8UnormSrgb, PixelFormat.ETC2RGB8Unorm)]
    [InlineData(PixelFormat.ETC2RGB8A1UnormSrgb, PixelFormat.ETC2RGB8A1Unorm)]
    [InlineData(PixelFormat.ETC2RGBA8UnormSrgb, PixelFormat.ETC2RGBA8Unorm)]
    public void TestSrgbToLinearFormat(PixelFormat srgbFormat, PixelFormat linearFormat)
    {
        Assert.True(srgbFormat.IsSrgb());
        Assert.Equal(linearFormat, srgbFormat.SrgbToLinearFormat());
        Assert.Equal(srgbFormat, linearFormat.LinearToSrgbFormat());
    }

    [Theory]
    [InlineData(PixelFormat.BC1RGBAUnorm, 8, FormatKind.Unorm, DxgiFormat.BC1Unorm)]
    [InlineData(PixelFormat.BC1RGBAUnormSrgb, 8, FormatKind.UnormSrgb, DxgiFormat.BC1UnormSrgb)]
    [InlineData(PixelFormat.BC2RGBAUnorm, 16, FormatKind.Unorm, DxgiFormat.BC2Unorm)]
    [InlineData(PixelFormat.BC2RGBAUnormSrgb, 16, FormatKind.UnormSrgb, DxgiFormat.BC2UnormSrgb)]
    [InlineData(PixelFormat.BC3RGBAUnorm, 16, FormatKind.Unorm, DxgiFormat.BC3Unorm)]
    [InlineData(PixelFormat.BC3RGBAUnormSrgb, 16, FormatKind.UnormSrgb, DxgiFormat.BC3UnormSrgb)]
    [InlineData(PixelFormat.BC4RUnorm, 8, FormatKind.Unorm, DxgiFormat.BC4Unorm)]
    [InlineData(PixelFormat.BC4RSnorm, 8, FormatKind.Snorm, DxgiFormat.BC4Snorm)]
    [InlineData(PixelFormat.BC5RGUnorm, 16, FormatKind.Unorm, DxgiFormat.BC5Unorm)]
    [InlineData(PixelFormat.BC5RGSnorm, 16, FormatKind.Snorm, DxgiFormat.BC5Snorm)]
    [InlineData(PixelFormat.BC6HRGBUfloat, 16, FormatKind.Float, DxgiFormat.BC6HUF16)]
    [InlineData(PixelFormat.BC6HRGBFloat, 16, FormatKind.Float, DxgiFormat.BC6HSF16)]
    [InlineData(PixelFormat.BC7RGBAUnorm, 16, FormatKind.Unorm, DxgiFormat.BC7Unorm)]
    [InlineData(PixelFormat.BC7RGBAUnormSrgb, 16, FormatKind.UnormSrgb, DxgiFormat.BC7UnormSrgb)]
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
        Assert.True(format.IsBCCompressedFormat());
        Assert.False(format.IsASTCCompressedFormat());
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
    [InlineData(PixelFormat.ETC2RGB8Unorm, 8, FormatKind.Unorm)]
    [InlineData(PixelFormat.ETC2RGB8UnormSrgb, 8, FormatKind.UnormSrgb)]
    [InlineData(PixelFormat.ETC2RGB8A1Unorm, 16, FormatKind.Unorm)]
    [InlineData(PixelFormat.ETC2RGB8A1UnormSrgb, 16, FormatKind.UnormSrgb)]
    [InlineData(PixelFormat.ETC2RGBA8Unorm, 16, FormatKind.Unorm)]
    [InlineData(PixelFormat.ETC2RGBA8UnormSrgb, 16, FormatKind.UnormSrgb)]
    [InlineData(PixelFormat.EACR11Unorm, 8, FormatKind.Unorm)]
    [InlineData(PixelFormat.EACR11Snorm, 8, FormatKind.Snorm)]
    [InlineData(PixelFormat.EACRG11Unorm, 16, FormatKind.Unorm)]
    [InlineData(PixelFormat.EACRG11Snorm, 16, FormatKind.Snorm)]
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
        Assert.False(format.IsBCCompressedFormat());
        Assert.False(format.IsASTCCompressedFormat());
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
}
