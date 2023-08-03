// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Xunit;

namespace Alimer.Graphics.Tests;

[Trait("Graphics", "Image")]
public class ImageTests
{
    [Theory]
    [InlineData(PixelFormat.RGBA8Unorm, 1024, 512)]
    public void Test2D_Creation(PixelFormat format, uint width, uint height)
    {
        using Image image = new(width, height, format);
        Assert.Equal(TextureDimension.Texture2D, image.Dimension);
        Assert.Equal(format, image.Format);
        Assert.Equal(width, image.Width);
        Assert.Equal(height, image.Height);
        Assert.Equal(1u, image.Depth);
        Assert.Equal(1u, image.ArrayLayers);
        Assert.Equal(1u, image.MipLevelCount);
    }
}
