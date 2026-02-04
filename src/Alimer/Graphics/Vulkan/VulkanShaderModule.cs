// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanShaderModule : ShaderModule
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkShaderModule _handle = VkShaderModule.Null;
    private readonly VkPipelineShaderStageCreateInfo _stageInfo;

    public VulkanShaderModule(VulkanGraphicsDevice device, in ShaderModuleDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;

        VkShaderModuleCreateInfo createInfo = new();
        fixed (byte* codePtr = descriptor.ByteCode)
        {
            createInfo.codeSize = (UIntPtr)descriptor.ByteCode.Length;
            createInfo.pCode = (uint*)codePtr;

            _device.DeviceApi.vkCreateShaderModule(descriptor.ByteCode, null, out _handle).CheckResult();
        }

        //if (!string.IsNullOrEmpty(descriptor.Label))
        //{
        //    OnLabelChanged(descriptor.Label!);
        //}

        _stageInfo = new VkPipelineShaderStageCreateInfo
        {
            stage = descriptor.Stage.ToVk(),
            module = _handle,
            pName = (byte*)EntryPoint,
        };
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public VkShaderModule Handle => _handle;
    public ref readonly VkPipelineShaderStageCreateInfo StageInfo => ref _stageInfo;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanShaderModule" /> class.
    /// </summary>
    ~VulkanShaderModule() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _device.DeviceApi.vkDestroyShaderModule(_handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VK_OBJECT_TYPE_SHADER_MODULE, Handle, newLabel);
    }
}
