// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Graphics;
using static Alimer.Utilities.UnsafeUtilities;

using BindlessMaterialBufferIndex = System.Int32;
namespace Alimer.Rendering;

using InstanceDictionary = Dictionary<BindlessMaterialBufferIndex, GPUBatchEntry>;
//using GeometryDictionary = Dictionary<SubMesh, InstanceDictionary>;
//using PipelineMaterials = Dictionary<GPURenderPipeline, MaterialGeometry>;

public sealed unsafe class RenderBatch : DisposableObject
{
    private static uint InstanceSizeInBytes = SizeOf<GPUInstance>();
    private const uint InitialInstanceCount = 128;

    private GpuBuffer[] _instanceBuffer;
    private BindGroup[] _instanceBindGroup;
    private uint _instanceCapacity = 0;
    private uint _totalInstanceCount = 0;

    // TODO: Fix this
    private Dictionary<GPURenderPipeline, Dictionary<SubMesh, InstanceDictionary>> _pipelineGeometries = [];
    private readonly List<GPUInstance> _instanceData = [];

    public RenderBatch(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        _instanceBuffer = new GpuBuffer[device.MaxFramesInFlight];
        _instanceBindGroup = new BindGroup[device.MaxFramesInFlight];
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

        for (int i = 0; i < _instanceBuffer.Length; i++)
        {
            _instanceBuffer[i]?.Dispose();

            _instanceBuffer[i] = ToDispose(Device.CreateBuffer(
                InstanceSizeInBytes * capacity,
                BufferUsage.ShaderRead,
                MemoryType.Upload,
                label: $"Upload Instance Buffer Frame {i}"
                ));
            _instanceBindGroup[i] = InstanceBindGroupLayout.CreateBindGroup(
                new BindGroupEntry(0, _instanceBuffer[i], stride: InstanceSizeInBytes)
            );
        }
    }

    public void AddRenderable(SubMesh geometry, GPURenderPipeline pipeline, BindlessMaterialBufferIndex materialBufferIndex, GPUInstanceData instance)
    {
        int instanceBufferIndex = _instanceBuffer[Device.FrameIndex].BindlessShaderWriteIndex;

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

        if (!materialInstances.TryGetValue(materialBufferIndex, out GPUBatchEntry? instances))
        {
            instances = new GPUBatchEntry(instanceBufferIndex, _instanceData.Count);
            materialInstances[materialBufferIndex] = instances;
        }

        instances.InstanceCount++;
        instances.Transforms.Add(instance.WorldMatrix);
        _instanceData.Add(new GPUInstance
        {
            worldMatrix = instance.WorldMatrix,
            MaterialIndex = instance.MaterialIndex
        });
        //instances.Colors.Add(instance.Color);
        _totalInstanceCount += 1;
    }

    public BindGroup UpdateInstanceBuffer(uint frameIndex)
    {
        // Only upload if we have instance data
        if (_instanceData.Count > 0)
        {
            var instanceData = CollectionsMarshal.AsSpan(_instanceData);
            _instanceBuffer[frameIndex].SetData(instanceData);
        }

        return _instanceBindGroup[frameIndex];
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

public record struct GPUInstanceData(Matrix4x4 WorldMatrix, int MaterialIndex);

public class GPUBatchEntry
{
    public GPUBatchEntry(int instanceBufferIndex, int firstInstance = 0)
    {
        InstanceBufferIndex = instanceBufferIndex;
        FirstInstance = (uint)firstInstance;
    }

    public readonly int InstanceBufferIndex;
    public readonly uint FirstInstance;
    public uint InstanceCount;
    public readonly List<Matrix4x4> Transforms = [];
    //public readonly List<Color> Colors = [];
    public int BufferOffset;
}
