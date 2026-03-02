// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Graphics.Tests;
using NUnit.Framework;

namespace Alimer.Rendering;

[TestFixture(TestOf = typeof(Mesh))]
public class MeshTests : GraphicsDeviceTestBase
{
    public MeshTests()
        : base(GraphicsBackend.Null)
    {

    }

    [Theory]
    public void TestDefault()
    {
        using Mesh mesh = new(Device, 24, VertexPositionNormalTexture.VertexAttributes);

        Assert.That(() => mesh.VertexCount, Is.EqualTo(24));
        Assert.That(() => mesh.IndexFormat, Is.EqualTo(IndexFormat.Uint16));
    }

}
