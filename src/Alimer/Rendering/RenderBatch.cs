// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Engine;
using Alimer.Graphics;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Rendering;

using InstanceDictionary = Dictionary</*GPUMaterialBindGroups*/BindGroup, GPUBatchEntry>;
//using GeometryDictionary = Dictionary<SubMesh, InstanceDictionary>;
//using PipelineMaterials = Dictionary<GPURenderPipeline, MaterialGeometry>;

public sealed unsafe class RenderBatch : DisposableObject
{
    private const uint InitialInstanceCount = 128;

    private static uint InstanceSizeInBytes = SizeOf<InstanceData>(); // 4x float (16) + 1x float4 (color)

    private sealed class PerFrame
    {
        public GraphicsBuffer? InstanceBuffer;
        public BindGroup? InstanceBindGroup;
    }

    private PerFrame[]? _perFrameData;  
    private uint _instanceCapacity = 0;
    private uint _totalInstanceCount = 0;

    // TODO: Fix this
    private Dictionary<GPURenderPipeline, Dictionary<SubMesh, InstanceDictionary>> _pipelineGeometries = [];
    private readonly List<InstanceData> _instanceData = [];

    public RenderBatch(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        _perFrameData = new PerFrame[(int)device.MaxFramesInFlight];
        for (int i = 0; i < _perFrameData.Length; i++)
        {
            _perFrameData[i] = new PerFrame();
        }

        InstanceBindGroupLayout = ToDispose(device.CreateBindGroupLayout(
            "Instance bind group layout",
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.ShaderRead), 0, ShaderStages.Vertex)
        ));
        ResizeInstanceBuffer(InitialInstanceCount);
    }

    public GraphicsDevice Device { get; }
    public BindGroupLayout InstanceBindGroupLayout { get; }

    public void ResizeInstanceBuffer(uint capacity)
    {
        _instanceCapacity = capacity;

        for (int i = 0; i < _perFrameData!.Length; i++)
        {
            _perFrameData[i].InstanceBuffer?.Dispose();

            _perFrameData[i].InstanceBuffer = ToDispose(Device.CreateBuffer(
                InstanceSizeInBytes * capacity,
                BufferUsage.ShaderRead,
                MemoryType.Upload,
                label: $"Upload Instance Buffer Frame {i}"
                ));
            _perFrameData[i].InstanceBindGroup = InstanceBindGroupLayout.CreateBindGroup(
                new BindGroupEntry(0, _perFrameData[i].InstanceBuffer!, stride: (uint)sizeof(InstanceData))
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
        _instanceData.Add(new InstanceData
        {
            worldMatrix = instance.WorldMatrix,
            //materialIndex = instance.MaterialIndex
        });
        //instances.Colors.Add(instance.Color);
        _totalInstanceCount += 1;
    }

    public BindGroup UpdateInstanceBuffer(uint frameIndex)
    {
        PerFrame frame = _perFrameData![frameIndex];

        // Only upload if we have instance data
        if (_instanceData.Count > 0)            
        {
            var instanceData = CollectionsMarshal.AsSpan(_instanceData);
            frame.InstanceBuffer!.SetData(instanceData);
        }

        return frame.InstanceBindGroup!;
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
    }

    public IEnumerable<GPURenderPipeline> SortedPipelines
    {
        get => _pipelineGeometries.Keys.OrderBy(item => item.RenderOrder);
    }
}

public record struct GPUInstanceData(Matrix4x4 WorldMatrix, uint MaterialIndex, uint Count = 1);

public class GPUBatchEntry
{
    public GPUBatchEntry(uint firstInstance = 0)
    {
        FirstInstance = firstInstance;
    }

    public readonly uint FirstInstance;
    public uint InstanceCount;
    public readonly List<Matrix4x4> Transforms = [];
    //public readonly List<Color> Colors = [];
    public int BufferOffset;
}
