// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

#pragma warning disable CS0649

namespace Alimer.Graphics.Vulkan;


[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly partial struct VmaAllocator(nint handle) : IEquatable<VmaAllocator>
{
    public nint Handle { get; } = handle; public bool IsNull => Handle == 0;
    public bool IsNotNull => Handle != 0;
    public static VmaAllocator Null => new(0);
    public static implicit operator VmaAllocator(nint handle) => new(handle);
    public static implicit operator nint(VmaAllocator handle) => handle.Handle;
    public static bool operator ==(VmaAllocator left, VmaAllocator right) => left.Handle == right.Handle;
    public static bool operator !=(VmaAllocator left, VmaAllocator right) => left.Handle != right.Handle;
    public static bool operator ==(VmaAllocator left, nint right) => left.Handle == right;
    public static bool operator !=(VmaAllocator left, nint right) => left.Handle != right;
    public bool Equals(VmaAllocator other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is VmaAllocator handle && Equals(handle);
    /// <inheritdoc/>
    public override int GetHashCode() => Handle.GetHashCode();
    private string DebuggerDisplay => $"{nameof(VmaAllocator)} [0x{Handle:X}]";
}

[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly partial struct VmaAllocation(nint handle) : IEquatable<VmaAllocation>
{
    public nint Handle { get; } = handle; public bool IsNull => Handle == 0;
    public bool IsNotNull => Handle != 0;
    public static VmaAllocation Null => new(0);
    public static implicit operator VmaAllocation(nint handle) => new(handle);
    public static implicit operator nint(VmaAllocation handle) => handle.Handle;
    public static bool operator ==(VmaAllocation left, VmaAllocation right) => left.Handle == right.Handle;
    public static bool operator !=(VmaAllocation left, VmaAllocation right) => left.Handle != right.Handle;
    public static bool operator ==(VmaAllocation left, nint right) => left.Handle == right;
    public static bool operator !=(VmaAllocation left, nint right) => left.Handle != right;
    public bool Equals(VmaAllocation other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is VmaAllocation handle && Equals(handle);
    /// <inheritdoc/>
    public override int GetHashCode() => Handle.GetHashCode();
    private string DebuggerDisplay => $"{nameof(VmaAllocation)} [0x{Handle:X}]";
}

[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly partial struct VmaPool(nint handle) : IEquatable<VmaPool>
{
    public nint Handle { get; } = handle; public bool IsNull => Handle == 0;
    public bool IsNotNull => Handle != 0;
    public static VmaPool Null => new(0);
    public static implicit operator VmaPool(nint handle) => new(handle);
    public static implicit operator nint(VmaPool handle) => handle.Handle;
    public static bool operator ==(VmaPool left, VmaPool right) => left.Handle == right.Handle;
    public static bool operator !=(VmaPool left, VmaPool right) => left.Handle != right.Handle;
    public static bool operator ==(VmaPool left, nint right) => left.Handle == right;
    public static bool operator !=(VmaPool left, nint right) => left.Handle != right;
    public bool Equals(VmaPool other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is VmaPool handle && Equals(handle);
    /// <inheritdoc/>
    public override int GetHashCode() => Handle.GetHashCode();
    private string DebuggerDisplay => $"{nameof(VmaPool)} [0x{Handle:X}]";
}

[Flags]
internal enum VmaAllocatorCreateFlags
{
    None = 0,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT</unmanaged>
    ExternallySynchronized = 0x00000001,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT</unmanaged>
    KHRDedicatedAllocation = 0x00000002,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT</unmanaged>
    KHRBindMemory2 = 0x00000004,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT</unmanaged>
    EXTMemoryBudget = 0x00000008,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT</unmanaged>
    AMDDeviceCoherentMemory = 0x00000010,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT</unmanaged>
    BufferDeviceAddress = 0x00000020,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT</unmanaged>
    EXTMemoryPriority = 0x00000040,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT</unmanaged>
    KHRMaintenance4 = 0x00000080,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT</unmanaged>
    KHRMaintenance5 = 0x00000100,
    /// <unmanaged>VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT</unmanaged>
    KHRExternalMemoryWin32 = 0x00000200,
}

internal enum VmaMemoryUsage
{
    VMA_MEMORY_USAGE_UNKNOWN = 0,
    VMA_MEMORY_USAGE_GPU_ONLY = 1,
    VMA_MEMORY_USAGE_CPU_ONLY = 2,
    VMA_MEMORY_USAGE_CPU_TO_GPU = 3,
    VMA_MEMORY_USAGE_GPU_TO_CPU = 4,
    VMA_MEMORY_USAGE_CPU_COPY = 5,
    VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED = 6,
    VMA_MEMORY_USAGE_AUTO = 7,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE = 8,
    VMA_MEMORY_USAGE_AUTO_PREFER_HOST = 9,
}

[Flags]
internal enum VmaAllocationCreateFlags
{
    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT = 0x00000001,
    VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT = 0x00000002,
    VMA_ALLOCATION_CREATE_MAPPED_BIT = 0x00000004,
    VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT = 0x00000020,
    VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT = 0x00000040,
    VMA_ALLOCATION_CREATE_DONT_BIND_BIT = 0x00000080,
    VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT = 0x00000100,
    VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT = 0x00000200,
    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT = 0x00000400,
    VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT = 0x00000800,
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT = 0x00001000,
    VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT = 0x00010000,
    VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT = 0x00020000,
    VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT = 0x00040000,
    VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
    VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
    VMA_ALLOCATION_CREATE_STRATEGY_MASK = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
        | VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT |
        VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,
}

internal unsafe partial struct VmaDeviceMemoryCallbacks
{
    public delegate* unmanaged<VmaAllocator, uint, VkDeviceMemory, ulong, void*, void> pfnAllocate;
    public delegate* unmanaged<VmaAllocator, uint, VkDeviceMemory, ulong, void*, void> pfnFree;
    public void* pUserData;
}

internal unsafe partial struct VmaVulkanFunctions
{
    public delegate* unmanaged<VkInstance, byte*, PFN_vkVoidFunction> vkGetInstanceProcAddr;
    public delegate* unmanaged<VkDevice, byte*, PFN_vkVoidFunction> vkGetDeviceProcAddr;
    public delegate* unmanaged<VkPhysicalDevice, VkPhysicalDeviceProperties*, void> vkGetPhysicalDeviceProperties;
    public delegate* unmanaged<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties*, void> vkGetPhysicalDeviceMemoryProperties;
    public delegate* unmanaged<VkDevice, VkMemoryAllocateInfo*, VkAllocationCallbacks*, VkDeviceMemory*, VkResult> vkAllocateMemory;
    public delegate* unmanaged<VkDevice, VkDeviceMemory, VkAllocationCallbacks*, void> vkFreeMemory;
    public delegate* unmanaged<VkDevice, VkDeviceMemory, ulong, ulong, VkMemoryMapFlags, void**, VkResult> vkMapMemory;
    public delegate* unmanaged<VkDevice, VkDeviceMemory, void> vkUnmapMemory;
    public delegate* unmanaged<VkDevice, uint, VkMappedMemoryRange*, VkResult> vkFlushMappedMemoryRanges;
    public delegate* unmanaged<VkDevice, uint, VkMappedMemoryRange*, VkResult> vkInvalidateMappedMemoryRanges;
    public delegate* unmanaged<VkDevice, VkBuffer, VkDeviceMemory, ulong, VkResult> vkBindBufferMemory;
    public delegate* unmanaged<VkDevice, VkImage, VkDeviceMemory, ulong, VkResult> vkBindImageMemory;
    public delegate* unmanaged<VkDevice, VkBuffer, VkMemoryRequirements*, void> vkGetBufferMemoryRequirements;
    public delegate* unmanaged<VkDevice, VkImage, VkMemoryRequirements*, void> vkGetImageMemoryRequirements;
    public delegate* unmanaged<VkDevice, VkBufferCreateInfo*, VkAllocationCallbacks*, VkBuffer*, VkResult> vkCreateBuffer;
    public delegate* unmanaged<VkDevice, VkBuffer, VkAllocationCallbacks*, void> vkDestroyBuffer;
    public delegate* unmanaged<VkDevice, VkImageCreateInfo*, VkAllocationCallbacks*, VkImage*, VkResult> vkCreateImage;
    public delegate* unmanaged<VkDevice, VkImage, VkAllocationCallbacks*, void> vkDestroyImage;
    public delegate* unmanaged<VkCommandBuffer, VkBuffer, VkBuffer, uint, VkBufferCopy*, void> vkCmdCopyBuffer;
    public delegate* unmanaged<VkDevice, VkBufferMemoryRequirementsInfo2*, VkMemoryRequirements2*, void> vkGetBufferMemoryRequirements2KHR;
    public delegate* unmanaged<VkDevice, VkImageMemoryRequirementsInfo2*, VkMemoryRequirements2*, void> vkGetImageMemoryRequirements2KHR;
    public delegate* unmanaged<VkDevice, uint, VkBindBufferMemoryInfo*, VkResult> vkBindBufferMemory2KHR;
    public delegate* unmanaged<VkDevice, uint, VkBindImageMemoryInfo*, VkResult> vkBindImageMemory2KHR;
    public delegate* unmanaged<VkPhysicalDevice, VkPhysicalDeviceMemoryProperties2*, void> vkGetPhysicalDeviceMemoryProperties2KHR;
    public delegate* unmanaged<VkDevice, VkDeviceBufferMemoryRequirements*, VkMemoryRequirements2*, void> vkGetDeviceBufferMemoryRequirements;
    public delegate* unmanaged<VkDevice, VkDeviceImageMemoryRequirements*, VkMemoryRequirements2*, void> vkGetDeviceImageMemoryRequirements;
    public void* vkGetMemoryWin32HandleKHR;
}

internal unsafe partial struct VmaAllocatorCreateInfo
{
    public VmaAllocatorCreateFlags flags;
    public VkPhysicalDevice physicalDevice;
    public VkDevice device;
    public ulong preferredLargeHeapBlockSize;
    public VkAllocationCallbacks* pAllocationCallbacks;
    public VmaDeviceMemoryCallbacks* pDeviceMemoryCallbacks;
    public ulong* pHeapSizeLimit;
    internal VmaVulkanFunctions* pVulkanFunctions;
    public VkInstance instance;
    public VkVersion vulkanApiVersion;
    public VkExternalMemoryHandleTypeFlagsKHR* pTypeExternalMemoryHandleTypes;
}

internal unsafe partial struct VmaAllocationCreateInfo
{
    public VmaAllocationCreateFlags flags;
    public VmaMemoryUsage usage;
    public VkMemoryPropertyFlags requiredFlags;
    public VkMemoryPropertyFlags preferredFlags;
    public uint memoryTypeBits;
    public VmaPool pool;
    public void* pUserData;
    public float priority;
}

internal unsafe partial struct VmaAllocationInfo
{
    public uint memoryType;
    public VkDeviceMemory deviceMemory;
    public ulong offset;
    public ulong size;
    public void* pMappedData;
    public void* pUserData;
    public byte* pName;
}

internal partial struct VmaAllocationInfo2
{
    public VmaAllocationInfo allocationInfo;
    public ulong blockSize;
    public VkBool32 dedicatedMemory;
}

internal partial struct VmaStatistics
{
    public uint blockCount;
    public uint allocationCount;
    public ulong blockBytes;
    public ulong allocationBytes;
}

internal partial struct VmaDetailedStatistics
{
    public VmaStatistics statistics;
    public uint unusedRangeCount;
    public ulong allocationSizeMin;
    public ulong allocationSizeMax;
    public ulong unusedRangeSizeMin;
    public ulong unusedRangeSizeMax;
}

internal partial struct VmaTotalStatistics
{
    public memoryType__FixedBuffer memoryType;

    [InlineArray(32)]
    public partial struct memoryType__FixedBuffer
    {
        public VmaDetailedStatistics e0;
    }
    public memoryHeap__FixedBuffer memoryHeap;

    [InlineArray(16)]
    public partial struct memoryHeap__FixedBuffer
    {
        public VmaDetailedStatistics e0;
    }
    public VmaDetailedStatistics total;
}


internal unsafe partial class Vma
{
    public const string LibraryName = AlimerApi.LibraryName;

    [SkipLocalsInit]
    public static VkResult vmaCreateAllocator(in VmaAllocatorCreateInfo createInfo, out VmaAllocator allocator)
    {
        Unsafe.SkipInit(out allocator);

        VmaVulkanFunctions functions = default;
        functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr_ptr;
        functions.vkGetDeviceProcAddr = ((delegate* unmanaged<VkDevice, byte*, PFN_vkVoidFunction>)GetApi(createInfo.instance).vkGetDeviceProcAddr_ptr.Value);

        fixed (VmaAllocator* allocatorPtr = &allocator)
        {
            VmaAllocatorCreateInfo createInfoIn = createInfo;
            createInfoIn.pVulkanFunctions = &functions;
            return __PInvoke(&createInfoIn, allocatorPtr);
        }

        // Local P/Invoke
        [DllImport(LibraryName, EntryPoint = "vmaCreateAllocator", ExactSpelling = true)]
        static extern VkResult __PInvoke(VmaAllocatorCreateInfo* createInfo, VmaAllocator* pAllocator);
    }

    [LibraryImport(LibraryName, EntryPoint = "vmaDestroyAllocator")]
    public static partial void vmaDestroyAllocator(VmaAllocator allocator);

    [LibraryImport(LibraryName, EntryPoint = "vmaCreateBuffer")]
    public static partial VkResult vmaCreateBuffer(VmaAllocator allocator, VkBufferCreateInfo* bufferCreateInfo, VmaAllocationCreateInfo* allocationCreateInfo, out VkBuffer buffer, out VmaAllocation allocation, VmaAllocationInfo* allocationInfo = default);

    [LibraryImport(LibraryName, EntryPoint = "vmaDestroyBuffer")]
    public static partial void vmaDestroyBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);

    [LibraryImport(LibraryName, EntryPoint = "vmaCreateImage")]
    public static partial VkResult vmaCreateImage(VmaAllocator allocator, VkImageCreateInfo* imageCreateInfo, VmaAllocationCreateInfo* allocationCreateInfo, out VkImage image, out VmaAllocation allocation, VmaAllocationInfo* allocationInfo = default);

    [LibraryImport(LibraryName, EntryPoint = "vmaDestroyImage")]
    public static partial void vmaDestroyImage(VmaAllocator allocator, VkImage image, VmaAllocation allocation);

    [LibraryImport(LibraryName, EntryPoint = "vmaCalculateStatistics")]
    public static partial void vmaCalculateStatistics(VmaAllocator allocator, VmaTotalStatistics* stats);
}
