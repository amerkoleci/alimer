// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

public sealed partial class Mesh : DisposableObject
{
    private readonly List<SubMesh> _subMeshes = [];
    private BoundingBox _bounds;
    private bool _boundsDirty = true;
    private VertexBufferLayout _layout;

    public Mesh(GraphicsDevice device)
    {
        Guard.IsNotNull(device, nameof(device));

        Device = device;
    }

    public GraphicsDevice Device { get; }

    /// <summary>
    /// The number of vertices in the vertex buffers.
    /// </summary>
    public int VertexCount { get; set; }

    /// <summary>
    /// Gets the collection of sub-meshes that make up this mesh.
    /// </summary>
    public IReadOnlyList<SubMesh> SubMeshes => _subMeshes;

    public VertexBufferLayout Layout => _layout;

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

    /// <summary>
    /// 
    /// </summary>
    /// <param name="indexStart"></param>
    /// <param name="indexCount"></param>
    /// <param name="materialIndex"></param>
    /// <returns></returns>
    public SubMesh AddSubMesh(int indexStart, int indexCount, int materialIndex = 0)
    {
        SubMesh subMesh = new(this, indexStart, indexCount, materialIndex);
        _subMeshes.Add(subMesh);
        return subMesh;
    }
}
