// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers.Binary;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;
using XenoAtom.Collections;

namespace Alimer.Rendering;

// TODO: Mesh on GPU with CPU access

public sealed unsafe partial class Mesh : DisposableObject
{
    //private readonly UnsafeList<Vector3> _positions = [];
    private VertexAttribute[] _vertexAttributes;
    private readonly VertexBuffer _vertexBuffer;
    private readonly CpuBuffer _indexBuffer;
    private int _positionOffset = -1;
    private readonly List<SubMesh> _subMeshes = [];
    private BoundingBox _bounds;
    private bool _boundsDirty = true;

    //private GraphicsBuffer? _gpuPositionsBuffer;
    private GraphicsBuffer? _gpuVertexBuffer;
    private GraphicsBuffer? _gpuIndexBuffer;

    public Mesh(int vertexCount,
        Span<VertexAttribute> vertexAttributes,
        int indexCount,
        IndexFormat indexFormat,
        int vertexStride = 0)
    {
        _vertexAttributes = vertexAttributes.ToArray();
        // Compute stride from vertex attributes if required
        if (vertexStride == 0)
        {
            for (int i = 0; i < _vertexAttributes.Length; i++)
            {
                int elementSize = _vertexAttributes[i].Format.GetSizeInBytes();
                if (_vertexAttributes[i].Offset != 0)
                {
                    vertexStride = _vertexAttributes[i].Offset + elementSize;
                }
                else
                {
                    vertexStride += elementSize;
                }
            }
        }

        _vertexBuffer = new VertexBuffer(vertexCount, vertexStride);
        _indexBuffer = new CpuBuffer(indexCount, indexFormat == IndexFormat.UInt32 ? 4 : 2);
        _positionOffset = GetVertexAttributeOffset(VertexAttributeSemantic.Position);
    }

    /// <summary>
    /// The number of vertices in the vertex buffers.
    /// </summary>
    public int VertexCount => _vertexBuffer.ElementCount;

    /// <summary>
    /// Gets the size, in bytes, of a single vertex element in the layout.
    /// </summary>
    public int VertexStride => _vertexBuffer.ElementSize;

    /// <summary>
    /// Gets the number of indices contained in the index buffer.
    /// </summary>
    public int IndexCount => _indexBuffer.ElementCount;

    /// <summary>
    /// The data format of the indices that the index buffer stores.
    /// </summary>
    public IndexFormat IndexFormat => _indexBuffer.ElementSize == 4 ? IndexFormat.UInt32 : IndexFormat.UInt16;

    /// <summary>
    /// Gets the collection of sub-meshes that make up this mesh.
    /// </summary>
    public IEnumerable<SubMesh> SubMeshes => _subMeshes;

    /// <summary>
    /// Gets the bounds of the mesh.
    /// </summary>
    public BoundingBox Bounds => _bounds;

    public GraphicsBuffer? GpuVertexBuffer => _gpuVertexBuffer;
    public GraphicsBuffer? GpuIndexBuffer => _gpuIndexBuffer;

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

        // TODO: Separate positions
        ulong vertexBufferSize = (ulong)(_vertexBuffer.ElementCount * VertexStride);
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

    public bool HasVertexAttribute(VertexAttributeSemantic semantic)
    {
        for (int i = 0; i < _vertexAttributes.Length; i++)
        {
            if (_vertexAttributes[i].Semantic == semantic)
                return true;
        }

        return false;
    }

    public VertexAttribute? GetVertexAttribute(VertexAttributeSemantic semantic)
    {
        for (int i = 0; i < _vertexAttributes.Length; i++)
        {
            if (_vertexAttributes[i].Semantic == semantic)
                return _vertexAttributes[i];
        }

        return default;
    }

    public int GetVertexAttributeOffset(VertexAttributeSemantic semantic)
    {
        for (int i = 0; i < _vertexAttributes.Length; i++)
        {
            if (_vertexAttributes[i].Semantic == semantic)
                return _vertexAttributes[i].Offset;
        }

        return -1;
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
        GetAttributeData(VertexAttributeSemantic.Normal, normals);
    }

    public void GetTangents(Span<Vector3> tangents)
    {
        GetAttributeData(VertexAttributeSemantic.Tangent, tangents);
    }

    public unsafe void GetAttributeData<T>(VertexAttributeSemantic semantic, Span<T> data)
        where T : unmanaged
    {
        VertexAttribute? attribute = GetVertexAttribute(semantic) ?? throw new InvalidOperationException($"Mesh does not contain attribute with semantic {semantic}.");

        int offset = attribute.Value.Offset;
        int attributeSize = attribute.Value.Format.GetSizeInBytes();
        Guard.IsEqualTo(sizeof(T), attributeSize, nameof(T));

        _vertexBuffer.GetElementData(data, offset);
    }

    public unsafe void SetAttributeData<T>(VertexAttributeSemantic semantic, Span<T> data)
        where T : unmanaged
    {
        VertexAttribute? attribute = GetVertexAttribute(semantic) ?? throw new InvalidOperationException($"Mesh does not contain attribute with semantic {semantic}.");

        int offset = attribute.Value.Offset;
        int attributeSize = attribute.Value.Format.GetSizeInBytes();
        Guard.IsEqualTo(sizeof(T), attributeSize, nameof(T));

        _vertexBuffer.SetElementData(data, offset);
    }

    public void SetVertices<T>(ReadOnlySpan<T> source, int vertexCount = 0)
        where T : unmanaged
    {
        Guard.IsTrue(sizeof(T) == VertexStride, nameof(T), "Size of T must match the element size of the vertex buffer.");

        if (vertexCount == 0)
            vertexCount = _vertexBuffer.ElementCount;

        source.CopyTo(new Span<T>(_vertexBuffer.GetRawData(), vertexCount));
    }

    public void GetVertices<T>(Span<T> destination)
        where T : unmanaged
    {
        Guard.IsTrue(destination.Length == VertexCount);
        Guard.IsTrue(sizeof(T) == VertexStride, nameof(T), "Size of T must match the element size of the vertex buffer.");

        new Span<T>(_vertexBuffer.GetRawData(), destination.Length).CopyTo(destination);
    }

    public void SetIndices(ReadOnlySpan<ushort> source, int indexCount = 0)
    {
        Guard.IsTrue(IndexFormat == IndexFormat.UInt16, nameof(source), "Index buffer is not of type UInt16.");

        if (indexCount == 0)
            indexCount = source.Length;

        fixed (ushort* sourcePtr = source)
        {
            NativeMemory.Copy(sourcePtr, _indexBuffer.GetRawData(), (uint)(indexCount * 2));
        }
    }

    public void SetIndices(ReadOnlySpan<uint> source, int indexCount = 0)
    {
        Guard.IsTrue(IndexFormat == IndexFormat.UInt32, nameof(source), "Index buffer is not of type UInt32.");

        if (indexCount == 0)
            indexCount = source.Length;

        fixed (uint* sourcePtr = source)
        {
            NativeMemory.Copy(sourcePtr, _indexBuffer.GetRawData(), (uint)(indexCount * 4));
        }
    }

    public void GetIndices(Span<ushort> destination)
    {
        byte* sourcePtr = _indexBuffer.GetRawData();
        fixed (ushort* destinationPtr = destination)
        {
            switch (IndexFormat)
            {
                case IndexFormat.UInt16:
                    NativeMemory.Copy(sourcePtr, destinationPtr, (uint)(destination.Length * sizeof(ushort)));
                    break;
                case IndexFormat.UInt32:
                    for (int i = 0; i < destination.Length; i++)
                    {
                        uint value = 0;
                        NativeMemory.Copy(&sourcePtr[i * 4], &value, 4u);
                        destinationPtr[i] = (ushort)value;
                    }
                    break;
            }
        }
    }

    public void GetIndices(Span<uint> destination)
    {
        byte* sourcePtr = _indexBuffer.GetRawData();
        fixed (uint* destinationPtr = destination)
        {
            switch (IndexFormat)
            {
                case IndexFormat.UInt16:
                    for (int i = 0; i < destination.Length; i++)
                    {
                        ushort value = 0;
                        NativeMemory.Copy(&sourcePtr[i * 2], &value, 2u);
                        destinationPtr[i] = value;
                    }

                    break;
                case IndexFormat.UInt32:
                    NativeMemory.Copy(sourcePtr, destinationPtr, (uint)(destination.Length * sizeof(uint)));
                    break;
            }
        }
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
        VertexAttribute? normalAttribute = GetVertexAttribute(VertexAttributeSemantic.Normal);
        VertexAttribute? texCoord0Attribute = GetVertexAttribute(VertexAttributeSemantic.TexCoord0);
        VertexAttribute? tangentAttribute = GetVertexAttribute(VertexAttributeSemantic.Tangent);

        if (!normalAttribute.HasValue || !texCoord0Attribute.HasValue)
            return false;

        // TODO: Add tangent attribute if missing
        if (!tangentAttribute.HasValue)
            return false;

        Span<Vector3> vertexTangents = stackalloc Vector3[VertexCount];
        GetTangents(vertexTangents);

        return false;
    } 


    public enum VertexAttributeSemantic
    {
        Position,
        Normal,
        Tangent,
        Color,
        BlendIndices,
        BlendWeights,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        TexCoord4,
        TexCoord5,
        TexCoord6,
        TexCoord7,

        Custom
    }

    public record struct VertexAttribute(VertexAttributeSemantic Semantic, VertexFormat Format, int Offset);

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

        protected CpuBuffer()
        {
        }

        public CpuBuffer(int elementCount, int elementSize)
        {
            Resize(elementCount, elementSize);
        }

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
        public VertexBuffer(int vertexCount, int vertexStride)
            : base(vertexCount, vertexStride)
        {
        }

    }
}
