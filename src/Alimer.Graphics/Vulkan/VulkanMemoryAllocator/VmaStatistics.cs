// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Runtime.CompilerServices;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

public struct VmaStatistics
{
    public uint BlockCount;
    public uint AllocationCount;
    public ulong BlockBytes;
    public ulong AllocationBytes;
}

public struct VmaDetailedStatistics
{
    /// Basic statistics.
    public VmaStatistics Statistics;
    /// Number of free ranges of memory between allocations.
    public uint UnusedRangeCount;
    /// Smallest allocation size. `VK_WHOLE_SIZE` if there are 0 allocations.
    public ulong AllocationSizeMin;
    /// Largest allocation size. 0 if there are 0 allocations.
    public ulong AllocationSizeMax;
    /// Smallest empty range size. `VK_WHOLE_SIZE` if there are 0 empty ranges.
    public ulong UnusedRangeSizeMin;
    /// Largest empty range size. 0 if there are 0 empty ranges.
    public ulong UnusedRangeSizeMax;
}

public struct VmaTotalStatistics
{
    public memoryType__FixedBuffer MemoryType;
    public memoryHeap__FixedBuffer MemoryHeap;
    public VmaDetailedStatistics Total;

    [InlineArray((int)VK_MAX_MEMORY_TYPES)]
    public partial struct memoryType__FixedBuffer
    {
        public VmaDetailedStatistics e0;
    }

    [InlineArray((int)VK_MAX_MEMORY_TYPES)]
    public partial struct memoryHeap__FixedBuffer
    {
        public VmaDetailedStatistics e0;
    }
}

public struct VmaBudget 
{
    public VmaStatistics Statistics;
    public ulong Usage;
    public ulong Budget;
}
