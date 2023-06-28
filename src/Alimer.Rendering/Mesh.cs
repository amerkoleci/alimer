// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public sealed class Mesh : DisposableObject
{
    public Mesh()
    {
    }

    /// <summary>
    /// The number of vertices in the vertex buffers.
    /// </summary>
    public int VertexCount { get; set; }

    /// <summary>
    /// Finalizes an instance of the <see cref="Mesh" /> class.
    /// </summary>
    ~Mesh() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
        }
    }
}
