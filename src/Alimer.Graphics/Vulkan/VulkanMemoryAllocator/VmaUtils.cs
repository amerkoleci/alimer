// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

internal unsafe partial class VmaUtils
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static unsafe nuint __alignof<T>()
            where T : unmanaged
    {
        AlignOf<T> alignof = new AlignOf<T>();
        return (nuint)(nint)(Unsafe.ByteOffset(ref alignof.Origin, ref Unsafe.As<T, byte>(ref alignof.Target)));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static unsafe uint __sizeof<T>()
        where T : unmanaged
    {
        return (uint)(sizeof(T));
    }

    public static void* memset(void* s, int c, nuint n)
    {
        Unsafe.InitBlock(s, (byte)(c), (uint)(n));
        return s;
    }

    public static void* memcpy(void* s1, void* s2, nuint n)
    {
        Unsafe.CopyBlock(s1, s2, (uint)(n));
        return s1;
    }

    public static void* memmove(void* s1, void* s2, nuint n)
    {
        Unsafe.CopyBlock(s1, s2, (uint)(n));
        return s1;
    }

    public static void ZeroMemory(void* dst, nuint size)
    {
        _ = memset(dst, 0, size);
    }

    public static T* AllocateArray<T>(nuint count)
        where T : unmanaged
    {
        T* result = (T*)(DefaultAllocate(__sizeof<T>() * count, __alignof<T>()));
        ZeroMemory(result, __sizeof<T>() * count);
        return result;
    }

    public static void DeleteArray<T>(T* memory)
        where T : unmanaged
    {
        if (memory != null)
        {
            Free(memory);
        }
    }

    public static void DeleteArray<T>(T* memory, nuint count)
        where T : unmanaged, IDisposable
    {
        if (memory != null)
        {
            for (nuint i = count; i-- != 0;)
            {
                memory[i].Dispose();
            }
            Free(memory);
        }
    }

    public static void Free(void* pMemory)
    {
        DefaultFree(pMemory);
    }

    internal static void VMA_SWAP<T>(ref T a, ref T b)
        where T : unmanaged
    {
        T tmp = a;
        a = b;
        b = tmp;
    }

    /// <summary>Scans integer for index of first nonzero bit from the Least Significant Bit (LSB). If mask is 0 then returns byte.MaxValue</summary>
    /// <param name="mask"></param>
    /// <returns></returns>
    internal static byte VmaBitScanLSB(ulong mask)
    {
        byte pos = 0;
        ulong bit = 1;

        do
        {
            if ((mask & bit) != 0)
            {
                return pos;
            }
            bit <<= 1;
        }
        while (pos++ < 63);

        return byte.MaxValue;
    }

    internal static byte VmaBitScanMSB(ulong mask)
    {
        byte pos = 63;
        ulong bit = 1ul << 63;

        do
        {
            if ((mask & bit) != 0)
            {
                return pos;
            }
            bit >>= 1;
        }
        while (pos-- > 0);

        return byte.MaxValue;
    }

    /// <summary>Scans integer for index of first nonzero bit from the Most Significant Bit (MSB). If mask is 0 then returns byte.MaxValue</summary>
    /// <param name="mask"></param>
    /// <returns></returns>
    internal static byte VmaBitScanMSB(uint mask)
    {
        byte pos = 31;
        uint bit = 1U << 31;

        do
        {
            if ((mask & bit) != 0)
            {
                return pos;
            }

            bit >>= 1;
        }
        while (pos-- > 0);

        return byte.MaxValue;
    }

    private static void* DefaultAllocate(nuint size, nuint alignment)
    {
        return NativeMemory.AlignedAlloc(size, alignment);
    }

    private static void DefaultFree(void* pMemory)
    {
        NativeMemory.AlignedFree(pMemory);
    }

    public static void VmaClearStatistics(ref Statistics stats)
    {
        stats.BlockCount = 0;
        stats.AllocationCount = 0;
        stats.BlockBytes = 0;
        stats.AllocationBytes = 0;
    }

    public static void VmaAddStatistics(ref Statistics stats, in Statistics src)
    {
        stats.BlockCount += src.BlockCount;
        stats.AllocationCount += src.AllocationCount;
        stats.BlockBytes += src.BlockBytes;
        stats.AllocationBytes += src.AllocationBytes;
    }

    public static void VmaClearDetailedStatistics(ref DetailedStatistics outStats)
    {
        VmaClearStatistics(ref outStats.Statistics);
        outStats.UnusedRangeCount = 0;
        outStats.AllocationSizeMin = VK_WHOLE_SIZE;
        outStats.AllocationSizeMax = 0;
        outStats.UnusedRangeSizeMin = VK_WHOLE_SIZE;
        outStats.UnusedRangeSizeMax = 0;
    }


    public static void VmaAddDetailedStatisticsAllocation(ref DetailedStatistics inoutStats, ulong size)
    {
        inoutStats.Statistics.AllocationCount++;
        inoutStats.Statistics.AllocationBytes += size;
        inoutStats.AllocationSizeMin = Math.Min(inoutStats.AllocationSizeMin, size);
        inoutStats.AllocationSizeMax = Math.Max(inoutStats.AllocationSizeMax, size);
    }

    public static void VmaAddDetailedStatisticsUnusedRange(ref DetailedStatistics inoutStats, ulong size)
    {
        inoutStats.UnusedRangeCount++;
        inoutStats.UnusedRangeSizeMin = Math.Min(inoutStats.UnusedRangeSizeMin, size);
        inoutStats.UnusedRangeSizeMax = Math.Max(inoutStats.UnusedRangeSizeMax, size);
    }

    public static void VmaAddDetailedStatistics(ref DetailedStatistics stats, in DetailedStatistics src)
    {
        VmaAddStatistics(ref stats.Statistics, src.Statistics);
        stats.UnusedRangeCount += src.UnusedRangeCount;
        stats.AllocationSizeMin = Math.Min(stats.AllocationSizeMin, src.AllocationSizeMin);
        stats.AllocationSizeMax = Math.Max(stats.AllocationSizeMax, src.AllocationSizeMax);
        stats.UnusedRangeSizeMin = Math.Min(stats.UnusedRangeSizeMin, src.UnusedRangeSizeMin);
        stats.UnusedRangeSizeMax = Math.Max(stats.UnusedRangeSizeMax, src.UnusedRangeSizeMax);
    }

    private struct AlignOf<T> where T : unmanaged
    {
        public byte Origin;

        public T Target;
    }
}
