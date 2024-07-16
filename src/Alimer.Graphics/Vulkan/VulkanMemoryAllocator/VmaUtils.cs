// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

internal unsafe partial class VmaUtils
{
    public const uint VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT = 0x00000004;
    public const uint VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD_COPY = 0x00000040;
    public const uint VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD_COPY = 0x00000080;
    public const uint VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_COPY = 0x00020000;
    public const uint VK_IMAGE_CREATE_DISJOINT_BIT_COPY = 0x00000200;
    public const int VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT_COPY = 1000158000;
    public const uint VMA_ALLOCATION_INTERNAL_STRATEGY_MIN_OFFSET = 0x10000000u;
    public const uint VMA_ALLOCATION_TRY_COUNT = 32;
    public const uint VMA_VENDOR_ID_AMD = 4098;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static unsafe nuint __alignof<T>()
            where T : unmanaged
    {
        AlignOf<T> alignof = new();
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
    public static void* memcpy(void* destination, void* source, nuint n)
    {
        NativeMemory.Copy(source, destination, n);
        return destination;
    }

    public static void* memmove(void* destination, void* source, nuint n)
    {
        Unsafe.CopyBlock(destination, source, (uint)(n));
        return destination;
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

    public static void VmaPnextChainPushFront<MainT, NewT>(MainT* mainStruct, NewT* newStruct)
        where MainT : unmanaged, IChainType
        where NewT : unmanaged, IChainType
    {
        newStruct->pNext = mainStruct->pNext;
        mainStruct->pNext = newStruct;
    }

    public static FindT* VmaPnextChainFind<FindT>(in IChainType mainStruct, VkStructureType sType)
        where FindT : unmanaged, IChainType
    {
        for (VkBaseInStructure* s = (VkBaseInStructure*)mainStruct.pNext; s != null; s = s->pNext)
        {
            if (s->sType == sType)
            {
                return (FindT*)s;
            }
        }

        return default;
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

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool VmaIsPow2(ulong value) => BitOperations.IsPow2(value);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong VmaAlignUp(ulong address, ulong alignment)
    {
        Debug.Assert(VmaIsPow2(alignment));
        return (address + alignment - 1) & ~(alignment - 1);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong VmaAlignDown(ulong address, ulong alignment)
    {
        Debug.Assert(VmaIsPow2(alignment));
        return address & ~(alignment - 1);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong VmaDivideRoundingUp(ulong x, ulong y)
    {
        return (x + y - 1) / y;
    }

    private static void* DefaultAllocate(nuint size, nuint alignment)
    {
        return NativeMemory.AlignedAlloc(size, alignment);
    }

    private static void DefaultFree(void* pMemory)
    {
        NativeMemory.AlignedFree(pMemory);
    }

    public static void VmaClearStatistics(ref VmaStatistics stats)
    {
        stats.BlockCount = 0;
        stats.AllocationCount = 0;
        stats.BlockBytes = 0;
        stats.AllocationBytes = 0;
    }

    public static void VmaAddStatistics(ref VmaStatistics stats, in VmaStatistics src)
    {
        stats.BlockCount += src.BlockCount;
        stats.AllocationCount += src.AllocationCount;
        stats.BlockBytes += src.BlockBytes;
        stats.AllocationBytes += src.AllocationBytes;
    }

    public static void VmaClearDetailedStatistics(ref VmaDetailedStatistics outStats)
    {
        VmaClearStatistics(ref outStats.Statistics);
        outStats.UnusedRangeCount = 0;
        outStats.AllocationSizeMin = VK_WHOLE_SIZE;
        outStats.AllocationSizeMax = 0;
        outStats.UnusedRangeSizeMin = VK_WHOLE_SIZE;
        outStats.UnusedRangeSizeMax = 0;
    }


    public static void VmaAddDetailedStatisticsAllocation(ref VmaDetailedStatistics inoutStats, ulong size)
    {
        inoutStats.Statistics.AllocationCount++;
        inoutStats.Statistics.AllocationBytes += size;
        inoutStats.AllocationSizeMin = Math.Min(inoutStats.AllocationSizeMin, size);
        inoutStats.AllocationSizeMax = Math.Max(inoutStats.AllocationSizeMax, size);
    }

    public static void VmaAddDetailedStatisticsUnusedRange(ref VmaDetailedStatistics inoutStats, ulong size)
    {
        inoutStats.UnusedRangeCount++;
        inoutStats.UnusedRangeSizeMin = Math.Min(inoutStats.UnusedRangeSizeMin, size);
        inoutStats.UnusedRangeSizeMax = Math.Max(inoutStats.UnusedRangeSizeMax, size);
    }

    public static void VmaAddDetailedStatistics(ref VmaDetailedStatistics stats, in VmaDetailedStatistics src)
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

internal readonly struct VmaMutexLockRead : IDisposable
{
    private readonly ReaderWriterLockSlim? _mutex;

    public VmaMutexLockRead(ReaderWriterLockSlim mutex, bool useMutex)
    {
        if (useMutex)
        {
            _mutex = mutex;
            mutex.EnterReadLock();
        }
    }

    public void Dispose()
    {
        _mutex?.ExitReadLock();
    }
}

internal readonly struct VmaMutexLockWrite : IDisposable
{
    private readonly ReaderWriterLockSlim? _mutex;

    public VmaMutexLockWrite(ReaderWriterLockSlim mutex, bool useMutex)
    {
        if (useMutex)
        {
            _mutex = mutex;
            mutex.EnterWriteLock();
        }
    }

    public void Dispose()
    {
        _mutex?.ExitWriteLock();
    }
}
