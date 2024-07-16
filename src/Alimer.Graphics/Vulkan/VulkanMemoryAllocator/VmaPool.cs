// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

/// <summary>
/// Flags to be passed as <see cref="VmaPoolCreateInfo.Flags"/>.
/// </summary>
[Flags]
public enum VmaPoolCreateFlags
{
    /// <summary>
    /// Use this flag if you always allocate only buffers and linear images or only optimal images out of this pool and so Buffer-Image Granularity can be ignored.
    /// 
    /// This is an optional optimization flag.
    /// 
    /// If you always allocate using vmaCreateBuffer(), vmaCreateImage(),
    /// vmaAllocateMemoryForBuffer(), then you don't need to use it because allocator
    /// knows exact type of your allocations so it can handle Buffer-Image Granularity
    /// in the optimal way.
    /// 
    /// If you also allocate using vmaAllocateMemoryForImage() or vmaAllocateMemory(),
    /// exact type of such allocations is not known, so allocator must be conservative
    /// in handling Buffer-Image Granularity, which can lead to suboptimal allocation
    /// (wasted memory). In that case, if you can make sure you always allocate only
    /// buffers and linear images or only optimal images out of this pool, use this flag
    /// to make allocator disregard Buffer-Image Granularity and so make allocations
    /// faster and more optimal.
    /// </summary>
    IgnoreBufferImageGranularity = 0x00000002,

    /// <summary>
    /// Enables alternative, linear allocation algorithm in this pool.
    /// 
    /// Specify this flag to enable linear allocation algorithm, which always creates
    /// new allocations after last one and doesn't reuse space from allocations freed in
    /// between.It trades memory consumption for simplified algorithm and data
    /// structure, which has better performance and uses less memory for metadata.
    /// 
    /// By using this flag, you can achieve behavior of free-at-once, stack, ring buffer, and double stack.
    /// </summary>
    LinearAlgorithm = 0x00000004,

    /// <summary>
    /// Bit mask to extract only `ALGORITHM` bits from entire set of flags.
    /// </summary>
    AlgorithmMask = LinearAlgorithm,
}

/// <summary>
/// Describes parameter of created <see cref="VmaPool"/>.
/// </summary>
public struct VmaPoolCreateInfo
{
    /// <summary>
    /// Vulkan memory type index to allocate this pool from.
    /// </summary>
    public int MemoryTypeIndex;

    /// <summary>
    /// Use combination of <see cref="VmaPoolCreateFlags"/>.
    /// </summary>
    public VmaPoolCreateFlags Flags;
    /// <summary>
    /// Size of a single `VkDeviceMemory` block to be allocated as part of this pool, in bytes. Optional.
    ///
    ///Specify nonzero to set explicit, constant size of memory blocks used by this pool.
    ///
    ///Leave 0 to use default and let the library manage block sizes automatically.
    ///Sizes of particular blocks may vary.
    ///In this case, the pool will also support dedicated allocations.
    /// </summary>
    public ulong BlockSize;
    /// <summary>
    /// Minimum number of blocks to be always allocated in this pool, even if they stay empty.
    /// Set to 0 to have no preallocated blocks and allow the pool be completely empty.
    /// </summary>
    public nuint MinBlockCount;
    /// <summary>
    /// Maximum number of blocks that can be allocated in this pool. Optional.
    /// Set to 0 to use default, which is `<see cref="nuint.MaxValue"/>`, which means no limit.
    /// Set to same value as <see cref="VmaPoolCreateInfo.MinBlockCount"/> to have fixed amount of memory allocated
    /// throughout whole lifetime of this pool.
    /// </summary>
    public nuint MaxBlockCount;
    /// <summary>
    /// A floating-point value between 0 and 1, indicating the priority of the allocations in this pool relative to other memory allocations.
    /// 
    /// It is used only when <see cref="VmaAllocatorCreateFlags.ExtMemoryPriority"/> flag was used during creation of the <see cref="VmaAllocator"/> object.
    /// Otherwise, this variable is ignored.
    /// </summary>
    public float Priority;
    /// <summary>
    /// Additional minimum alignment to be used for all allocations created from this pool. Can be 0.
    ///
    ///Leave 0 (default) not to impose any additional alignment.If not 0, it must be a power of two.
    ///It can be useful in cases where alignment returned by Vulkan by functions like `vkGetBufferMemoryRequirements` is not enough,
    ///e.g.when doing interop with OpenGL.
    /// </summary>
    public ulong MinAllocationAlignment;
    /// <summary>
    /// Additional `pNext` chain to be attached to `VkMemoryAllocateInfo` used for every allocation made by this pool. Optional.
    ///
    /// Optional, can be null. If not null, it must point to a `pNext` chain of structures that can be attached to `VkMemoryAllocateInfo`.
    /// It can be useful for special needs such as adding `VkExportMemoryAllocateInfoKHR`.
    /// Structures pointed by this member must remain alive and unchanged for the whole lifetime of the custom pool.
    ///
    /// Please note that some structures, e.g. `VkMemoryPriorityAllocateInfoEXT`, `VkMemoryDedicatedAllocateInfoKHR`,
    /// can be attached automatically by this library when using other, more convenient of its features.
    /// </summary>
    public unsafe void* pMemoryAllocateNext; // VkMemoryAllocateInfo
}

public unsafe class VmaPool : IDisposable
{
    internal VmaPool? _prevPool;
    internal VmaPool? _nextPool;

    internal VmaBlockVector BlockVector { get; }
    internal VmaDedicatedAllocationList DedicatedAllocations { get; }

    public uint Id { get; internal set; }

    public VmaPool(VmaAllocator allocator, in VmaPoolCreateInfo createInfo, ulong preferredBlockSize)
    {
        BlockVector = new(allocator,
            this, // hParentPool
            createInfo.MemoryTypeIndex,
            createInfo.BlockSize != 0 ? createInfo.BlockSize : preferredBlockSize,
            createInfo.MinBlockCount,
            createInfo.MaxBlockCount,
            (createInfo.Flags & VmaPoolCreateFlags.IgnoreBufferImageGranularity) != 0 ? 1 : allocator.BufferImageGranularity,
            createInfo.BlockSize != 0, // explicitBlockSize
            (uint)(createInfo.Flags & VmaPoolCreateFlags.AlgorithmMask), // algorithm
            createInfo.Priority,
            Math.Max(allocator.GetMemoryTypeMinAlignment(createInfo.MemoryTypeIndex), createInfo.MinAllocationAlignment),
            createInfo.pMemoryAllocateNext
        );
    }

    public void Dispose()
    {
        Debug.Assert(_prevPool == null && _nextPool == null);
        BlockVector.Dispose();
    }

    // vmaGetPoolStatistics
    public VmaStatistics GetPoolStatistics()
    {
        GetPoolStatistics(out VmaStatistics statistics);
        return statistics;
    }

    // vmaGetPoolStatistics
    public void GetPoolStatistics(out VmaStatistics statistics)
    {
        statistics = new();
        VmaClearStatistics(ref statistics);

        BlockVector.AddStatistics(ref statistics);
        //DedicatedAllocations.AddStatistics(ref statistics);
    }
}

