// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Xunit;

namespace Alimer.Graphics.Tests;

[Trait("Graphics", "Image")]
public class ImageTests
{
    [Theory]
    [InlineData(PixelFormat.RGBA8Unorm, 1024, 512)]
    public void Test2D_Creation(PixelFormat format, int width, int height)
    {
        using Image image = new(ImageDescription.Image2D(format, width, height, 0));
        Assert.Equal(TextureDimension.Texture2D, image.Dimension);
        Assert.Equal(format, image.Format);
        Assert.Equal(width, image.Width);
        Assert.Equal(height, image.Height);
        Assert.Equal(1, image.Depth);
        Assert.Equal(1, image.ArrayLayers);
        Assert.Equal(11, image.MipLevelCount);
    }

    [Theory]
    [InlineData(PixelFormat.RGBA8Unorm, 1024, 512, 256)]
    public void Test3D_Creation(PixelFormat format, int width, int height, int depth)
    {
        using Image image = new(ImageDescription.Image3D(format, width, height, depth));
        Assert.Equal(TextureDimension.Texture3D, image.Dimension);
        Assert.Equal(format, image.Format);
        Assert.Equal(width, image.Width);
        Assert.Equal(height, image.Height);
        Assert.Equal(depth, image.Depth);
        Assert.Equal(1, image.ArrayLayers);
        Assert.Equal(1, image.MipLevelCount);
    }
}
