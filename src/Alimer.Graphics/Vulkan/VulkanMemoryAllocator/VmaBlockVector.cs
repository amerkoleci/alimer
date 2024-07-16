// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using static Vortice.Vulkan.VmaUtils;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

internal enum VmaSuballocationType
{
    Free = 0,
    Unknown = 1,
    Buffer = 2,
    ImageUnknown = 3,
    ImageLinear = 4,
    ImageOptimal = 5
}

internal unsafe class VmaBlockVector : IDisposable
{
    private readonly VmaAllocator _allocator;
    private readonly ReaderWriterLockSlim _mutex = new(LockRecursionPolicy.NoRecursion);
    // Incrementally sorted by sumFreeSize, ascending.
    private readonly List<VmaDeviceMemoryBlock> _blocks = [];
    private uint _nextBlockId;
    private readonly void* _pMemoryAllocateNext;

    public VmaPool? ParentPool { get; }

    public int MemoryTypeIndex { get; }
    public ulong PreferredBlockSize { get; }

    public nuint MinBlockCount { get; }
    public nuint MaxBlockCount { get; }
    public ulong BufferImageGranularity { get; }

    public bool HasExplicitBlockSize { get; }
    public uint Algorithm { get; }
    public float Priority { get; }
    public ulong MinAllocationAlignment { get; }

    public bool IncrementalSort { get; set; } = true;

    public VmaBlockVector(VmaAllocator allocator,
        VmaPool? parentPool, 
        int memoryTypeIndex,
        ulong preferredBlockSize,
        nuint minBlockCount,
        nuint maxBlockCount,
        ulong bufferImageGranularity,
        bool hasExplicitBlockSize,
        uint algorithm,
        float priority,
        ulong minAllocationAlignment,
        void* pMemoryAllocateNext)
    {
        _allocator = allocator;
        ParentPool = parentPool;
        MemoryTypeIndex = memoryTypeIndex;
        PreferredBlockSize = preferredBlockSize;
        MinBlockCount = minBlockCount;
        MaxBlockCount = maxBlockCount;
        BufferImageGranularity = bufferImageGranularity;
        HasExplicitBlockSize = hasExplicitBlockSize;
        Algorithm = algorithm;
        MinAllocationAlignment = minAllocationAlignment;
        Priority = priority;
        _pMemoryAllocateNext = pMemoryAllocateNext;
    }

    public void Dispose()
    {
        for (int i = _blocks.Count; i-- != 0;)
        {
            _blocks[i].Destroy(_allocator);
            _blocks[i].Dispose();
        }
    }

    public void Free(VmaAllocation allocation)
    {
        VmaDeviceMemoryBlock? blockToDelete = default;

        bool budgetExceeded = false;
        {
            int heapIndex = _allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex);
            VmaBudget heapBudget;
            _allocator.GetHeapBudgets(&heapBudget, heapIndex, 1);
            budgetExceeded = heapBudget.Usage >= heapBudget.Budget;
        }

        // Scope for lock.
        {
            using VmaMutexLockWrite lockMutex = new(_mutex, _allocator.UseMutex);

            VmaDeviceMemoryBlock pBlock = allocation.GetBlock();

            if (IsCorruptionDetectionEnabled())
            {
                //VkResult res = pBlock->ValidateMagicValueAfterAllocation(m_hAllocator, hAllocation->GetOffset(), hAllocation->GetSize());
                //VMA_ASSERT(res == VK_SUCCESS && "Couldn't map block memory to validate magic value.");
            }

            if (allocation.IsPersistentMap)
            {
                pBlock.Unmap(_allocator, 1);
            }

            bool hadEmptyBlockBeforeFree = HasEmptyBlock();
            pBlock.MetaData.Free(allocation.GetAllocHandle());
            pBlock.PostFree(_allocator);
            Debug.Assert(pBlock.Validate());

            Debug.WriteLine($"  Freed from MemoryTypeIndex={MemoryTypeIndex}");

            bool canDeleteBlock = _blocks.Count > unchecked((int)MinBlockCount);
            // pBlock became empty after this deallocation.
            if (pBlock.MetaData.IsEmpty)
            {
                // Already had empty block. We don't want to have two, so delete this one.
                if ((hadEmptyBlockBeforeFree || budgetExceeded) && canDeleteBlock)
                {
                    blockToDelete = pBlock;
                    Remove(pBlock);
                }
                // else: We now have one empty block - leave it. A hysteresis to avoid allocating whole block back and forth.
            }
            // pBlock didn't become empty, but we have another empty block - find and free that one.
            // (This is optional, heuristics.)
            else if (hadEmptyBlockBeforeFree && canDeleteBlock)
            {
                VmaDeviceMemoryBlock pLastBlock = _blocks[_blocks.Count - 1]; // m_Blocks.back();
                if (pLastBlock.MetaData.IsEmpty)
                {
                    blockToDelete = pLastBlock;
                    //m_Blocks.pop_back();
                    _blocks.RemoveAt(_blocks.Count - 1);
                }
            }

            IncrementallySortBlocks();
        }

        // Destruction of a free block. Deferred until this point, outside of mutex
        // lock, for performance reason.
        if (blockToDelete != null)
        {
            //VMA_DEBUG_LOG_FORMAT("    Deleted empty block #%" PRIu32, pBlockToDelete->GetId());
            blockToDelete.Destroy(_allocator);
            blockToDelete.Dispose();
            //vma_delete(m_hAllocator, pBlockToDelete);
        }

        _allocator._budget.RemoveAllocation(_allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex), allocation.Size);
        //_allocator->m_AllocationObjectAllocator.Free(hAllocation);
        allocation.Dispose();
    }

    public void AddStatistics(ref VmaStatistics stats)
    {
        using VmaMutexLockRead lockMutex = new(_mutex, _allocator.UseMutex);

        int blockCount = _blocks.Count;
        for (int blockIndex = 0; blockIndex < blockCount; ++blockIndex)
        {
            VmaDeviceMemoryBlock pBlock = _blocks[blockIndex];
            Debug.Assert(pBlock != null);
            Debug.Assert(pBlock.Validate());
            pBlock.MetaData.AddStatistics(ref stats);
        }
    }

    public void AddDetailedStatistics(ref VmaDetailedStatistics stats)
    {
        using VmaMutexLockRead lockRead = new(_mutex, _allocator.UseMutex);

        int blockCount = _blocks.Count;
        for (int blockIndex = 0; blockIndex < blockCount; ++blockIndex)
        {
            VmaDeviceMemoryBlock pBlock = _blocks[blockIndex];
            Debug.Assert(pBlock != null);
            Debug.Assert(pBlock.Validate());
            pBlock.MetaData.AddDetailedStatistics(ref stats);
        }
    }

    private void Remove(VmaDeviceMemoryBlock pBlock)
    {
        for (int blockIndex = 0; blockIndex < _blocks.Count; ++blockIndex)
        {
            if (_blocks[blockIndex] == pBlock)
            {
                //VmaVectorRemove(m_Blocks, blockIndex);
                _blocks.RemoveAt(blockIndex);
                return;
            }
        }
        Debug.Assert(false);
    }

    public VkResult CreateMinBlocks()
    {
        for (nuint i = 0; i < MinBlockCount; ++i)
        {
            VkResult result = CreateBlock(PreferredBlockSize, out _);

            if (result != VkResult.Success)
            {
                return result;
            }
        }

        return VkResult.Success;
    }

    private VkResult CreateBlock(ulong blockSize, out int newBlockIndex)
    {
        VkMemoryAllocateInfo allocInfo = new()
        {
            pNext = _pMemoryAllocateNext,
            memoryTypeIndex = (uint)MemoryTypeIndex,
            allocationSize = blockSize
        };

        // Every standalone block can potentially contain a buffer with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT - always enable the feature.
        VkMemoryAllocateFlagsInfo allocFlagsInfo = new();
        if (_allocator.UseKhrBufferDeviceAddress)
        {
            allocFlagsInfo.flags = VkMemoryAllocateFlags.DeviceAddress;
            VmaPnextChainPushFront(&allocInfo, &allocFlagsInfo);
        }

        VkMemoryPriorityAllocateInfoEXT priorityInfo = new();
        if (_allocator.UseExtMemoryPriority)
        {
            Debug.Assert(Priority >= 0.0f && Priority <= 1.0f);
            priorityInfo.priority = Priority;

            priorityInfo.pNext = allocInfo.pNext;
            allocInfo.pNext = &priorityInfo;
        }

        // Attach VkExportMemoryAllocateInfoKHR if necessary.
        VkExportMemoryAllocateInfo exportMemoryAllocInfo = new();
        exportMemoryAllocInfo.handleTypes = _allocator.GetExternalMemoryHandleTypeFlags(MemoryTypeIndex);
        if (exportMemoryAllocInfo.handleTypes != 0)
        {
            exportMemoryAllocInfo.pNext = allocInfo.pNext;
            allocInfo.pNext = &exportMemoryAllocInfo;
        }

        VkResult res = _allocator.AllocateVulkanMemory(allocInfo, out VkDeviceMemory memory);
        if (res < 0)
        {
            newBlockIndex = -1;
            return res;
        }

        // New VkDeviceMemory successfully created.

        // Create new Allocation for it.
        VmaDeviceMemoryBlock block = new(_allocator, ParentPool, MemoryTypeIndex, memory,
            allocInfo.allocationSize, _nextBlockId++, Algorithm, BufferImageGranularity);

        _blocks.Add(block);
        newBlockIndex = _blocks.Count - 1;

        return VkResult.Success;
    }

    public VkResult Allocate(ulong size, ulong alignment, in VmaAllocationCreateInfo createInfo, VmaSuballocationType suballocType, out VmaAllocation? allocation)
    {
        allocation = default;

        VkResult res = VkResult.Success;

        alignment = Math.Max(alignment, MinAllocationAlignment);

        //if (IsCorruptionDetectionEnabled())
        //{
        //    size = VmaAlignUp<VkDeviceSize>(size, sizeof(VMA_CORRUPTION_DETECTION_MAGIC_VALUE));
        //    alignment = VmaAlignUp<VkDeviceSize>(alignment, sizeof(VMA_CORRUPTION_DETECTION_MAGIC_VALUE));
        //}

        {
            using VmaMutexLockWrite lockLock = new(_mutex, _allocator.UseMutex);

            int allocIndex = 0;
            int allocationCount = 1;
            for (allocIndex = 0; allocIndex < allocationCount; ++allocIndex)
            {
                res = AllocatePage(
                    size,
                    alignment,
                    createInfo,
                    suballocType,
                    out allocation);
                if (res != VK_SUCCESS)
                {
                    break;
                }
            }

        }

        if (res != VkResult.Success)
        {
            // Free the already created allocation.
            Free(allocation!);
        }

        return res;
    }

    private VkResult AllocatePage(ulong size, ulong alignment, in VmaAllocationCreateInfo createInfo, VmaSuballocationType suballocType, out VmaAllocation? allocation)
    {
        allocation = default;
        bool isUpperAddress = (createInfo.Flags & VmaAllocationCreateFlags.UpperAddress) != 0;

        ulong freeMemory;
        {
            int heapIndex = _allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex);
            Span<VmaBudget> heapBudget = stackalloc VmaBudget[1];
            _allocator.GetHeapBudgets(heapBudget, heapIndex, 1);
            freeMemory = (heapBudget[0].Usage < heapBudget[0].Budget) ? (heapBudget[0].Budget - heapBudget[0].Usage) : 0;
        }

        bool canFallbackToDedicated = !HasExplicitBlockSize
           && (createInfo.Flags & VmaAllocationCreateFlags.NeverAllocate) == 0;
        bool canCreateNewBlock =
            ((createInfo.Flags & VmaAllocationCreateFlags.NeverAllocate) == 0)
            && (_blocks.Count < unchecked((uint)MaxBlockCount))
            && (freeMemory >= size || !canFallbackToDedicated);
        uint strategy = (uint)(createInfo.Flags & VmaAllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MASK);

        // Upper address can only be used with linear allocator and within single memory block.
        if (isUpperAddress &&
            (Algorithm != VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT || MaxBlockCount > 1))
        {
            return VkResult.ErrorFeatureNotPresent;
        }

        // Early reject: requested allocation size is larger that maximum block size for this block vector.
        if (size + VmaAllocator.DebugMargin > PreferredBlockSize)
        {
            return VkResult.ErrorOutOfDeviceMemory;
        }

        // 1. Search existing allocations. Try to allocate.
        if (Algorithm == VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT)
        {
            // Use only last block.
            //if (!_blocks.empty())
            //{
            //    VmaDeviceMemoryBlock * const pCurrBlock = m_Blocks.back();
            //    VMA_ASSERT(pCurrBlock);
            //    VkResult res = AllocateFromBlock(
            //        pCurrBlock, size, alignment, createInfo.flags, createInfo.pUserData, suballocType, strategy, pAllocation);
            //    if (res == VK_SUCCESS)
            //    {
            //        VMA_DEBUG_LOG_FORMAT("    Returned from last block #%" PRIu32, pCurrBlock->GetId());
            //        IncrementallySortBlocks();
            //        return VK_SUCCESS;
            //    }
            //}
        }
        else
        {
            if (strategy != (uint)VmaAllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT) // MIN_MEMORY or default
            {
                bool isHostVisible = (_allocator.PhysicalDeviceMemoryProperties.memoryTypes[MemoryTypeIndex].propertyFlags & VkMemoryPropertyFlags.HostVisible) != 0;
                if (isHostVisible)
                {
                    bool isMappingAllowed = (createInfo.Flags & (VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.HostAccessRandom)) != 0;
                    // For non-mappable allocations, check blocks that are not mapped first.
                    // For mappable allocations, check blocks that are already mapped first.
                    // This way, having many blocks, we will separate mappable and non-mappable allocations,
                    // hopefully limiting the number of blocks that are mapped, which will help tools like RenderDoc.
                    for (int mappingI = 0; mappingI < 2; ++mappingI)
                    {
                        // Forward order in m_Blocks - prefer blocks with smallest amount of free space.
                        for (int blockIndex = 0; blockIndex < _blocks.Count; ++blockIndex)
                        {
                            VmaDeviceMemoryBlock pCurrBlock = _blocks[blockIndex];
                            Debug.Assert(pCurrBlock != null);
#if TODO
                        bool isBlockMapped = pCurrBlock->GetMappedData() != VMA_NULL;
                        if ((mappingI == 0) == (isMappingAllowed == isBlockMapped))
                        {
                            VkResult res = AllocateFromBlock(
                                pCurrBlock, size, alignment, createInfo.flags, createInfo.pUserData, suballocType, strategy, pAllocation);
                            if (res == VK_SUCCESS)
                            {
                                //Debug.WriteLine("    Returned from existing block #%" PRIu32, pCurrBlock->GetId());
                                IncrementallySortBlocks();
                                return VK_SUCCESS;
                            }
                        } 
#endif
                        }
                    }
                }
                else
                {
                    // Forward order in m_Blocks - prefer blocks with smallest amount of free space.
                    for (int blockIndex = 0; blockIndex < _blocks.Count; ++blockIndex)
                    {
                        VmaDeviceMemoryBlock pCurrBlock = _blocks[blockIndex];
                        Debug.Assert(pCurrBlock != null);
#if TODO
                    VkResult res = AllocateFromBlock(
                                            pCurrBlock, size, alignment, createInfo.Flags, createInfo.Usage, suballocType, strategy, out allocation);
                    if (res == VK_SUCCESS)
                    {
                        //Debug.WriteLine("    Returned from existing block #%" PRIu32, pCurrBlock->GetId());
                        IncrementallySortBlocks();
                        return VK_SUCCESS;
                    } 
#endif
                    }
                }
            }
            else // VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT
            {
                // Backward order in m_Blocks - prefer blocks with largest amount of free space.
                for (int blockIndex = _blocks.Count - 1; blockIndex >= 0; blockIndex--)
                {
                    VmaDeviceMemoryBlock pCurrBlock = _blocks[blockIndex];
                    Debug.Assert(pCurrBlock != null);
#if TODO
                VkResult res = AllocateFromBlock(pCurrBlock, size, alignment, createInfo.flags, createInfo.pUserData, suballocType, strategy, pAllocation);
                if (res == VK_SUCCESS)
                {
                    //Debug.WriteLine("    Returned from existing block #%" PRIu32, pCurrBlock->GetId());
                    IncrementallySortBlocks();
                    return VK_SUCCESS;
                } 
#endif
                }
            }
        }

        // 2. Try to create new block.
        if (canCreateNewBlock)
        {
            // Calculate optimal size for new block.
            ulong newBlockSize = PreferredBlockSize;
            uint newBlockSizeShift = 0;
            const uint NEW_BLOCK_SIZE_SHIFT_MAX = 3;

            if (!HasExplicitBlockSize)
            {
                // Allocate 1/8, 1/4, 1/2 as first blocks.
                ulong maxExistingBlockSize = CalcMaxBlockSize();
                for (uint i = 0; i < NEW_BLOCK_SIZE_SHIFT_MAX; ++i)
                {
                    ulong smallerNewBlockSize = newBlockSize / 2;
                    if (smallerNewBlockSize > maxExistingBlockSize && smallerNewBlockSize >= size * 2)
                    {
                        newBlockSize = smallerNewBlockSize;
                        ++newBlockSizeShift;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            int newBlockIndex = 0;
            VkResult res = (newBlockSize <= freeMemory || !canFallbackToDedicated) ? CreateBlock(newBlockSize, out newBlockIndex) : VkResult.ErrorOutOfDeviceMemory;

            // Allocation of this size failed? Try 1/2, 1/4, 1/8 of m_PreferredBlockSize.
            if (!HasExplicitBlockSize)
            {
                while (res < 0 && newBlockSizeShift < NEW_BLOCK_SIZE_SHIFT_MAX)
                {
                    ulong smallerNewBlockSize = newBlockSize / 2;
                    if (smallerNewBlockSize >= size)
                    {
                        newBlockSize = smallerNewBlockSize;
                        ++newBlockSizeShift;
                        res = (newBlockSize <= freeMemory || !canFallbackToDedicated) ?
                            CreateBlock(newBlockSize, out newBlockIndex) : VkResult.ErrorOutOfDeviceMemory;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if (res == VkResult.Success)
            {
                VmaDeviceMemoryBlock block = _blocks[newBlockIndex];
                Debug.Assert(block.MetaData.Size >= size);

                res = AllocateFromBlock(block, size, alignment, createInfo.Flags, createInfo.UserData, suballocType, strategy, out allocation);
                if (res == VK_SUCCESS)
                {
                    //Debug.WriteLine("    Created new block #%" PRIu32 " Size=%" PRIu64, pBlock->GetId(), newBlockSize);
                    IncrementallySortBlocks();
                    return VK_SUCCESS;
                }
                else
                {
                    // Allocation from new block failed, possibly due to VMA_DEBUG_MARGIN or alignment.
                    return VkResult.ErrorOutOfDeviceMemory;
                }
            }
        }

        return VkResult.ErrorOutOfDeviceMemory;
    }

    private VkResult AllocateFromBlock(VmaDeviceMemoryBlock block, ulong size, ulong alignment,
        VmaAllocationCreateFlags allocFlags,
        object? userData,
        VmaSuballocationType suballocType,
        uint strategy,
        out VmaAllocation? allocation)
    {
        bool isUpperAddress = (allocFlags & VmaAllocationCreateFlags.UpperAddress) != 0;

        AllocationRequest currRequest = default;
        if (block.MetaData.CreateAllocationRequest(
            size,
            alignment,
            isUpperAddress,
            suballocType,
            strategy,
            &currRequest))
        {
            return CommitAllocationRequest(currRequest, block, alignment, allocFlags, userData, suballocType, out allocation);
        }

        allocation = default;
        return VkResult.ErrorOutOfDeviceMemory;
    }

    private VkResult CommitAllocationRequest(in AllocationRequest allocRequest,
        VmaDeviceMemoryBlock pBlock,
        ulong alignment,
        VmaAllocationCreateFlags allocFlags,
        object? userData,
        VmaSuballocationType suballocType,
        out VmaAllocation? allocation)
    {
        bool mapped = (allocFlags & VmaAllocationCreateFlags.Mapped) != 0;
        //bool isUserDataString = (allocFlags & AllocationCreateFlags.u VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT) != 0;
        bool isMappingAllowed = (allocFlags & (VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.HostAccessRandom)) != 0;

        pBlock.PostAlloc(_allocator);
        // Allocate from pCurrBlock.
        if (mapped)
        {
            VkResult res = pBlock.Map(_allocator, 1, null);
            if (res != VkResult.Success)
            {
                allocation = default;
                return res;
            }
        }

        //        *pAllocation = m_hAllocator->m_AllocationObjectAllocator.Allocate(isMappingAllowed);
        allocation = new(isMappingAllowed);
        pBlock.MetaData.Alloc(allocRequest, suballocType, allocation);
        allocation.InitBlockAllocation(
            pBlock,
            allocRequest.allocHandle,
            alignment,
            allocRequest.size, // Not size, as actual allocation size may be larger than requested!
            MemoryTypeIndex,
            suballocType,
            mapped);

        //Debug.Assert(pBlock.Validate());
        //        if (isUserDataString)
        //            (*pAllocation)->SetName(m_hAllocator, (const char*)pUserData);
        //else
        //            (*pAllocation)->SetUserData(m_hAllocator, pUserData);

        _allocator._budget.AddAllocation(_allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex), allocRequest.size);
        //        if (VMA_DEBUG_INITIALIZE_ALLOCATIONS)
        //        {
        //            m_hAllocator->FillAllocation(*pAllocation, VMA_ALLOCATION_FILL_PATTERN_CREATED);
        //        }
        //        if (IsCorruptionDetectionEnabled())
        //        {
        //            VkResult res = pBlock->WriteMagicValueAfterAllocation(m_hAllocator, (*pAllocation)->GetOffset(), allocRequest.size);
        //            VMA_ASSERT(res == VK_SUCCESS && "Couldn't map block memory to write magic value.");
        //        }

        return VkResult.Success;
    }

    private ulong CalcMaxBlockSize()
    {
        ulong result = 0;

        for (int i = _blocks.Count - 1; i >= 0; --i)
        {
            ulong blockSize = _blocks[i].MetaData.Size;

            if (result < blockSize)
            {
                result = blockSize;
            }

            if (result >= PreferredBlockSize)
            {
                break;
            }
        }

        return result;
    }
    private void IncrementallySortBlocks()
    {
        if (!IncrementalSort)
            return;

        if (Algorithm != VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT)
        {
            // Bubble sort only until first swap.
            for (int i = 1; i < _blocks.Count; ++i)
            {
                if (_blocks[i - 1].MetaData.GetSumFreeSize() > _blocks[i].MetaData.GetSumFreeSize())
                {
                    //Helpers.VMA_SWAP(ref _blocks[i - 1], ref _blocks[i]);
                    VmaDeviceMemoryBlock tmp = _blocks[i - 1];
                    _blocks[i - 1] = _blocks[i];
                    _blocks[i] = tmp;
                    return;
                }
            }
        }
    }

    private bool HasEmptyBlock()
    {
        for (int index = 0, count = _blocks.Count; index < count; ++index)
        {
            VmaDeviceMemoryBlock pBlock = _blocks[index];
            if (pBlock.MetaData.IsEmpty)
            {
                return true;
            }
        }
        return false;
    }

    public bool IsCorruptionDetectionEnabled()
    {
        return false;
        //uint requiredMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        //return (VMA_DEBUG_DETECT_CORRUPTION != 0) &&
        //    (DebugMargin > 0) &&
        //    (m_Algorithm == 0 || m_Algorithm == VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT) &&
        //    (m_hAllocator->m_MemProps.memoryTypes[m_MemoryTypeIndex].propertyFlags & requiredMemFlags) == requiredMemFlags;
    }
}
