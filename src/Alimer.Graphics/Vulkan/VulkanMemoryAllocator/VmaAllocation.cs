// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

public enum VmaAllocationType
{
    None,
    Block,
    Dedicated
}

/// <summary>
/// The object containing details on a suballocation of Vulkan Memory
/// </summary>
public unsafe class VmaAllocation : IDisposable
{
    private Flags _flags;
    // Reference counter for vmaMapMemory()/vmaUnmapMemory().
    private byte _mapCount;
    // Allocation out of VmaDeviceMemoryBlock.
    private VmaDeviceMemoryBlock? _blockAllocationBlock;
    private ulong _blockAllocationHandle;

    // Allocation for an object that has its own private VkDeviceMemory.
    //struct DedicatedAllocation
    //{
    //    VmaPool m_hParentPool; // VK_NULL_HANDLE if not belongs to custom pool.
    //    VkDeviceMemory m_hMemory;
    //    void* m_pMappedData; // Not null means memory is mapped.
    //    VmaAllocation_T* m_Prev;
    //    VmaAllocation_T* m_Next;
    //};
    private VkDeviceMemory _dedicatedAllocationMemory = VkDeviceMemory.Null;
    private void* _dedicatedAllocationMappedData; // Not null means memory is mapped.

    public VmaAllocation(bool mappingAllowed)
    {
        if (mappingAllowed)
        {
            _flags |= Flags.MappingAllowed;
        }
    }

    public void Dispose()
    {
        Debug.Assert(_mapCount == 0, "Allocation was not unmapped before destruction.");
        GC.SuppressFinalize(this);
    }

    public ulong Alignment { get; private set; } = 1;
    public ulong Size { get; private set; }
    public ulong Offset
    {
        get
        {
            switch (Type)
            {
                case VmaAllocationType.Block:
                    return _blockAllocationBlock!.MetaData.GetAllocationOffset(_blockAllocationHandle);
                case VmaAllocationType.Dedicated:
                    return 0;
                default:
                    Debug.Assert(false);
                    return 0;
            }
        }
    }
    public VmaAllocationType Type { get; private set; }
    public int MemoryTypeIndex { get; private set; }

    public bool IsPersistentMap => (_flags & Flags.PersistentMap) != 0;
    public bool IsMappingAllowed => (_flags & Flags.MappingAllowed) != 0;

    internal VmaSuballocationType  SuballocationType { get; private set; }

    internal VmaPool? ParentPool
    {
        get
        {
            switch (Type)
            {
                case VmaAllocationType.Block:
                    return _blockAllocationBlock!.ParentPool;
                case VmaAllocationType.Dedicated:
                    //return m_DedicatedAllocation.m_hParentPool;
                    throw new NotImplementedException();
                default:
                    Debug.Assert(false);
                    throw new InvalidOperationException();
            }
        }
    }

    internal void InitBlockAllocation(VmaDeviceMemoryBlock block,
        ulong allocHandle,
        ulong alignment,
        ulong size,
        int memoryTypeIndex,
        VmaSuballocationType  suballocationType,
        bool mapped)
    {
        Debug.Assert(Type == VmaAllocationType.None);
        Debug.Assert(block != null);

        Type = VmaAllocationType.Block;
        Alignment = alignment;
        Size = size;
        MemoryTypeIndex = memoryTypeIndex;
        if (mapped)
        {
            Debug.Assert(IsMappingAllowed, "Mapping is not allowed on this allocation! Please use one of the new VMA_ALLOCATION_CREATE_HOST_ACCESS_* flags when creating it.");
            _flags |= Flags.PersistentMap;
        }

        SuballocationType = suballocationType;
        _blockAllocationBlock = block;
        _blockAllocationHandle = allocHandle;
    }

    internal VmaDeviceMemoryBlock GetBlock() => _blockAllocationBlock!;
    internal ulong GetAllocHandle() => _blockAllocationHandle;

    public VkDeviceMemory GetMemory()
    {
        switch (Type)
        {
            case VmaAllocationType.Block:
                return _blockAllocationBlock!.Memory;
            case VmaAllocationType.Dedicated:
                return _dedicatedAllocationMemory;
            default:
                Debug.Assert(false);
                return VkDeviceMemory.Null;
        }
    }

    public void* GetMappedData()
    {
        switch (Type)
        {
            case VmaAllocationType.Block:
                if (_mapCount != 0 || IsPersistentMap)
                {
                    void* pBlockData = _blockAllocationBlock!.GetMappedData();
                    Debug.Assert(pBlockData != null);
                    return (char*)pBlockData + Offset;
                }
                return null;

            case VmaAllocationType.Dedicated:
                Debug.Assert((_dedicatedAllocationMappedData != null) == (_mapCount != 0 || IsPersistentMap));
                return _dedicatedAllocationMappedData;
            default:
                Debug.Assert(false);
                return null;
        }
    }

    public void BlockAllocMap()
    {
        Debug.Assert(Type == VmaAllocationType.Block);
        Debug.Assert(IsMappingAllowed, "Mapping is not allowed on this allocation! Please use one of the new VMA_ALLOCATION_CREATE_HOST_ACCESS_* flags when creating it.");

        if (_mapCount < 0xFF)
        {
            ++_mapCount;
        }
        else
        {
            Debug.Assert(false, "Allocation mapped too many times simultaneously.");
        }
    }

    public void BlockAllocUnmap()
    {
        Debug.Assert(Type == VmaAllocationType.Block);

        if (_mapCount > 0)
        {
            --_mapCount;
        }
        else
        {
            Debug.Assert(false, "Unmapping allocation not previously mapped.");
        }
    }

    internal VkResult DedicatedAllocMap(VmaAllocator allocator, void** ppData)
    {
        Debug.Assert(Type == VmaAllocationType.Dedicated);
        Debug.Assert(IsMappingAllowed, "Mapping is not allowed on this allocation! Please use one of the new VMA_ALLOCATION_CREATE_HOST_ACCESS_* flags when creating it.");

        if (_mapCount != 0 || IsPersistentMap)
        {
            if (_mapCount < 0xFF)
            {
                Debug.Assert(_dedicatedAllocationMappedData != null);
                *ppData = _dedicatedAllocationMappedData;
                ++_mapCount;
                return VkResult.Success;
            }
            else
            {
                Debug.Assert(false, "Dedicated allocation mapped too many times simultaneously.");
                return VkResult.ErrorMemoryMapFailed;
            }
        }
        else
        {
            VkResult result = vkMapMemory(
                allocator.Device,
                _dedicatedAllocationMemory,
                0, // offset
                VK_WHOLE_SIZE,
                0, // flags
                ppData);
            if (result == VkResult.Success)
            {
                _dedicatedAllocationMappedData = *ppData;
                _mapCount = 1;
            }

            return result;
        }
    }

    internal void DedicatedAllocUnmap(VmaAllocator allocator)
    {
        Debug.Assert(Type == VmaAllocationType.Dedicated);

        if (_mapCount > 0)
        {
            --_mapCount;
            if (_mapCount == 0 && !IsPersistentMap)
            {
                _dedicatedAllocationMappedData = null;
                vkUnmapMemory(allocator.Device, _dedicatedAllocationMemory);
            }
        }
        else
        {
            Debug.Assert(false, "Unmapping dedicated allocation not previously mapped.");
        }
    }

    [Flags]
    enum Flags : byte
    {
        None = 0,
        PersistentMap = 0x01,
        MappingAllowed = 0x02
    }
}

public readonly struct VmaAllocationInfo
{
    public uint MemoryType { get; init; }
    public VkDeviceMemory DeviceMemory { get; init; }
    public ulong Offset { get; init; }
    public ulong Size { get; init; }
    public unsafe void* pMappedData { get; init; }

    /// <summary>
    /// Size of the `VkDeviceMemory` block that the allocation belongs to.
    /// 
    /// In case of an allocation with dedicated memory, it will be equal to `<see cref="VmaAllocationInfo.Size"/>`.
    /// </summary>
    public ulong BlockSize { get; init; }
    /// <summary>
    /// <b>true</b> if the allocation has dedicated memory, <b>false</b> if it was placed as part of a larger memory block.
    ///
    /// When <b>true</b>, it also means `VkMemoryDedicatedAllocateInfo` was used when creating the allocation
    /// (if VK_KHR_dedicated_allocation extension or Vulkan version >= 1.1 is enabled).
    /// </summary>
    public bool DedicatedMemory { get; init; }
}

