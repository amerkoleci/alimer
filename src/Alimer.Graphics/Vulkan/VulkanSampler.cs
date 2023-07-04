// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanSampler : Sampler
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkSampler _handle = VkSampler.Null;

    public VulkanSampler(VulkanGraphicsDevice device, in SamplerDescription description)
        : base(description)
    {
        _device = device;

        VkSamplerCreateInfo createInfo = new()
        {
        };

        VkResult result = vkCreateSampler(device.Handle, &createInfo, null, out _handle);

        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create sampler.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public VkSampler Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanSampler" /> class.
    /// </summary>
    ~VulkanSampler() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroySampler(_device.Handle, _handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.Sampler, _handle.Handle, newLabel);
    }
}
