// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Linq;
using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

using InstanceDictionary = Dictionary</*GPUMaterialBindGroups*/BindGroup, GPUBatchEntry>;
//using GeometryDictionary = Dictionary<SubMesh, InstanceDictionary>;

public sealed unsafe class RenderBatch : DisposableObject
{
    private const uint InitialInstanceCount = 128;

    private const uint InstanceSizeInBytes = 20; // 4x float (16) + 1x float4 (color)

    private GraphicsBuffer? _instanceBuffer;
    private bool _instanceBufferDirty = true;
    private uint _instanceCapacity = 0;
    private uint _totalInstanceCount = 0;

    // TODO: Fix this
    private Dictionary<GPUMaterialPipeline, Dictionary<SubMesh, InstanceDictionary>> _pipelineGeometries = [];

    public RenderBatch(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        ResizeInstanceBuffer(InitialInstanceCount);
    }

    public GraphicsDevice Device { get; }

    public void ResizeInstanceBuffer(uint capacity)
    {
        _instanceBuffer?.Dispose();

        _instanceBufferDirty = true;
        _instanceCapacity = capacity;
        _instanceBuffer = ToDispose(Device.CreateBuffer(
            InstanceSizeInBytes * capacity,
            BufferUsage.Vertex,
            MemoryType.Upload,
            label: "Upload Instance Buffer"
            ));
    }

    public void AddRenderable(SubMesh geometry, GPUMaterialPipeline pipeline, /*GPUMaterialBindGroups*/BindGroup bindGroups, GPUInstanceData instance)
    {
        if (!_pipelineGeometries.TryGetValue(pipeline, out Dictionary<SubMesh, InstanceDictionary>? geometryMaterials))
        {
            // Create new entry
            geometryMaterials = [];
            _pipelineGeometries[pipeline] = geometryMaterials;
        }

        if (!geometryMaterials.TryGetValue(geometry, out InstanceDictionary? materialInstances))
        {
            materialInstances = [];
            geometryMaterials[geometry] = materialInstances;
        }

        if (!materialInstances.TryGetValue(bindGroups, out GPUBatchEntry? instances))
        {
            instances = new GPUBatchEntry(0, [], [], 0);
            materialInstances[bindGroups] = instances;
        }

        //instances.InstanceCount += instance.Count;
        instances.Transforms.Add(instance.WorldMatrix);
        instances.colors.Add(instance.Color);
        _totalInstanceCount += 1;
    }

    public Dictionary<SubMesh, InstanceDictionary> GetGeometryList(GPUMaterialPipeline pipeline)
    {
        return _pipelineGeometries[pipeline];
    }

    public void Clear()
    {
        _pipelineGeometries.Clear();
        _totalInstanceCount = 0;
        _instanceBufferDirty = true;
    }

    public IEnumerable<GPUMaterialPipeline> SortedPipelines
    {
        get => _pipelineGeometries.Keys.OrderBy(item => item.RenderOrder);
    }
}

public record struct GPUInstanceData(Matrix4x4 WorldMatrix, Color Color, uint Count = 1);

public record class GPUBatchEntry(uint InstanceCount, List<Matrix4x4> Transforms, List<Color> colors, int BufferOffset = 0);
