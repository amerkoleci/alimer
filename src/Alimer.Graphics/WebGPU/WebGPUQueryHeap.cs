// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUQueryHeap : QueryHeap
{
    private readonly WebGPUGraphicsDevice _device;

    public WebGPUQueryHeap(WebGPUGraphicsDevice device, in QueryHeapDescriptor description)
        : base(description)
    {
        _device = device;
        WGPUQuerySetDescriptor createInfo = new()
        {
            type = description.Type.ToWebGPU(),
            count = (uint)description.Count
        };

        if (description.Type == QueryType.PipelineStatistics)
        {
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.InputAssemblyVertices) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.InputAssemblyVertices;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.InputAssemblyPrimitives) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.InputAssemblyPrimitives;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.VertexShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.VertexShaderInvocations;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.GeometryShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.GeometryShaderInvocations;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.GeometryShaderPrimitives) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.GeometryShaderPrimitives;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.ClippingInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.ClippingInvocations;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.ClippingPrimitives) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.ClippingPrimitives;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.PixelShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.FragmentShaderInvocations;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.HullShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.TessellationControlShaderPatches;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.DomainShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.TessellationEvaluationShaderInvocations;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.ComputeShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.ComputeShaderInvocations;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.AmplificationShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.TaskShaderInvocationsEXT;
            //
            //if ((description.PipelineStatistics & QueryPipelineStatisticFlags.MeshShaderInvocations) != 0)
            //    createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.MeshShaderInvocationsEXT;
        }

        Handle = wgpuDeviceCreateQuerySet(device.Handle, &createInfo);
        if (Handle.IsNull)
        {
            Log.Error("WebGPU: Failed to create QueryHeap.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public WGPUQuerySet Handle { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUQueryHeap" /> class.
    /// </summary>
    ~WebGPUQueryHeap() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        wgpuQuerySetRelease(Handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuQuerySetSetLabel(Handle, newLabel);
    }
}
