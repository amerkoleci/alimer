// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

namespace Vortice.Vulkan;

/// <summary>
/// Flags for created <see cref="VmaAllocator"/>.
/// </summary>
[Flags]
public enum VmaAllocatorCreateFlags
{
    None = 0,
    /// <summary>
    /// Allocator and all objects created from it will not be synchronized internally, so you must guarantee they are used from only one thread at a time or synchronized externally by you.
    ///
    /// Using this flag may increase performance because internal mutexes are not used.
    /// </summary>
    ExternallySynchronized = 1 << 0,
    /// <summary>
    /// Enables usage of VK_EXT_memory_budget extension.
    /// 
    /// You may set this flag only if you found out that this device extension is supported,
    /// you enabled it while creating Vulkan device passed as VmaAllocatorCreateInfo::device,
    /// and you want it to be used internally by this library, along with another instance extension
    /// VK_KHR_get_physical_device_properties2, which is required by it (or Vulkan 1.1, where this extension is promoted).
    /// 
    /// The extension provides query for current memory usage and budget, which will probably
    /// be more accurate than an estimation used by the library otherwise.
    /// </summary>
    ExtMemoryBudget = 1 << 1,
    /// <summary>
    /// Enables usage of VK_AMD_device_coherent_memory extension.
    ///
    /// You may set this flag only if you:
    /// 
    /// - found out that this device extension is supported and enabled it while creating Vulkan device passed as VmaAllocatorCreateInfo::device,
    /// - checked that `VkPhysicalDeviceCoherentMemoryFeaturesAMD::deviceCoherentMemory` is true and set it while creating the Vulkan device,
    /// - want it to be used internally by this library.
    /// 
    /// The extension and accompanying device feature provide access to memory types with
    /// `VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD` and `VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD` flags.
    /// They are useful mostly for writing breadcrumb markers - a common method for debugging GPU crash/hang/TDR.
    /// 
    /// When the extension is not enabled, such memory types are still enumerated, but their usage is illegal.
    /// To protect from this error, if you don't create the allocator with this flag, it will refuse to allocate any memory or create a custom pool in such memory type,
    /// returning `VK_ERROR_FEATURE_NOT_PRESENT`.
    /// </summary>
    AMDDeviceCoherentMemory = 1 << 2,
    /// <summary>
    /// Enables usage of "buffer device address" feature, which allows you to use function
    /// `vkGetBufferDeviceAddress*` to get raw GPU pointer to a buffer and pass it for usage inside a shader.
    /// 
    /// You may set this flag only if you:
    /// 
    /// 1. (For Vulkan version < 1.2) Found as available and enabled device extension
    /// VK_KHR_buffer_device_address.
    /// This extension is promoted to core Vulkan 1.2.
    /// 2. Found as available and enabled device feature `VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress`.
    /// 
    /// When this flag is set, you can create buffers with `VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT` using VMA.
    /// The library automatically adds `VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT` to
    /// allocated memory blocks wherever it might be needed.
    /// </summary>
    BufferDeviceAddress = 1 << 3,
    /// <summary>
    /// Enables usage of VK_EXT_memory_priority extension in the library.
    /// 
    /// You may set this flag only if you found available and enabled this device extension,
    /// along with `VkPhysicalDeviceMemoryPriorityFeaturesEXT::memoryPriority == VK_TRUE`,
    /// while creating Vulkan device passed as VmaAllocatorCreateInfo::device.
    /// 
    /// When this flag is used, VmaAllocationCreateInfo::priority and VmaPoolCreateInfo::priority
    /// are used to set priorities of allocated Vulkan memory. Without it, these variables are ignored.
    /// 
    /// A priority must be a floating-point value between 0 and 1, indicating the priority of the allocation relative to other memory allocations.
    /// Larger values are higher priority. The granularity of the priorities is implementation-dependent.
    /// It is automatically passed to every call to `vkAllocateMemory` done by the library using structure `VkMemoryPriorityAllocateInfoEXT`.
    /// The value to be used for default priority is 0.5.
    /// </summary>
    ExtMemoryPriority = 1 << 4,
    /// <summary>
    /// Enables usage of VK_KHR_maintenance4 extension in the library.
    ///
    /// You may set this flag only if you found available and enabled this device extension,
    /// while creating Vulkan device passed as VmaAllocatorCreateInfo::device.
    /// </summary>
    KhrMaintenance4 = 1 << 5,
    /// <summary>
    /// Enables usage of VK_KHR_maintenance5 extension in the library.
    /// 
    /// You should set this flag if you found available and enabled this device extension,
    /// while creating Vulkan device passed as VmaAllocatorCreateInfo::device.
    /// </summary>
    KhrMaintenance5 = 1 << 6,
}

public readonly unsafe struct VmaAllocatorCreateInfo
{
    /// <summary>
    /// Flags for created allocator.
    /// </summary>
    public VmaAllocatorCreateFlags Flags { get; init; }

    /// <summary>
    /// Vulkan physical device.
    /// </summary>
    public required VkPhysicalDevice PhysicalDevice { get; init; }

    /// <summary>
    /// Vulkan device.
    /// </summary>
    public required VkDevice Device { get; init; }

    /// <summary>
    /// Optional preferred size of a single `VkDeviceMemory` block to be allocated from large heaps > 1 GiB.
    /// Set to 0 to use default, which is currently 256 MiB. 
    /// </summary>
    public ulong PreferredLargeHeapBlockSize { get; init; }

    /// <summary>
    /// Optional custom CPU memory allocation callbacks.
    /// Can be null. When specified, will also be used for all CPU-side memory allocations.
    /// </summary>
    public unsafe VkAllocationCallbacks* pAllocationCallbacks { get; init; }

#if TODO
    /// Informative callbacks for `vkAllocateMemory`, `vkFreeMemory`. Optional.
    /** Optional, can be null. */
    const VmaDeviceMemoryCallbacks* VMA_NULLABLE pDeviceMemoryCallbacks;
#endif

    /** \brief Either null or a pointer to an array of limits on maximum number of bytes that can be allocated out of particular Vulkan memory heap.

    If not NULL, it must be a pointer to an array of
    `VkPhysicalDeviceMemoryProperties::memoryHeapCount` elements, defining limit on
    maximum number of bytes that can be allocated out of particular Vulkan memory
    heap.

    Any of the elements may be equal to `VK_WHOLE_SIZE`, which means no limit on that
    heap. This is also the default in case of `pHeapSizeLimit` = NULL.

    If there is a limit defined for a heap:

    - If user tries to allocate more memory from that heap using this allocator,
      the allocation fails with `VK_ERROR_OUT_OF_DEVICE_MEMORY`.
    - If the limit is smaller than heap size reported in `VkMemoryHeap::size`, the
      value of this limit will be reported instead when using vmaGetMemoryProperties().

    Warning! Using this feature may not be equivalent to installing a GPU with
    smaller amount of memory, because graphics driver doesn't necessary fail new
    allocations with `VK_ERROR_OUT_OF_DEVICE_MEMORY` result when memory capacity is
    exceeded. It may return success and just silently migrate some device memory
    blocks to system RAM. This driver behavior can also be controlled using
    VK_AMD_memory_overallocation_behavior extension.
    */
    public ulong* pHeapSizeLimit { get; init; }


    /** \brief Optional. Vulkan version that the application uses.

    It must be a value in the format as created by macro `VK_MAKE_VERSION` or a constant like: `VK_API_VERSION_1_1`, `VK_API_VERSION_1_0`.
    The patch version number specified is ignored. Only the major and minor versions are considered.
    Only versions 1.0, 1.1, 1.2, 1.3 are supported by the current implementation.
    Leaving it initialized to zero is equivalent to `VK_API_VERSION_1_0`.
    It must match the Vulkan version used by the application and supported on the selected physical device,
    so it must be no higher than `VkApplicationInfo::apiVersion` passed to `vkCreateInstance`
    and no higher than `VkPhysicalDeviceProperties::apiVersion` found on the physical device used.
    */
    public VkVersion VulkanApiVersion { get; init; }

    /// <summary>
    /// Either null or a pointer to an array of external memory handle types for each Vulkan memory type.
    /// 
    /// If not NULL, it must be a pointer to an array of `VkPhysicalDeviceMemoryProperties::memoryTypeCount`
    /// elements, defining external memory handle types of particular Vulkan memory type,
    /// to be passed using `VkExportMemoryAllocateInfoKHR`.
    /// 
    /// Any of the elements may be equal to 0, which means not to use `VkExportMemoryAllocateInfoKHR` on this memory type.
    /// This is also the default in case of `pTypeExternalMemoryHandleTypes` = NULL.
    /// </summary>
    public VkExternalMemoryHandleTypeFlags* pTypeExternalMemoryHandleTypes { get; init; }
}
