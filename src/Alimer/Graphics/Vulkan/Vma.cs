// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using VkDeviceSize = System.UInt64;

#pragma warning disable CS0649

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class Vma
{
    private const string LibraryName = "vma";

    #region Handles
    public readonly record struct VmaAllocator(nint Handle)
    {
        public bool IsNull => Handle == 0;
        public static VmaAllocator Null => default;
    }

    public readonly record struct VmaAllocation(nint Handle)
    {
        public bool IsNull => Handle == 0;
        public static VmaAllocation Null => default;
    }

    public readonly record struct VmaPool(nint Handle)
    {
        public bool IsNull => Handle == 0;
        public static VmaPool Null => default;
    }
    #endregion

    #region Enums
    [Flags]
    public enum VmaAllocatorCreateFlags
    {
        VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT = 0x00000001,
        VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT = 0x00000002,
        VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT = 0x00000004,
        VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT = 0x00000008,
        VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT = 0x00000010,
        VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT = 0x00000020,
        VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT = 0x00000040,
        VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT = 0x00000080,
        VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT = 0x00000100,
        VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT = 0x00000200,
    }

    public enum VmaMemoryUsage
    {
        VMA_MEMORY_USAGE_UNKNOWN = 0,
        VMA_MEMORY_USAGE_GPU_ONLY = 1, // deprecated
        //VMA_MEMORY_USAGE_CPU_ONLY = 2, // deprecated
        //VMA_MEMORY_USAGE_CPU_TO_GPU = 3, // deprecated
        //VMA_MEMORY_USAGE_GPU_TO_CPU = 4, // deprecated
        // VMA_MEMORY_USAGE_CPU_COPY = 5, // deprecated
        VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED = 6,
        VMA_MEMORY_USAGE_AUTO = 7,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE = 8,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST = 9,
    }

    [Flags]
    public enum VmaAllocationCreateFlags
    {
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT = 0x00000001,
        VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT = 0x00000002,
        VMA_ALLOCATION_CREATE_MAPPED_BIT = 0x00000004,
        //VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT = 0x00000020, // deprecated
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
        VMA_ALLOCATION_CREATE_STRATEGY_MASK =
            VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT |
            VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT |
            VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,
    }

    [Flags]
    public enum VmaPoolCreateFlags
    {
        VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT = 0x00000002,
        VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT = 0x00000004,
        /// <summary>
        /// Bit mask to extract only `ALGORITHM` bits from entire set of flags.
        /// </summary>
        VMA_POOL_CREATE_ALGORITHM_MASK = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
    }
    #endregion

    #region Structs
    /// Callback function called after successful vkAllocateMemory.
    /// typedef void (VKAPI_PTR* PFN_vmaAllocateDeviceMemoryFunction) (VmaAllocator allocator, uint32_t memoryType, VkDeviceMemory memory,VkDeviceSize size,void* pUserData);

    /// Callback function called before vkFreeMemory.
    /// typedef void (VKAPI_PTR* PFN_vmaFreeDeviceMemoryFunction) (VmaAllocator allocator,uint32_t memoryType, VkDeviceMemory memory, VkDeviceSize size,void* pUserData);

    public struct VmaDeviceMemoryCallbacks
    {
        // PFN_vmaAllocateDeviceMemoryFunction
        public delegate* unmanaged<VmaAllocator, uint, VkDeviceMemory, ulong, void*, void> pfnAllocate;
        // PFN_vmaFreeDeviceMemoryFunction
        public delegate* unmanaged<VmaAllocator, uint, VkDeviceMemory, ulong, void*, void> pfnFree;
        public void* pUserData;
    }

    public struct VmaVulkanFunctions
    {
        public required delegate* unmanaged<VkInstance, byte*, PFN_vkVoidFunction> vkGetInstanceProcAddr;
        public required delegate* unmanaged<VkDevice, byte*, PFN_vkVoidFunction> vkGetDeviceProcAddr;
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
        // typedef VkResult (VKAPI_PTR *PFN_vkGetMemoryWin32HandleKHR)(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle);
        public delegate* unmanaged<VkDevice, VkMemoryGetWin32HandleInfoKHR*, void*, VkResult> vkGetMemoryWin32HandleKHR;
        /// Fetch from "vkGetPhysicalDeviceProperties2" on Vulkan >= 1.1, but you can also fetch it from "vkGetPhysicalDeviceProperties2KHR" if you enabled extension VK_KHR_get_physical_device_properties2.
        public delegate* unmanaged<VkPhysicalDevice, VkPhysicalDeviceProperties2, void> vkGetPhysicalDeviceProperties2KHR;
    }

    public struct VmaAllocatorCreateInfo
    {
        public VmaAllocatorCreateFlags flags;
        public VkPhysicalDevice physicalDevice;
        public VkDevice device;
        public ulong preferredLargeHeapBlockSize;
        public VkAllocationCallbacks* pAllocationCallbacks;
        public VmaDeviceMemoryCallbacks* pDeviceMemoryCallbacks;
        public VkDeviceSize* pHeapSizeLimit;
        public VmaVulkanFunctions* pVulkanFunctions;
        public VkInstance instance;
        public VkVersion vulkanApiVersion;
        public VkExternalMemoryHandleTypeFlagsKHR* pTypeExternalMemoryHandleTypes;
    }

    public struct VmaAllocationCreateInfo
    {
        public VmaAllocationCreateFlags flags;
        public VmaMemoryUsage usage;
        public VkMemoryPropertyFlags requiredFlags;
        public VkMemoryPropertyFlags preferredFlags;
        public uint memoryTypeBits;
        public VmaPool pool;
        public nint pUserData;
        public float priority;
        public VkDeviceSize minAlignment;
    }

    public struct VmaAllocationInfo
    {
        public uint memoryType;
        public VkDeviceMemory deviceMemory;
        public VkDeviceSize offset;
        public VkDeviceSize size;
        public void* pMappedData;
        public void* pUserData;
        public byte* pName;
    }

    public struct VmaAllocationInfo2
    {
        public VmaAllocationInfo allocationInfo;
        public VkDeviceSize blockSize;
        public VkBool32 dedicatedMemory;
    }

    public struct VmaStatistics
    {
        public uint blockCount;
        public uint allocationCount;
        public VkDeviceSize blockBytes;
        public VkDeviceSize allocationBytes;
    }

    public struct VmaDetailedStatistics
    {
        public VmaStatistics statistics;
        public uint unusedRangeCount;
        public VkDeviceSize allocationSizeMin;
        public VkDeviceSize allocationSizeMax;
        public VkDeviceSize unusedRangeSizeMin;
        public VkDeviceSize unusedRangeSizeMax;
    }

    public struct VmaTotalStatistics
    {
        public memoryType__FixedBuffer memoryType;

        [InlineArray((int)VK_MAX_MEMORY_TYPES)]
        public struct memoryType__FixedBuffer
        {
            public VmaDetailedStatistics e0;
        }
        public memoryHeap__FixedBuffer memoryHeap;

        [InlineArray((int)VK_MAX_MEMORY_HEAPS)]
        public struct memoryHeap__FixedBuffer
        {
            public VmaDetailedStatistics e0;
        }
        public VmaDetailedStatistics total;
    }

    public struct VmaBudget
    {
        public VmaStatistics statistics;
        public VkDeviceSize usage;
        public VkDeviceSize budget;
    }

    public struct VmaPoolCreateInfo
    {
        public uint memoryTypeIndex;
        public VmaPoolCreateFlags flags;
        public VkDeviceSize blockSize;
        public nuint minBlockCount;
        public nuint maxBlockCount;
        public float priority;
        /** \brief Additional minimum alignment to be used for all allocations created from this pool. Can be 0.

        Leave 0 (default) not to impose any additional alignment. If not 0, it must be a power of two.

        When creating a buffer or an image, specifying a custom alignment is not needed in most cases,
        because Vulkan implementation inspects the `CreateInfo` structure (including intended usage flags)
        and returns required alignment through functions like `vkGetBufferMemoryRequirements2`, which VMA automatically
        uses and respects.
        Extra alignment may be needed in some cases, like when using a buffer for acceleration structure scratch
        (`VkPhysicalDeviceAccelerationStructurePropertiesKHR::minAccelerationStructureScratchOffsetAlignment`, see also issue #523)
        or when doing interop with OpenGL.
        */
        public VkDeviceSize minAllocationAlignment;
        /// <summary>
        /// Additional `pNext` chain to be attached to `VkMemoryAllocateInfo` used for every allocation made by this pool. Optional.
        ///
        /// Optional, can be null.If not null, it must point to a `pNext` chain of structures that can be attached to `VkMemoryAllocateInfo`.
        /// It can be useful for special needs such as adding `VkExportMemoryAllocateInfoKHR`.
        /// Structures pointed by this member must remain alive and unchanged for the whole lifetime of the custom pool.
        /// 
        /// Please note that some structures, e.g. `VkMemoryPriorityAllocateInfoEXT`, `VkMemoryDedicatedAllocateInfoKHR`,
        /// can be attached automatically by this library when using other, more convenient of its features.
        /// </summary>
        public void* pMemoryAllocateNext; // VMA_EXTENDS_VK_STRUCT(VkMemoryAllocateInfo)
    }

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
            return vmaCreateAllocator(&createInfoIn, allocatorPtr);
        }
    }
    #endregion

    [LibraryImport(LibraryName, EntryPoint = "vmaCreateAllocator")]
    public static partial VkResult vmaCreateAllocator(VmaAllocatorCreateInfo* createInfo, VmaAllocator* pAllocator);

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

    [LibraryImport(LibraryName)]
    public static partial void vmaGetHeapBudgets(VmaAllocator allocator, VmaBudget* pBudgets);

    [LibraryImport(LibraryName)]
    public static partial VkResult vmaCreatePool(VmaAllocator allocator, VmaPoolCreateInfo* pCreateInfo, VmaPool* pPool);

    [LibraryImport(LibraryName)]
    public static partial void vmaDestroyPool(VmaAllocator allocator, VmaPool pool);
}
