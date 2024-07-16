// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

internal unsafe class VmaBlockBufferImageGranularity : IDisposable
{
    private const ulong MAX_LOW_BUFFER_IMAGE_GRANULARITY = 256;
    private struct RegionInfo
    {
        public byte allocType;
        public ushort allocCount;
    };

    public readonly ulong BufferImageGranularity;
    private uint _regionCount;
    private RegionInfo* _regionInfo;

    public VmaBlockBufferImageGranularity(ulong bufferImageGranularity)
    {
        BufferImageGranularity = bufferImageGranularity;
    }

    public void Dispose()
    {
        if (_regionInfo != null)
        {
            NativeMemory.Free(_regionInfo);
            _regionInfo = default;
        }

        GC.SuppressFinalize(this);
    }

    public bool IsEnabled => BufferImageGranularity > MAX_LOW_BUFFER_IMAGE_GRANULARITY;

    public void Init(ulong size)
    {
        if (!IsEnabled)
            return;

        _regionCount = (uint)VmaDivideRoundingUp(size, BufferImageGranularity);
        _regionInfo = (RegionInfo*)NativeMemory.Alloc(_regionCount, (nuint)sizeof(RegionInfo));
        memset(_regionInfo, 0, (nuint)(_regionCount * sizeof(RegionInfo)));
    }

    public void RoundupAllocRequest(VmaSuballocationType allocType, ref ulong allocSize, ref ulong allocAlignment)
    {
        if (BufferImageGranularity > 1 &&
            BufferImageGranularity <= MAX_LOW_BUFFER_IMAGE_GRANULARITY)
        {
            if (allocType == VmaSuballocationType.Unknown
                || allocType == VmaSuballocationType.ImageUnknown
                || allocType == VmaSuballocationType.ImageOptimal)
            {
                allocAlignment = Math.Max(allocAlignment, BufferImageGranularity);
                allocSize = VmaAlignUp(allocSize, BufferImageGranularity);
            }
        }
    }

    public ValidationContext StartValidation(bool isVirtual)
    {
        ValidationContext ctx = new();
        if (!isVirtual && IsEnabled)
        {
            ctx.PageAllocs = (ushort*)NativeMemory.Alloc(_regionCount, (nuint)sizeof(ushort));
            memset(ctx.PageAllocs, 0, _regionCount * sizeof(ushort));
        }
        return ctx;
    }

    public bool Validate(ref ValidationContext ctx, ulong offset, ulong size)
    {
        if (IsEnabled)
        {
            uint start = GetStartPage(offset);
            ++ctx.PageAllocs[start];
            Debug.Assert(_regionInfo[start].allocCount > 0);

            uint end = GetEndPage(offset, size);
            if (start != end)
            {
                ++ctx.PageAllocs[end];
                Debug.Assert(_regionInfo[end].allocCount > 0);
            }
        }

        return true;
    }

    public bool FinishValidation(ref ValidationContext ctx)
    {
        // Check proper page structure
        if (IsEnabled)
        {
            Debug.Assert(ctx.PageAllocs != null, "Validation context not initialized!");

            for (uint page = 0; page < _regionCount; ++page)
            {
                Debug.Assert(ctx.PageAllocs[page] == _regionInfo[page].allocCount);
            }
            NativeMemory.Free(ctx.PageAllocs);
            ctx.PageAllocs = null;
        }

        return true;
    }

    private uint GetStartPage(ulong offset) => OffsetToPageIndex(offset & ~(BufferImageGranularity - 1));
    private uint GetEndPage(ulong offset, ulong size) => OffsetToPageIndex((offset + size - 1) & ~(BufferImageGranularity - 1));

    private uint OffsetToPageIndex(ulong offset) => (uint)(offset >> VmaBitScanMSB(BufferImageGranularity));

    public struct ValidationContext
    {
        public ushort* PageAllocs;
    }
}
