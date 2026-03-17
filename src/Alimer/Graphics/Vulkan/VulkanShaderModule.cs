// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Utilities.MemoryUtilities;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanShaderModule : ShaderModule
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkShaderModule _handle = VkShaderModule.Null;
    private readonly VkPipelineShaderStageCreateInfo _stageInfo;
    private readonly byte* _pEntryPoint;

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

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        ReadOnlySpan<byte> entryPointName = EntryPoint.GetUtf8Span();
        int entryPointNameLength = entryPointName.Length + 1;

        _pEntryPoint = AllocateArray<byte>((uint)entryPointNameLength);
        var destination = new Span<byte>(_pEntryPoint, entryPointNameLength);

        entryPointName.CopyTo(destination);
        destination[entryPointName.Length] = 0x00;

        _stageInfo = new VkPipelineShaderStageCreateInfo
        {
            stage = descriptor.Stage.ToVk(),
            module = _handle,
            pName = _pEntryPoint
        };
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public VkShaderModule Handle => _handle;
    public ref readonly VkPipelineShaderStageCreateInfo StageInfo => ref _stageInfo;

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_pEntryPoint);
        _device.DeviceApi.vkDestroyShaderModule(_handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VK_OBJECT_TYPE_SHADER_MODULE, Handle, newLabel);
    }
}
