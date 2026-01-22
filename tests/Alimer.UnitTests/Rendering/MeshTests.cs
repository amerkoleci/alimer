// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using NUnit.Framework;

namespace Alimer.Rendering;

[TestFixture(TestOf = typeof(Mesh))]
public class MeshTests
{
    [Theory]
    public void TestDefault()
    {
        Mesh mesh = new Mesh();

        Assert.That(() => mesh.VertexCount, Is.EqualTo(0));
        Assert.That(() => mesh.IndexFormat, Is.EqualTo(IndexFormat.UInt16));
    }

}
