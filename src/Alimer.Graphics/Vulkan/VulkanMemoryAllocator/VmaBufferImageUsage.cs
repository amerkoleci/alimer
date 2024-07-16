// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

internal readonly struct VmaBufferImageUsage : IEquatable<VmaBufferImageUsage>, IFormattable
{
    public readonly ulong Value;

    public VmaBufferImageUsage(ulong value)
    {
        Value = value;
    }

    public unsafe VmaBufferImageUsage(in VkBufferCreateInfo createInfo, bool useKhrMaintenance5)
    {
        if (useKhrMaintenance5)
        {
            // If VkBufferCreateInfo::pNext chain contains VkBufferUsageFlags2CreateInfoKHR,
            // take usage from it and ignore VkBufferCreateInfo::usage, per specification
            // of the VK_KHR_maintenance5 extension.
            VkBufferUsageFlags2CreateInfoKHR* usageFlags2 = VmaPnextChainFind<VkBufferUsageFlags2CreateInfoKHR>(createInfo, VkStructureType.BufferUsageFlags2CreateInfoKHR);
            if (usageFlags2 != null)
            {
                Value = (ulong)usageFlags2->usage;
                return;
            }
        }

        Value = (ulong)createInfo.usage;
    }

    public VmaBufferImageUsage(in VkImageCreateInfo createInfo)
    {
        // Maybe in the future there will be VK_KHR_maintenanceN extension with structure
        // VkImageUsageFlags2CreateInfoKHR, like the one for buffers...

        Value = (ulong)createInfo.usage;
    }

    public static VmaBufferImageUsage Unknown => new(0);

    public readonly bool Contains(ulong flag) => (Value & flag) != 0;

    public bool ContainsDeviceAccess
    {
        get
        {
            ulong flags = (ulong)(VkBufferUsageFlags.TransferDst | VkBufferUsageFlags.TransferSrc);
            // This relies on values of VK_IMAGE_USAGE_TRANSFER* being the same as VK_BUFFER_IMAGE_TRANSFER*.
            return (Value & ~flags) != 0;
        }
    }

    public static bool operator ==(VmaBufferImageUsage left, VmaBufferImageUsage right) => left.Value == right.Value;

    public static bool operator !=(VmaBufferImageUsage left, VmaBufferImageUsage right) => left.Value != right.Value;

    public override bool Equals(object? obj) => (obj is VmaBufferImageUsage other) && Equals(other);

    public bool Equals(VmaBufferImageUsage other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value.ToString();

    public string ToString(string? format, IFormatProvider? formatProvider) => Value.ToString(format, formatProvider);
}

