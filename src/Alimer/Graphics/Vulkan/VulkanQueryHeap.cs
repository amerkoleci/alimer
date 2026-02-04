// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanQueryHeap : QueryHeap
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkQueryPool _handle = VkQueryPool.Null;

    public VulkanQueryHeap(VulkanGraphicsDevice device, in QueryHeapDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        VkQueryPoolCreateInfo createInfo = new()
        {
            queryType = descriptor.Type.ToVk(),
            queryCount = descriptor.Count
        };

        if (descriptor.Type == QueryType.PipelineStatistics)
        {
            VkQueryPipelineStatisticFlags pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
                | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
                | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

            // TODO:
            if (_device.VkAdapter.Extensions.MeshShader)
            {
                //pipelineStatistics |= VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;
            }

            createInfo.pipelineStatistics = pipelineStatistics;
        }

        VkResult result = _device.DeviceApi.vkCreateQueryPool(in createInfo, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create QueryHeap.");
            return;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        QueryResultSize = (uint)((descriptor.Type == QueryType.PipelineStatistics ? (_device.VkAdapter.Extensions.MeshShader ? 13 : 11) : 1) * sizeof(ulong));
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override uint QueryResultSize { get; }

    public VkQueryPool Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanQueryHeap" /> class.
    /// </summary>
    ~VulkanQueryHeap() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _device.DeviceApi.vkDestroyQueryPool(_handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VkObjectType.QueryPool, _handle, newLabel);
    }
}
