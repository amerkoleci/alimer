// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

public struct Statistics
{
    public uint BlockCount;
    public uint AllocationCount;
    public ulong BlockBytes;
    public ulong AllocationBytes;
}

public struct DetailedStatistics
{
    /// Basic statistics.
    public Statistics Statistics;
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

public struct TotalStatistics
{
    public memoryType__FixedBuffer MemoryType;
    public memoryHeap__FixedBuffer MemoryHeap;
    public DetailedStatistics Total;

    [InlineArray((int)VK_MAX_MEMORY_TYPES)]
    public partial struct memoryType__FixedBuffer
    {
        public DetailedStatistics e0;
    }

    [InlineArray((int)VK_MAX_MEMORY_TYPES)]
    public partial struct memoryHeap__FixedBuffer
    {
        public DetailedStatistics e0;
    }
}

public struct MemoryBudget // VmaBudget
{
    public Statistics Statistics;
    public ulong Usage;
    public ulong Budget;
}
