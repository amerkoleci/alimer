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
        bool samplerMirrorClampToEdge = device.PhysicalDeviceFeatures1_2.samplerMirrorClampToEdge;

        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
        VkSamplerCreateInfo createInfo = new()
        {
            flags = 0,
            pNext = null,
            magFilter = description.MagFilter.ToVk(),
            minFilter = description.MinFilter.ToVk(),
            mipmapMode = description.MipFilter.ToVk(),
            addressModeU = description.AddressModeU.ToVk(samplerMirrorClampToEdge),
            addressModeV = description.AddressModeV.ToVk(samplerMirrorClampToEdge),
            addressModeW = description.AddressModeW.ToVk(samplerMirrorClampToEdge),
            mipLodBias = 0.0f,
        };

        ushort maxAnisotropy = description.MaxAnisotropy;
        if (maxAnisotropy > 1 && device.PhysicalDeviceFeatures2.features.samplerAnisotropy)
        {
            createInfo.anisotropyEnable = true;
            createInfo.maxAnisotropy = Math.Min((float)maxAnisotropy, device.PhysicalDeviceProperties.properties.limits.maxSamplerAnisotropy);
        }
        else
        {
            createInfo.anisotropyEnable = false;
            createInfo.maxAnisotropy = 1;
        }

        if (description.ReductionType == SamplerReductionType.Comparison)
        {
            createInfo.compareOp = description.CompareFunction.ToVk();
            createInfo.compareEnable = true;
        }
        else
        {
            createInfo.compareOp = VkCompareOp.Never;
            createInfo.compareEnable = false;
        }

        createInfo.minLod = description.MinLod;
        createInfo.maxLod = description.MaxLod;
        createInfo.borderColor = description.BorderColor.ToVk();
        createInfo.unnormalizedCoordinates = false;

        VkSamplerReductionModeCreateInfo samplerReductionModeInfo = default;
        if (description.ReductionType == SamplerReductionType.Minimum ||
            description.ReductionType == SamplerReductionType.Maximum)
        {
            samplerReductionModeInfo = new()
            {
                reductionMode = description.ReductionType == SamplerReductionType.Maximum ? VkSamplerReductionMode.Max: VkSamplerReductionMode.Min
            };

            createInfo.pNext = &samplerReductionModeInfo;
        }

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
