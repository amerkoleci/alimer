// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers.Binary;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

// TODO: Mesh on GPU with CPU access

public sealed partial class Mesh : DisposableObject
{
    private readonly List<SubMesh> _subMeshes = [];
    private BoundingBox _bounds;
    private bool _boundsDirty = true;
    private MeshVertexBufferLayout _layout;
    private readonly VertexBuffer _vertexBuffer;
    private readonly IndexBuffer _indexBuffer;
    private int _positionOffset = -1;
    private GraphicsBuffer? _gpuVertexBuffer;
    private GraphicsBuffer? _gpuIndexBuffer;

    public Mesh()
    {
        _vertexBuffer = new VertexBuffer();
        _indexBuffer = new IndexBuffer();
    }

    /// <summary>
    /// The number of vertices in the vertex buffers.
    /// </summary>
    public int VertexCount => _vertexBuffer.ElementCount;

    /// <summary>
    /// Gets the size, in bytes, of a single vertex element in the layout.
    /// </summary>
    public int VertexStride => _layout.Stride;

    public GraphicsBuffer? GpuVertexBuffer => _gpuVertexBuffer;
    public GraphicsBuffer? GpuIndexBuffer => _gpuIndexBuffer;

    public MeshVertexBufferLayout VertexBufferLayout => _layout;

    public IndexFormat IndexFormat => _indexBuffer.IndexFormat;

    /// <summary>
    /// Gets the collection of sub-meshes that make up this mesh.
    /// </summary>
    public IReadOnlyList<SubMesh> SubMeshes => _subMeshes;

    /// <summary>
    /// Gets the bounds of the mesh.
    /// </summary>
    public BoundingBox Bounds => _bounds;

    /// <summary>
    /// Finalizes an instance of the <see cref="Mesh" /> class.
    /// </summary>
    ~Mesh() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            DestroyGpuData();
        }
    }

    #region Gpu
    public unsafe void CreateGpuData(GraphicsDevice device, bool clearCpuData = true)
    {
        Guard.IsNotNull(device, nameof(device));

        DestroyGpuData();

        // Subset maps a part of the mesh to a material:
        if (_subMeshes.Count == 0)
        {
            _ = AddSubMesh(0, _indexBuffer.ElementCount);
        }

        ulong vertexBufferSize = (ulong)(_vertexBuffer.ElementCount * _layout.Stride);
        ulong indexBufferSize = (ulong)(_indexBuffer.ElementCount * _indexBuffer.ElementSize);
        BufferDescriptor vertexBufferDesc = new(vertexBufferSize, BufferUsage.Vertex | BufferUsage.ShaderRead);
        BufferDescriptor indexBufferDesc = new(indexBufferSize, BufferUsage.Index | BufferUsage.ShaderRead);

        _gpuVertexBuffer = device.CreateBuffer(vertexBufferDesc, _vertexBuffer.GetRawData());
        _gpuIndexBuffer = device.CreateBuffer(indexBufferDesc, _indexBuffer.GetRawData());

        if (clearCpuData)
        {
            ClearCpuData();
        }
    }

    private void DestroyGpuData()
    {
        _gpuVertexBuffer?.Dispose();
        _gpuIndexBuffer?.Dispose();
    }

    private void ClearCpuData()
    {
        _vertexBuffer.Clear();
        _indexBuffer.Clear();
    }

    public void Draw(RenderPassEncoder encoder, uint instanceCount = 1)
    {
        Guard.IsNotNull(encoder, nameof(encoder));

        encoder.SetVertexBuffer(0, _gpuVertexBuffer!);
        encoder.SetIndexBuffer(_gpuIndexBuffer!, IndexFormat);
        foreach (SubMesh subMesh in _subMeshes)
        {
            encoder.DrawIndexed((uint)subMesh.IndexCount, instanceCount, (uint)subMesh.IndexStart, 0, 0);
        }
    }
    #endregion

    public SubMesh AddSubMesh(int indexStart, int indexCount, int materialIndex = 0)
    {
        SubMesh subMesh = new(this, indexStart, indexCount, materialIndex);
        _subMeshes.Add(subMesh);
        return subMesh;
    }

    public bool HasVertexAttribute(MeshVertexAttributeSemantic semantic)
    {
        return _layout.HasAttribute(semantic);
    }

    public MeshVertexAttribute? GetVertexAttribute(MeshVertexAttributeSemantic semantic)
    {
        return _layout.GetAttribute(semantic);
    }

    public int GetVertexAttributeOffset(MeshVertexAttributeSemantic semantic)
    {
        return _layout.GetAttributeOffset(semantic);
    }

    public static unsafe int GetVertexAttributeSize(MeshVertexAttributeSemantic semantic)
    {
        return semantic switch
        {
            MeshVertexAttributeSemantic.Position => sizeof(Vector3),
            MeshVertexAttributeSemantic.Normal => sizeof(Vector3),
            MeshVertexAttributeSemantic.Tangent => sizeof(Vector4),
            MeshVertexAttributeSemantic.Color => sizeof(Color),
            MeshVertexAttributeSemantic.BlendIndices => sizeof(Vector4), // UInt4?
            MeshVertexAttributeSemantic.BlendWeights => sizeof(Vector4),
            MeshVertexAttributeSemantic.TexCoord0 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord1 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord2 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord3 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord4 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord5 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord6 => sizeof(Vector2),
            MeshVertexAttributeSemantic.TexCoord7 => sizeof(Vector2),
            _ => throw new ArgumentOutOfRangeException(nameof(semantic), "Unknown vertex attribute semantic."),
        };
    }

    public void SetVertexAttributes(params Span<MeshVertexAttribute> attributes)
    {
        _layout = new MeshVertexBufferLayout(attributes);
        _positionOffset = _layout.GetAttributeOffset(MeshVertexAttributeSemantic.Position);
    }

    public void SetVertexAttributes<T>()
        where T : unmanaged, IMeshVertex
    {
        _layout = new MeshVertexBufferLayout(T.VertexAttributes);
        _positionOffset = _layout.GetAttributeOffset(MeshVertexAttributeSemantic.Position);
    }

    public void GetVertexAttributes(Span<MeshVertexAttribute> attributes)
    {
        _layout.GetAttributes(attributes);
    }

    public void SetPositions(ReadOnlySpan<Vector3> positions)
    {
        _vertexBuffer.SetElementData(positions, _positionOffset);
        _boundsDirty = true;
    }

    public void GetPositions(Span<Vector3> positions)
    {
        _vertexBuffer.GetElementData(positions, _positionOffset);
    }

    public void GetNormals(Span<Vector3> normals)
    {
        GetAttributeData(MeshVertexAttributeSemantic.Normal, normals);
    }

    public unsafe void GetAttributeData<T>(MeshVertexAttributeSemantic semantic, Span<T> data)
        where T : unmanaged
    {
        MeshVertexAttribute? attribute = _layout.GetAttribute(semantic) ?? throw new InvalidOperationException($"Mesh does not contain attribute with semantic {semantic}.");

        int offset = attribute.Value.Offset;
        int attributeSize = GetVertexAttributeSize(attribute.Value.Semantic);
        Guard.IsEqualTo(sizeof(T), attributeSize, nameof(T));

        _vertexBuffer.GetElementData(data, offset);
    }

    public unsafe void SetAttributeData<T>(MeshVertexAttributeSemantic semantic, Span<T> data)
        where T : unmanaged
    {
        MeshVertexAttribute? attribute = _layout.GetAttribute(semantic) ?? throw new InvalidOperationException($"Mesh does not contain attribute with semantic {semantic}.");

        int offset = attribute.Value.Offset;
        int attributeSize = GetVertexAttributeSize(attribute.Value.Semantic);
        Guard.IsEqualTo(sizeof(T), attributeSize, nameof(T));

        _vertexBuffer.SetElementData(data, offset);
    }

    public void SetVertices<T>(ReadOnlySpan<T> vertices, int vertexCount = 0)
        where T : unmanaged, IMeshVertex
    {
        SetVertexAttributes<T>();
        _vertexBuffer.SetData(vertices, vertexCount);
    }

    public void GetVertices<T>(Span<T> vertices)
        where T : unmanaged, IMeshVertex
    {
        _vertexBuffer.GetData(vertices);
    }

    public void SetIndices(ReadOnlySpan<ushort> indices, int indexCount = 0)
    {
        _indexBuffer.SetData(indices, indexCount);
    }

    public void SetIndices(ReadOnlySpan<uint> indices, int indexCount = 0)
    {
        _indexBuffer.SetData(indices, indexCount);
    }

    public void GetIndices(Span<ushort> indices)
    {
        _indexBuffer.GetData(indices);
    }

    public void GetIndices(Span<uint> indices)
    {
        _indexBuffer.GetData(indices);
    }

    public void RecalculateBounds()
    {
        if (!_boundsDirty)
            return;

        Span<Vector3> vertexPositions = stackalloc Vector3[VertexCount];
        GetPositions(vertexPositions);
        _bounds = BoundingBox.CreateFromPoints(vertexPositions);
    }

    public bool RecalculateTangents()
    {
        MeshVertexAttribute? normalAttribute = GetVertexAttribute(MeshVertexAttributeSemantic.Normal);
        MeshVertexAttribute? texCoord0Attribute = GetVertexAttribute(MeshVertexAttributeSemantic.TexCoord0);
        MeshVertexAttribute? tangentAttribute = GetVertexAttribute(MeshVertexAttributeSemantic.Tangent);

        if (!normalAttribute.HasValue || !texCoord0Attribute.HasValue)
            return false;

        // TODO: Add tangent attribute if missing
        if (!tangentAttribute.HasValue)
            return false;

        return false;
    }

    public unsafe class CpuBuffer
    {
        protected byte* _data;

        /// <summary>
        /// Number of elements in the buffer.
        /// </summary>
        public int ElementCount { get; private set; }

        /// <summary>
        /// Size of a single element in the buffer.
        /// </summary>
        public int ElementSize { get; private set; }

        //public int MemorySize => ElementCount * ElementSize;
        public int MemorySize { get; private set; }

        public void Clear()
        {
            if (_data != null)
            {
                NativeMemory.Free(_data);
                _data = null;
            }

            ElementCount = 0;
            MemorySize = 0;
        }


        public void Resize(int elementCount, int elementSize)
        {
            ArgumentOutOfRangeException.ThrowIfNegativeOrZero(elementCount);
            ArgumentOutOfRangeException.ThrowIfNegativeOrZero(elementSize);

            if (ElementCount == elementCount
                && ElementSize == elementSize)
            {
                return;
            }

            ElementCount = elementCount;
            ElementSize = elementSize;

            int newSize = ElementCount * ElementSize;
            if (_data == null || MemorySize < newSize)
            {
                byte* newData = (byte*)NativeMemory.Alloc((uint)newSize);
                if (_data != null)
                {
                    NativeMemory.Copy(_data, newData, (uint)newSize);
                    NativeMemory.Free(_data);
                }

                _data = newData;
                MemorySize = newSize;
            }
        }

        public byte* GetRawData() => _data;

        public void SetData<T>(int index, int offset, T value)
            where T : unmanaged
        {
            Guard.IsEqualTo(sizeof(T), ElementSize, nameof(T));

            if (index >= ElementCount)
                return;

            int dataOffset = index * ElementSize + offset;
            NativeMemory.Copy(&value, &_data[dataOffset], (uint)sizeof(T));
        }

        public void SetElementData<T>(ReadOnlySpan<T> source, int offset)
            where T : unmanaged
        {
            for (int i = 0; i < source.Length; i++)
            {
                int dataOffset = i * ElementSize + offset;
                fixed (T* sourcePtr = &source[i])
                {
                    NativeMemory.Copy(sourcePtr, &_data[dataOffset], (uint)sizeof(T));
                }
            }
        }

        public void GetElementData<T>(Span<T> destination, int offset)
            where T : unmanaged
        {
            for (int i = 0; i < destination.Length; i++)
            {
                int dataOffset = i * ElementSize + offset;
                fixed (T* destinationPtr = &destination[i])
                {
                    NativeMemory.Copy(&_data[dataOffset], destinationPtr, (uint)sizeof(T));
                }
            }
        }
    }

    public unsafe class VertexBuffer : CpuBuffer
    {
        public void SetData<T>(ReadOnlySpan<T> source, int vertexCount)
            where T : unmanaged, IMeshVertex
        {
            if (vertexCount == 0)
                vertexCount = source.Length;

            Resize(vertexCount, sizeof(T));
            source.CopyTo(new Span<T>(_data, vertexCount));
        }

        public void GetData<T>(Span<T> destination)
            where T : unmanaged, IMeshVertex
        {
            Guard.IsTrue(sizeof(T) == ElementSize, nameof(T), "Size of T must match the element size of the vertex buffer.");
            new Span<T>(_data, destination.Length).CopyTo(destination);
        }
    }

    public unsafe class IndexBuffer : CpuBuffer
    {
        public IndexFormat IndexFormat => ElementSize == 4 ? IndexFormat.UInt32 : IndexFormat.UInt16;

        public void SetData(ReadOnlySpan<ushort> source, int indexCount = 0)
        {
            if (indexCount == 0)
                indexCount = source.Length;

            Resize(indexCount, 2);
            fixed (ushort* sourcePtr = source)
            {
                NativeMemory.Copy(sourcePtr, _data, (uint)(indexCount * 2));
            }
        }

        public void SetData(ReadOnlySpan<uint> source, int indexCount = 0)
        {
            if (indexCount == 0)
                indexCount = source.Length;

            Resize(indexCount, 4);
            fixed (uint* sourcePtr = source)
            {
                NativeMemory.Copy(sourcePtr, _data, (uint)(indexCount * 4));
            }
        }

        public void GetData(Span<ushort> data)
        {
            fixed (ushort* dataPtr = data)
            {
                switch (IndexFormat)
                {
                    case IndexFormat.UInt16:
                        NativeMemory.Copy(_data, dataPtr, (uint)(data.Length * sizeof(ushort)));
                        break;
                    case IndexFormat.UInt32:
                        for (int i = 0; i < data.Length; i++)
                        {
                            uint value = 0;
                            NativeMemory.Copy(&_data[i * 4], &value, 4u);
                            dataPtr[i] = (ushort)value;
                        }
                        break;
                }
            }
        }

        public void GetData(Span<uint> data)
        {
            fixed (uint* dataPtr = data)
            {
                switch (IndexFormat)
                {
                    case IndexFormat.UInt16:
                        for (int i = 0; i < data.Length; i++)
                        {
                            ushort value = 0;
                            NativeMemory.Copy(&_data[i * 2], &value, 2u);
                            dataPtr[i] = value;
                        }

                        break;
                    case IndexFormat.UInt32:
                        NativeMemory.Copy(_data, dataPtr, (uint)(data.Length * sizeof(uint)));
                        break;
                }
            }
        }
    }
}
