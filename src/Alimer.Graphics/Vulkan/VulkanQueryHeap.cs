// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanQueryHeap : QueryHeap
{
    private readonly VkQueryPool _handle = VkQueryPool.Null;

    public VulkanQueryHeap(VulkanGraphicsDevice device, in QueryHeapDescription description)
        : base(device, description)
    {
        VkQueryPoolCreateInfo createInfo = new()
        {
            queryType = description.Type.ToVk(),
            queryCount = (uint)description.Count
        };

        if (description.Type == QueryType.PipelineStatistics)
        {
            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.InputAssemblyVertices) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.InputAssemblyVertices;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.InputAssemblyPrimitives) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.InputAssemblyPrimitives;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.VertexShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.VertexShaderInvocations;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.GeometryShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.GeometryShaderInvocations;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.GeometryShaderPrimitives) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.GeometryShaderPrimitives;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.ClippingInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.ClippingInvocations;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.ClippingPrimitives) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.ClippingPrimitives;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.PixelShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.FragmentShaderInvocations;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.HullShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.TessellationControlShaderPatches;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.DomainShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.TessellationEvaluationShaderInvocations;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.ComputeShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.ComputeShaderInvocations;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.AmplificationShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.TaskShaderInvocationsEXT;

            if ((description.PipelineStatistics & QueryPipelineStatisticFlags.MeshShaderInvocations) != 0)
                createInfo.pipelineStatistics |= VkQueryPipelineStatisticFlags.MeshShaderInvocationsEXT;
        }

        VkResult result = vkCreateQueryPool(device.Handle, &createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create QueryHeap.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    public VkDevice VkDevice => ((VulkanGraphicsDevice)Device).Handle;
    public VkQueryPool Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanQueryHeap" /> class.
    /// </summary>
    ~VulkanQueryHeap() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyQueryPool(VkDevice, _handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        ((VulkanGraphicsDevice)Device).SetObjectName(VkObjectType.QueryPool, _handle.Handle, newLabel);
    }
}
