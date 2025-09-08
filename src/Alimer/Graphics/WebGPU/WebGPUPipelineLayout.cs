// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;
using static Alimer.Utilities.MemoryUtilities;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUPipelineLayout : PipelineLayout
{
    private readonly WebGPUGraphicsDevice _device;
    private readonly WGPUPushConstantRange* _pushConstantRanges;

    public WebGPUPipelineLayout(WebGPUGraphicsDevice device, in PipelineLayoutDescription description)
        : base(description)
    {
        _device = device;

        int bindGroupLayoutCount = description.BindGroupLayouts.Length;
        WGPUBindGroupLayout* bindGroupLayouts = stackalloc WGPUBindGroupLayout[bindGroupLayoutCount];

        for (int i = 0; i < bindGroupLayoutCount; i++)
        {
            bindGroupLayouts[i] = ((WebGPUBindGroupLayout)description.BindGroupLayouts[i]).Handle;
        }

        int pushConstantRangeCount = description.PushConstantRanges.Length;
        _pushConstantRanges = AllocateArray<WGPUPushConstantRange>((nuint)pushConstantRangeCount);
        {
            uint offset = 0;
            for (int i = 0; i < pushConstantRangeCount; i++)
            {
                _pushConstantRanges[i] = new WGPUPushConstantRange()
                {
                    stages = WGPUShaderStage.Vertex | WGPUShaderStage.Fragment | WGPUShaderStage.Compute,
                    start = offset,
                    end = description.PushConstantRanges[i].Size,
                };

                offset += _pushConstantRanges[i].end;
            }
        }

        WGPUPipelineLayoutDescriptor descriptor = new()
        {
            bindGroupLayoutCount = (nuint)bindGroupLayoutCount,
            bindGroupLayouts = bindGroupLayouts,
            //pushConstantRangeCount = (uint)pushConstantRangeCount,
            //pPushConstantRanges = _pushConstantRanges
        };

        Handle = wgpuDeviceCreatePipelineLayout(device.Handle, &descriptor);
        if (Handle.IsNull)
        {
            Log.Error($"WebGPU: Failed to create {nameof(PipelineLayout)}.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUPipelineLayout" /> class.
    /// </summary>
    ~WebGPUPipelineLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public WGPUPipelineLayout Handle { get; }

    public ref WGPUPushConstantRange GetPushConstantRange(uint index) => ref _pushConstantRanges[index];

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuPipelineLayoutSetLabel(Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_pushConstantRanges);
        wgpuPipelineLayoutRelease(Handle);
    }
}
