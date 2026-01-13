// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets;
using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(Image))]
public class ImageTests
{
    [TestCase(PixelFormat.RGBA8Unorm, 1024u, 512u)]
    public void Test2D_Creation(PixelFormat format, uint width, uint height)
    {
        using Image image = new(ImageDescription.Image2D(format, width, height, 0));

        Assert.That(() => image.Dimension, Is.EqualTo(TextureDimension.Texture2D));

        Assert.That(() => image.Format, Is.EqualTo(format));
        Assert.That(() => image.Width, Is.EqualTo(width));
        Assert.That(() => image.Height, Is.EqualTo(height));
        Assert.That(() => image.Depth, Is.EqualTo(1u));
        Assert.That(() => image.ArrayLayers, Is.EqualTo(1u));
        Assert.That(() => image.MipLevelCount, Is.EqualTo(11u));
    }

    [TestCase(PixelFormat.RGBA8Unorm, 1024u, 512u, 256u)]
    public void Test3D_Creation(PixelFormat format, uint width, uint height, uint depth)
    {
        using Image image = new(ImageDescription.Image3D(format, width, height, depth));

        Assert.That(() => image.Dimension, Is.EqualTo(TextureDimension.Texture3D));
        Assert.That(() => image.Format, Is.EqualTo(format));
        Assert.That(() => image.Width, Is.EqualTo(width));
        Assert.That(() => image.Height, Is.EqualTo(height));
        Assert.That(() => image.Depth, Is.EqualTo(depth));
        Assert.That(() => image.ArrayLayers, Is.EqualTo(1u));
        Assert.That(() => image.MipLevelCount, Is.EqualTo(1u));
    }
}
