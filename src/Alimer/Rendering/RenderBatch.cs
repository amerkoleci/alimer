// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

using InstanceDictionary = Dictionary</*GPUMaterialBindGroups*/BindGroup, GPUBatchEntry>;
//using GeometryDictionary = Dictionary<SubMesh, InstanceDictionary>;
//using PipelineMaterials = Dictionary<GPURenderPipeline, MaterialGeometry>;

public sealed unsafe class RenderBatch : DisposableObject
{
    private const uint InitialInstanceCount = 128;

    private static uint InstanceSizeInBytes = (uint)sizeof(InstanceData); // 4x float (16) + 1x float4 (color)

    private GraphicsBuffer[]? _instanceBuffers;
    private BindGroup[]? _instanceBindGroups;
    private bool _instanceBufferDirty = true;
    private uint _instanceCapacity = 0;
    private uint _totalInstanceCount = 0;

    // TODO: Fix this
    private Dictionary<GPURenderPipeline, Dictionary<SubMesh, InstanceDictionary>> _pipelineGeometries = [];
    private readonly List<InstanceData> _instanceData = [];

    public RenderBatch(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        _instanceBuffers = new GraphicsBuffer[(int)device.MaxFramesInFlight];
        _instanceBindGroups = new BindGroup[(int)device.MaxFramesInFlight];

        InstanceBindGroupLayout = ToDispose(device.CreateBindGroupLayout(
            "Instance bind group layout",
                new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.ReadOnlyStorage), 0, ShaderStages.Vertex)
        ));
        ResizeInstanceBuffer(InitialInstanceCount);
    }

    public GraphicsDevice Device { get; }
    public BindGroupLayout InstanceBindGroupLayout { get; }

    public void ResizeInstanceBuffer(uint capacity)
    {
        _instanceBufferDirty = true;
        _instanceCapacity = capacity;

        for (int i = 0; i < _instanceBuffers!.Length; i++)
        {
            _instanceBuffers[i]?.Dispose();

            _instanceBuffers[i] = ToDispose(Device.CreateBuffer(
                InstanceSizeInBytes * capacity,
                BufferUsage.ShaderRead,
                MemoryType.Upload,
                label: $"Upload Instance Buffer Frame {i}"
                ));
            _instanceBindGroups![i] = InstanceBindGroupLayout.CreateBindGroup(
                new BindGroupEntry(0, _instanceBuffers[i])
            );
        }
    }

    public void AddRenderable(SubMesh geometry, GPURenderPipeline pipeline, /*GPUMaterialBindGroups*/BindGroup bindGroups, GPUInstanceData instance)
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
            instances = new GPUBatchEntry((uint)_instanceData.Count);
            materialInstances[bindGroups] = instances;
        }

        instances.InstanceCount += instance.Count;
        instances.Transforms.Add(instance.WorldMatrix);
        _instanceData.Add(new InstanceData {
            worldMatrix = instance.WorldMatrix,
            color = instance.Color,
            materialIndex = 0 // TODO
        });
        instances.Colors.Add(instance.Color);
        _totalInstanceCount += 1;
    }

    public BindGroup UpdateInstanceBuffer(uint frameIndex)
    {
        if (!_instanceBufferDirty)
        {
            return _instanceBindGroups![frameIndex];
        }

        var instanceData = CollectionsMarshal.AsSpan(_instanceData);
        _instanceBuffers![frameIndex].SetData(instanceData);
        _instanceBufferDirty = false;

        return _instanceBindGroups![frameIndex];
    }

    public Dictionary<SubMesh, InstanceDictionary> GetGeometryList(GPURenderPipeline pipeline)
    {
        return _pipelineGeometries[pipeline];
    }

    public void Clear()
    {
        _pipelineGeometries.Clear();
        _totalInstanceCount = 0;
        _instanceData.Clear();
        _instanceBufferDirty = true;
    }

    public IEnumerable<GPURenderPipeline> SortedPipelines
    {
        get => _pipelineGeometries.Keys.OrderBy(item => item.RenderOrder);
    }
}

public record struct GPUInstanceData(Matrix4x4 WorldMatrix, Color Color, uint Count = 1);

public class GPUBatchEntry
{
    public GPUBatchEntry(uint firstInstance = 0)
    {
        FirstInstance = firstInstance;
    }

    public readonly uint FirstInstance;
    public uint InstanceCount;
    public readonly List<Matrix4x4> Transforms = [];
    public readonly List<Color> Colors = [];
    public int BufferOffset;
}
