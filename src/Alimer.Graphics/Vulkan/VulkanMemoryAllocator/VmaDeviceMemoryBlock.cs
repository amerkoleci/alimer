// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

internal unsafe partial class VmaDeviceMemoryBlock : IDisposable
{
    private readonly VmaAllocator _allocator;
    private readonly VmaMappingHysteresis _mappingHysteresis = new();
    private uint _mapCount;
    private void* _pMappedData;
    private readonly object _syncLock = new object();

    public VmaDeviceMemoryBlock(VmaAllocator allocator,
        VmaPool? parentPool,
        int memoryTypeIndex,
        VkDeviceMemory memory,
        ulong newSize,
        uint id,
        uint algorithm,
        ulong bufferImageGranularity)
    {
        _allocator = allocator;
        ParentPool = parentPool;
        MemoryTypeIndex = memoryTypeIndex;
        Memory = memory;
        Id = id;

        switch (algorithm)
        {
            case 0:
                MetaData = new VmaBlockMetadata_TLSF(newSize, bufferImageGranularity, false); // isVirtual
                break;
            case VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT:
                throw new NotImplementedException();
                //MetaData = new VmaBlockMetadata_Linear)(bufferImageGranularity, false); // isVirtual
                break;
            default:
                throw new InvalidOperationException();
        }
    }

    public VmaPool? ParentPool { get; }
    public int MemoryTypeIndex { get; }
    public VkDeviceMemory Memory { get; private set; }
    public uint Id { get; }

    public IBlockMetadata MetaData { get; }

    public void Dispose()
    {
        Debug.Assert(_mapCount == 0, "VkDeviceMemory block is being destroyed while it is still mapped.");
        Debug.Assert(Memory.IsNull);
    }

    public void* GetMappedData() => _pMappedData;
    public uint GetMapRefCount() => _mapCount;

    public void PostAlloc(VmaAllocator allocator)
    {
        lock (_syncLock)
        {
            _mappingHysteresis.PostAlloc();
        }
    }

    public void PostFree(VmaAllocator allocator)
    {
        lock (_syncLock)
        {
            if (_mappingHysteresis.PostFree())
            {
                Debug.Assert(_mappingHysteresis.ExtraMapping == 0);
                if (_mapCount == 0)
                {
                    _pMappedData = null;
                    vkUnmapMemory(allocator.Device, Memory);
                }
            }
        }
    }

    public bool Validate()
    {
        Debug.Assert(Memory.IsNotNull && MetaData.Size != 0);

        return MetaData.Validate();
    }

    public void Destroy(VmaAllocator allocator)
    {
        // Define macro VMA_DEBUG_LOG_FORMAT or more specialized VMA_LEAK_LOG_FORMAT
        // to receive the list of the unfreed allocations.
        if (!MetaData.IsEmpty)
        {
            //MetaData->DebugLogAllAllocations();
        }

        // This is the most important assert in the entire library.
        // Hitting it means you have some memory leak - unreleased VmaAllocation objects.
        Debug.Assert(MetaData.IsEmpty, "Some allocations were not freed before destruction of this memory block!");

        Debug.Assert(Memory.IsNotNull);
        allocator.FreeVulkanMemory(MemoryTypeIndex, MetaData.Size, Memory);
        Memory = VkDeviceMemory.Null;

        //vma_delete(allocator, m_pMetadata);
        MetaData.Dispose();
    }

    public VkResult Map(VmaAllocator allocator, uint count, void** ppData)
    {
        if (count == 0)
        {
            return VkResult.Success;
        }

        lock (_syncLock)
        {
            uint oldTotalMapCount = _mapCount + _mappingHysteresis.ExtraMapping;
            _mappingHysteresis.PostMap();
            if (oldTotalMapCount != 0)
            {
                _mapCount += count;
                Debug.Assert(_pMappedData != null);
                if (ppData != null)
                {
                    *ppData = _pMappedData;
                }

                return VkResult.Success;
            }
            else
            {
                void* mappedAddress;
                VkResult result = vkMapMemory(
                    _allocator.Device,
                    Memory,
                    0, // offset
                    VK_WHOLE_SIZE,
                    0, // flags
                    &mappedAddress
                    );
                _pMappedData = mappedAddress;
                if (result == VkResult.Success)
                {
                    if (ppData != null)
                    {
                        *ppData = _pMappedData;
                    }
                    _mapCount = count;
                }

                return result;
            }
        }
    }

    public void Unmap(VmaAllocator allocator, uint count)
    {
        if (count == 0)
        {
            return;
        }

        lock (_syncLock)
        {
            if (_mapCount >= count)
            {
                _mapCount -= count;
                uint totalMapCount = _mapCount + _mappingHysteresis.ExtraMapping;
                if (totalMapCount == 0)
                {
                    _pMappedData = null;
                    vkUnmapMemory(allocator.Device, Memory);
                }
                _mappingHysteresis.PostUnmap();
            }
            else
            {
                Debug.Assert(false, "VkDeviceMemory block is being unmapped while it was not previously mapped.");
            }
        }
    }

    public VkResult BindBufferMemory(VmaAllocator allocator, VmaAllocation allocation, ulong allocationLocalOffset, VkBuffer buffer, void* pNext)
    {
        Debug.Assert(allocation.Type == VmaAllocationType.Block && allocation.GetBlock() == this);
        Debug.Assert(allocationLocalOffset < allocation.Size, "Invalid allocationLocalOffset. Did you forget that this offset is relative to the beginning of the allocation, not the whole memory block?");

        ulong memoryOffset = allocation.Offset + allocationLocalOffset;
        // This lock is important so that we don't call vkBind... and/or vkMap... simultaneously on the same VkDeviceMemory from multiple threads.
        lock (_syncLock)
        {
            return allocator.BindVulkanBuffer(Memory, memoryOffset, buffer, pNext);
        }
    }

    public VkResult BindImageMemory(VmaAllocator allocator, VmaAllocation allocation, ulong allocationLocalOffset, VkImage image, void* pNext)
    {
        Debug.Assert(allocation.Type == VmaAllocationType.Block && allocation.GetBlock() == this);
        Debug.Assert(allocationLocalOffset < allocation.Size, "Invalid allocationLocalOffset. Did you forget that this offset is relative to the beginning of the allocation, not the whole memory block?");

        ulong memoryOffset = allocation.Offset + allocationLocalOffset;
        // This lock is important so that we don't call vkBind... and/or vkMap... simultaneously on the same VkDeviceMemory from multiple threads.
        lock (_syncLock)
        {
            return allocator.BindVulkanImage(Memory, memoryOffset, image, pNext);
        }
    }
}

