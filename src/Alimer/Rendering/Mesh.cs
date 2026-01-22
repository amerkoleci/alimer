// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

public sealed partial class Mesh : DisposableObject
{
    private readonly List<SubMesh> _subMeshes = [];
    private BoundingBox _bounds;
    private bool _boundsDirty = true;
    private VertexBufferLayout _layout;

    public Mesh(GraphicsDevice device, uint vertexCount, params VertexAttribute[] attributes)
    {
        Guard.IsNotNull(device, nameof(device));

        Device = device;
        VertexCount = vertexCount;
        _layout = new VertexBufferLayout(attributes);
    }

    public GraphicsDevice Device { get; }

    /// <summary>
    /// The number of vertices in the vertex buffers.
    /// </summary>
    public uint VertexCount { get; }

    /// <summary>
    /// Gets the size, in bytes, of a single vertex element in the layout.
    /// </summary>
    public uint VertexStride => _layout.Stride;

    /// <summary>
    /// Gets the collection of sub-meshes that make up this mesh.
    /// </summary>
    public IReadOnlyList<SubMesh> SubMeshes => _subMeshes;

    public VertexBufferLayout Layout => _layout;
    public IndexFormat IndexFormat { get; private set; }

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

    public void SetPositions(ReadOnlySpan<Vector3> positions, uint vertexCount)
    {

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

    public class VertexBuffer
    {
        public VertexBuffer(uint vertex)
        {

        }
    }
}
