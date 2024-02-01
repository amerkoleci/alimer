// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using Vortice.Mathematics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanAllocator : IDisposable
{
    const uint VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT = 0x00000004;

    private const uint VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD_COPY = 0x00000040;
    private const uint VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD_COPY = 0x00000080;
    private const uint VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_COPY = 0x00020000;
    private const uint VK_IMAGE_CREATE_DISJOINT_BIT_COPY = 0x00000200;
    private const int VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT_COPY = 1000158000;
    private const uint VMA_ALLOCATION_INTERNAL_STRATEGY_MIN_OFFSET = 0x10000000u;
    private const uint VMA_ALLOCATION_TRY_COUNT = 32;
    private const uint VMA_VENDOR_ID_AMD = 4098;

    private const long DebugMargin = 0;                         // VMA_DEBUG_MARGIN
    private const long MinAlignment = 1;                        // VMA_MIN_ALIGNMENT
    private const long DebugMinBufferImageGranularity = 1;

    /// Maximum size of a memory heap in Vulkan to consider it "small".
    private const ulong SmallHeapMaxSize = 1024 * 1024 * 1024; // VMA_SMALL_HEAP_MAX_SIZE

    /// Default size of a block allocated as single VkDeviceMemory from a "large" heap.
    private const ulong DefaultLargeHeapBlockSize = 256 * 1024 * 1024; // VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE

    private readonly VkDevice _device;
    private readonly VkPhysicalDevice _physicalDevice;
    private readonly VkPhysicalDeviceProperties _physicalDeviceProperties;
    private readonly VkPhysicalDeviceMemoryProperties _memoryProperties;
    private readonly ulong _preferredLargeHeapBlockSize;

    private readonly bool _useAmdDeviceCoherentMemory;

    private readonly BlockVector[] _blockVectors = new BlockVector[VK_MAX_MEMORY_TYPES]; //Default Pools
    private readonly DedicatedAllocationList[] _dedicatedAllocations = new DedicatedAllocationList[VK_MAX_MEMORY_TYPES];

    private readonly CurrentBudgetData _budget = new();
    private uint _deviceMemoryCount; // Total number of VkDeviceMemory objects.

    // Each bit (1 << i) is set if HeapSizeLimit is enabled for that heap, so cannot allocate more than the heap size.
    private readonly uint _heapSizeLimitMask;


    public VulkanAllocator(VkDevice device, VkPhysicalDevice physicalDevice, ulong preferredLargeHeapBlockSize = 0)
    {
        _device = device;
        _physicalDevice = physicalDevice;

        vkGetPhysicalDeviceProperties(physicalDevice, out _physicalDeviceProperties);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, out _memoryProperties);

        Debug.Assert(MathHelper.IsPow2(MinAlignment));
        Debug.Assert(MathHelper.IsPow2(DebugMinBufferImageGranularity));
        Debug.Assert(MathHelper.IsPow2(_physicalDeviceProperties.limits.bufferImageGranularity));
        Debug.Assert(MathHelper.IsPow2(_physicalDeviceProperties.limits.nonCoherentAtomSize));

        _preferredLargeHeapBlockSize = (preferredLargeHeapBlockSize != 0) ? preferredLargeHeapBlockSize : DefaultLargeHeapBlockSize;

        UseKhrDedicatedAllocation = true;
        GlobalMemoryTypeBits = CalculateGlobalMemoryTypeBits();
        for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
        {
            // Create only supported types
            if ((GlobalMemoryTypeBits & (1u << memTypeIndex)) != 0)
            {
                ulong preferredBlockSize = CalcPreferredBlockSize(memTypeIndex);
                _blockVectors[memTypeIndex] = new BlockVector(
                    this,
                    null, // hParentPool
                    memTypeIndex,
                    preferredBlockSize,
                    0,
                    nuint.MaxValue,
                    BufferImageGranularity,
                    false, // explicitBlockSize
                    0, // algorithm
                    0.5f, // priority (0.5 is the default per Vulkan spec)
                    GetMemoryTypeMinAlignment(memTypeIndex), // minAllocationAlignment
                    null); // // pMemoryAllocateNext
                // No need to call m_pBlockVectors[memTypeIndex][blockVectorTypeIndex]->CreateMinBlocks here,
                // becase minBlockCount is 0.
            }
        }
    }

    public uint MemoryHeapCount => _memoryProperties.memoryHeapCount;

    public uint MemoryTypeCount => _memoryProperties.memoryTypeCount;

    public ulong BufferImageGranularity => Math.Max(DebugMinBufferImageGranularity, _physicalDeviceProperties.limits.bufferImageGranularity);

    public uint GlobalMemoryTypeBits { get; }

    public bool UseKhrDedicatedAllocation { get; set; }
    public bool UseKhrBufferDeviceAddress { get; set; }

    public ref readonly VkPhysicalDeviceProperties PhysicalDeviceProperties => ref _physicalDeviceProperties;
    public ref readonly VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties => ref _memoryProperties;

    public void Dispose()
    {
    }

    public int MemoryTypeIndexToHeapIndex(int memoryTypeIndex)
    {
        Debug.Assert(memoryTypeIndex < _memoryProperties.memoryTypeCount);

        return (int)_memoryProperties.memoryTypes[memoryTypeIndex].heapIndex;
    }

    /// <summary>
    /// True when specific memory type is HOST_VISIBLE but not HOST_COHERENT.
    /// </summary>
    /// <param name="memoryTypeIndex"></param>
    /// <returns></returns>
    public bool IsMemoryTypeNonCoherent(int memoryTypeIndex)
    {
        return (_memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & (VkMemoryPropertyFlags.HostVisible | VkMemoryPropertyFlags.HostCoherent)) == VkMemoryPropertyFlags.HostVisible;
    }

    /// <summary>
    /// Minimum alignment for all allocations in specific memory type.
    /// </summary>
    /// <param name="memoryTypeIndex"></param>
    /// <returns></returns>
    public ulong GetMemoryTypeMinAlignment(int memoryTypeIndex)
    {
        return IsMemoryTypeNonCoherent(memoryTypeIndex) ?
            Math.Max(MinAlignment, _physicalDeviceProperties.limits.nonCoherentAtomSize) :
            MinAlignment;
    }

    public bool IsIntegratedGpu() => _physicalDeviceProperties.deviceType == VkPhysicalDeviceType.IntegratedGpu;

    public MemoryBudget[] GetHeapBudgets(int firstHeap, int heapCount)
    {
        // TODO: Add m_UseExtMemoryBudget
        Span<MemoryBudget> budgets = stackalloc MemoryBudget[heapCount];

        for (int i = 0; i < heapCount; ++i)
        {
            int heapIndex = firstHeap + i;

            budgets[i].Statistics.BlockCount = _budget.BlockCount[heapIndex];
            budgets[i].Statistics.AllocationCount = _budget.AllocationCount[heapIndex];
            budgets[i].Statistics.BlockBytes = _budget.BlockBytes[heapIndex];
            budgets[i].Statistics.AllocationBytes = _budget.AllocationBytes[heapIndex];

            budgets[i].Usage = budgets[i].Statistics.BlockBytes;
            budgets[i].Budget = _memoryProperties.memoryHeaps[heapIndex].size * 8 / 10; // 80% heuristics.
        }

        return budgets.ToArray();
    }

    public VkResult CreateBuffer(VkBufferCreateInfo createInfo,
        AllocationCreateInfo allocationCreateInfo,
        out VkBuffer buffer,
        out Allocation? allocation)
    {
        buffer = VkBuffer.Null;
        allocation = default;

        if (createInfo.size == 0)
        {
            return VkResult.ErrorInitializationFailed;
        }

        //if (((uint)createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_COPY) != 0 &&
        //    !allocator->m_UseKhrBufferDeviceAddress)
        //{
        //    Log.Error("Creating a buffer with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT is not valid if VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT was not used.");
        //    return VkResult.ErrorInitializationFailed;
        //}

        VkResult result = vkCreateBuffer(_device.Handle, &createInfo, null, out buffer);
        if (result >= VkResult.Success)
        {
            // 2. vkGetBufferMemoryRequirements.
            GetBufferMemoryRequirements(buffer, out VkMemoryRequirements vkMemReq, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation);

            // 3. Allocate memory using allocator.
            result = AllocateMemory(ref allocationCreateInfo, in vkMemReq, requiresDedicatedAllocation, prefersDedicatedAllocation,
                buffer, // dedicatedBuffer
                VkImage.Null, // dedicatedImage
                (uint)createInfo.usage, // dedicatedBufferImageUsage
                SuballocationType.Buffer,
                out allocation);

            if (result >= 0)
            {
                // 3. Bind buffer with memory.
                if ((allocationCreateInfo.Flags & AllocationCreateFlags.DontBind) == 0)
                {
                    //result = allocator->BindBufferMemory(*pAllocation, 0, *pBuffer, VMA_NULL);
                }

                if (result >= 0)
                {
                    // All steps succeeded.
#if VMA_STATS_STRING_ENABLED
                    (*pAllocation)->InitBufferImageUsage(pBufferCreateInfo->usage);
#endif
                    //if (pAllocationInfo != VMA_NULL)
                    //{
                    //    allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
                    //}

                    return VkResult.Success;
                }

                //allocator->FreeMemory(
                //    1, // allocationCount
                //    pAllocation);

                allocation = default;
                vkDestroyBuffer(_device, buffer, null);
                buffer = VkBuffer.Null;

                return result;
            }

            vkDestroyBuffer(_device, buffer, null);
            buffer = VkBuffer.Null;
            return result;
        }

        return result;
    }

    public void GetBufferMemoryRequirements(VkBuffer buffer, out VkMemoryRequirements memReq, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation)
    {
        // UseKhrDedicatedAllocation is always true
        if (UseKhrDedicatedAllocation)
        {
            VkBufferMemoryRequirementsInfo2 memReqInfo = new();
            memReqInfo.buffer = buffer;

            VkMemoryDedicatedRequirements memDedicatedReq = new();

            VkMemoryRequirements2 memReq2 = new VkMemoryRequirements2();
            //VmaPnextChainPushFront(&memReq2, &memDedicatedReq);
            memDedicatedReq.pNext = memReq2.pNext;
            memReq2.pNext = &memDedicatedReq;

            vkGetBufferMemoryRequirements2(_device, &memReqInfo, &memReq2);

            memReq = memReq2.memoryRequirements;
            requiresDedicatedAllocation = memDedicatedReq.requiresDedicatedAllocation != VkBool32.False;
            prefersDedicatedAllocation = memDedicatedReq.prefersDedicatedAllocation != VkBool32.False;
        }
        else
        {
            vkGetBufferMemoryRequirements(_device, buffer, out memReq);
            requiresDedicatedAllocation = false;
            prefersDedicatedAllocation = false;
        }
    }

    public VkResult FindMemoryTypeIndex(uint memoryTypeBits,
        in AllocationCreateInfo createInfo,
        uint bufferOrImageUsage,
        out int memoryTypeIndex)
    {
        memoryTypeBits &= GlobalMemoryTypeBits;

        if (createInfo.MemoryTypeBits != 0)
        {
            memoryTypeBits &= createInfo.MemoryTypeBits;
        }

        if (!FindMemoryPreferences(
            IsIntegratedGpu(),
            in createInfo,
            bufferOrImageUsage,
            out VkMemoryPropertyFlags requiredFlags, out VkMemoryPropertyFlags preferredFlags, out VkMemoryPropertyFlags notPreferredFlags))
        {
            memoryTypeIndex = int.MaxValue;
            return VkResult.ErrorFeatureNotPresent;
        }

        memoryTypeIndex = int.MaxValue;
        int minCost = int.MaxValue;
        for (int memTypeIndex = 0, memTypeBit = 1; memTypeIndex < MemoryTypeCount; ++memTypeIndex, memTypeBit <<= 1)
        {
            // This memory type is acceptable according to memoryTypeBits bitmask.
            if ((memTypeBit & memoryTypeBits) != 0)
            {
                VkMemoryPropertyFlags currFlags = _memoryProperties.memoryTypes[memTypeIndex].propertyFlags;
                // This memory type contains requiredFlags.
                if ((requiredFlags & ~currFlags) == 0)
                {
                    // Calculate cost as number of bits from preferredFlags not present in this memory type.
                    int currCost =
                        BitOperations.PopCount((uint)(preferredFlags & ~currFlags)) +
                        BitOperations.PopCount((uint)(currFlags & notPreferredFlags));
                    // Remember memory type with lowest cost.
                    if (currCost < minCost)
                    {
                        memoryTypeIndex = memTypeIndex;
                        if (currCost == 0)
                        {
                            return VkResult.Success;
                        }
                        minCost = currCost;
                    }
                }
            }
        }
        return (memoryTypeIndex != int.MaxValue) ? VkResult.Success : VkResult.ErrorFeatureNotPresent;
    }

    public VkResult AllocateVulkanMemory(VkMemoryAllocateInfo allocateInfo, out VkDeviceMemory memory)
    {
        //AtomicTransactionalIncrement<VMA_ATOMIC_UINT32> deviceMemoryCountIncrement;
        //const uint64_t prevDeviceMemoryCount = deviceMemoryCountIncrement.Increment(&m_DeviceMemoryCount);
#if VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT
    if(prevDeviceMemoryCount >= m_PhysicalDeviceProperties.limits.maxMemoryAllocationCount)
    {
        return VK_ERROR_TOO_MANY_OBJECTS;
    }
#endif

        memory = VkDeviceMemory.Null;
        int heapIndex = MemoryTypeIndexToHeapIndex((int)allocateInfo.memoryTypeIndex);

        // HeapSizeLimit is in effect for this heap.
        if ((_heapSizeLimitMask & (1u << (int)heapIndex)) != 0)
        {
            ulong heapSize = _memoryProperties.memoryHeaps[heapIndex].size;
            ulong blockBytes = _budget.BlockBytes[heapIndex];
            for (; ; )
            {
                ulong blockBytesAfterAllocation = blockBytes + allocateInfo.allocationSize;
                if (blockBytesAfterAllocation > heapSize)
                {
                    return VkResult.ErrorOutOfDeviceMemory;
                }

                // if (_budget.m_BlockBytes[heapIndex].compare_exchange_strong(blockBytes, blockBytesAfterAllocation))
                if (Interlocked.CompareExchange(ref _budget.BlockBytes[heapIndex], blockBytesAfterAllocation, blockBytes) != blockBytes)
                {
                    break;
                }
            }
        }
        else
        {
            Interlocked.Add(ref _budget.BlockBytes[heapIndex], allocateInfo.allocationSize);
        }
        Interlocked.Increment(ref _budget.BlockCount[heapIndex]);

        // VULKAN CALL vkAllocateMemory.
        VkResult res = vkAllocateMemory(_device, &allocateInfo, null, out memory);

        if (res == VK_SUCCESS)
        {
            Interlocked.Increment(ref _budget.OperationsSinceBudgetFetch);

            // Informative callback.
            //if (m_DeviceMemoryCallbacks.pfnAllocate != VMA_NULL)
            //{
            //    (*m_DeviceMemoryCallbacks.pfnAllocate)(this, pAllocateInfo->memoryTypeIndex, *pMemory, pAllocateInfo->allocationSize, m_DeviceMemoryCallbacks.pUserData);
            //}
            //
            //deviceMemoryCountIncrement.Commit();
        }
        else
        {
            Interlocked.Decrement(ref _budget.BlockCount[heapIndex]);
            //Interlocked.Add(ref _budget.BlockBytes[heapIndex], -(long)allocateInfo.allocationSize);
        }

        return res;
    }

    private static bool FindMemoryPreferences(
        bool isIntegratedGPU,
        in AllocationCreateInfo createInfo,
        uint bufImgUsage, // VkBufferCreateInfo::usage or VkImageCreateInfo::usage. UINT32_MAX if unknown.
        out VkMemoryPropertyFlags requiredFlags,
        out VkMemoryPropertyFlags preferredFlags,
        out VkMemoryPropertyFlags notPreferredFlags)
    {
        requiredFlags = createInfo.RequiredFlags;
        preferredFlags = createInfo.PreferredFlags;
        notPreferredFlags = 0;

        switch (createInfo.Usage)
        {
            case MemoryUsage.Unknown:
                break;
            case MemoryUsage.GpuOnly:
                if (!isIntegratedGPU || (preferredFlags & VkMemoryPropertyFlags.HostVisible) == 0)
                {
                    preferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                }
                break;
            case MemoryUsage.CpuOnly:
                requiredFlags |= VkMemoryPropertyFlags.HostVisible | VkMemoryPropertyFlags.HostCoherent;
                break;
            case MemoryUsage.CpuToGpu:
                requiredFlags |= VkMemoryPropertyFlags.HostVisible;
                if (!isIntegratedGPU || (preferredFlags & VkMemoryPropertyFlags.HostVisible) == 0)
                {
                    preferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                }
                break;
            case MemoryUsage.GpuToCpu:
                requiredFlags |= VkMemoryPropertyFlags.HostVisible;
                preferredFlags |= VkMemoryPropertyFlags.HostCached;
                break;
            case MemoryUsage.CpuCopy:
                notPreferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                break;
            case MemoryUsage.GpuLazilyAllocated:
                requiredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
                break;
            case MemoryUsage.Auto:
            case MemoryUsage.AutoPreferDevice:
            case MemoryUsage.AutoPreferHost:
                {
                    if (bufImgUsage == uint.MaxValue)
                    {
                        Log.Error("VMA_MEMORY_USAGE_AUTO* values can only be used with functions like vmaCreateBuffer, vmaCreateImage so that the details of the created resource are known.");
                        return false;
                    }

                    // This relies on values of VK_IMAGE_USAGE_TRANSFER* being the same VK_BUFFER_IMAGE_TRANSFER*.
                    bool deviceAccess = ((VkBufferUsageFlags)bufImgUsage & ~(VkBufferUsageFlags.TransferDst | VkBufferUsageFlags.TransferSrc)) != 0;
                    bool hostAccessSequentialWrite = (createInfo.Flags & AllocationCreateFlags.HostAccessSequentialWrite) != 0;
                    bool hostAccessRandom = (createInfo.Flags & AllocationCreateFlags.HostAccessRandom) != 0;
                    bool hostAccessAllowTransferInstead = (createInfo.Flags & AllocationCreateFlags.VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0;
                    bool preferDevice = createInfo.Usage == MemoryUsage.AutoPreferDevice;
                    bool preferHost = createInfo.Usage == MemoryUsage.AutoPreferHost;

                    // CPU random access - e.g. a buffer written to or transferred from GPU to read back on CPU.
                    if (hostAccessRandom)
                    {
                        // Prefer cached. Cannot require it, because some platforms don't have it (e.g. Raspberry Pi - see #362)!
                        preferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

                        if (!isIntegratedGPU && deviceAccess && hostAccessAllowTransferInstead && !preferHost)
                        {
                            // Nice if it will end up in HOST_VISIBLE, but more importantly prefer DEVICE_LOCAL.
                            // Omitting HOST_VISIBLE here is intentional.
                            // In case there is DEVICE_LOCAL | HOST_VISIBLE | HOST_CACHED, it will pick that one.
                            // Otherwise, this will give same weight to DEVICE_LOCAL as HOST_VISIBLE | HOST_CACHED and select the former if occurs first on the list.
                            preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                        }
                        else
                        {
                            // Always CPU memory.
                            requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                        }
                    }
                    // CPU sequential write - may be CPU or host-visible GPU memory, uncached and write-combined.
                    else if (hostAccessSequentialWrite)
                    {
                        // Want uncached and write-combined.
                        notPreferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

                        if (!isIntegratedGPU && deviceAccess && hostAccessAllowTransferInstead && !preferHost)
                        {
                            preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                        }
                        else
                        {
                            requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

                            // Direct GPU access, CPU sequential write (e.g. a dynamic uniform buffer updated every frame)
                            if (deviceAccess)
                            {
                                // Could go to CPU memory or GPU BAR/unified. Up to the user to decide. If no preference, choose GPU memory.
                                if (preferHost)
                                    notPreferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                                else
                                    preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                            }
                            // GPU no direct access, CPU sequential write (e.g. an upload buffer to be transferred to the GPU)
                            else
                            {
                                // Could go to CPU memory or GPU BAR/unified. Up to the user to decide. If no preference, choose CPU memory.
                                if (preferDevice)
                                    preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                                else
                                    notPreferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                            }
                        }
                    }
                    // No CPU access
                    else
                    {
                        // if(deviceAccess)
                        //
                        // GPU access, no CPU access (e.g. a color attachment image) - prefer GPU memory,
                        // unless there is a clear preference from the user not to do so.
                        //
                        // else:
                        //
                        // No direct GPU access, no CPU access, just transfers.
                        // It may be staging copy intended for e.g. preserving image for next frame (then better GPU memory) or
                        // a "swap file" copy to free some GPU memory (then better CPU memory).
                        // Up to the user to decide. If no preferece, assume the former and choose GPU memory.

                        if (preferHost)
                            notPreferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                        else
                            preferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                    }
                    break;
                }
            default:
                throw new InvalidOperationException();
        }

        // Avoid DEVICE_COHERENT unless explicitly requested.
        if (((uint)(createInfo.RequiredFlags | createInfo.PreferredFlags) &
            (VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD_COPY | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD_COPY)) == 0)
        {
            notPreferredFlags |= (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD_COPY;
        }

        return true;
    }

    private VkResult AllocateMemory(ref AllocationCreateInfo createInfo,
        in VkMemoryRequirements vkMemReq, bool requiresDedicatedAllocation, bool prefersDedicatedAllocation,
        VkBuffer dedicatedBuffer, VkImage dedicatedImage, uint dedicatedBufferImageUsage,
        SuballocationType suballocType,
        out Allocation? allocation)
    {
        allocation = default;
        if (vkMemReq.size == 0)
        {
            return VkResult.ErrorInitializationFailed;
        }

        VkResult result = CalcAllocationParams(ref createInfo, requiresDedicatedAllocation, prefersDedicatedAllocation);
        if (result != VkResult.Success)
            return result;

        if (createInfo.Pool != null)
        {
            //VmaBlockVector & blockVector = createInfoFinal.pool->m_BlockVector;
            //return AllocateMemoryOfType(
            //    createInfoFinal.pool,
            //    vkMemReq.size,
            //    vkMemReq.alignment,
            //    prefersDedicatedAllocation,
            //    dedicatedBuffer,
            //    dedicatedImage,
            //    dedicatedBufferImageUsage,
            //    createInfoFinal,
            //    blockVector.GetMemoryTypeIndex(),
            //    suballocType,
            //    createInfoFinal.pool->m_DedicatedAllocations,
            //    blockVector,
            //    allocationCount,
            //    pAllocations);

            allocation = default;
            return VkResult.ErrorOutOfDeviceMemory;
        }
        else
        {
            // Bit mask of memory Vulkan types acceptable for this allocation.
            uint memoryTypeBits = vkMemReq.memoryTypeBits;
            result = FindMemoryTypeIndex(memoryTypeBits, in createInfo, dedicatedBufferImageUsage, out int memoryTypeIndex);
            // Can't find any single memory type matching requirements. res is VK_ERROR_FEATURE_NOT_PRESENT.
            if (result != VkResult.Success)
                return result;

            do
            {
                BlockVector blockVector = _blockVectors[memoryTypeIndex];
                Debug.Assert(blockVector != null, "Trying to use unsupported memory type!");
                result = AllocateMemoryOfType(
                    null,
                    vkMemReq.size,
                    vkMemReq.alignment,
                    requiresDedicatedAllocation || prefersDedicatedAllocation,
                    dedicatedBuffer,
                    dedicatedImage,
                    dedicatedBufferImageUsage,
                    ref createInfo,
                    memoryTypeIndex,
                    suballocType,
                    _dedicatedAllocations[memoryTypeIndex],
                    blockVector,
                    out allocation);

                // Allocation succeeded
                if (result == VkResult.Success)
                    return VkResult.Success;

                // Remove old memTypeIndex from list of possibilities.
                memoryTypeBits &= ~(1u << memoryTypeIndex);
                // Find alternative memTypeIndex.
                result = FindMemoryTypeIndex(memoryTypeBits, in createInfo, dedicatedBufferImageUsage, out memoryTypeIndex);

            } while (result == VkResult.Success);

            // No other matching memory type index could be found.
            // Not returning res, which is VK_ERROR_FEATURE_NOT_PRESENT, because we already failed to allocate once.
            return VkResult.ErrorOutOfDeviceMemory;
        }
    }

    private VkResult AllocateMemoryOfType(object? pool, ulong size, ulong alignment, bool dedicatedPreferred,
        VkBuffer dedicatedBuffer, VkImage dedicatedImage, uint dedicatedBufferImageUsage,
        ref AllocationCreateInfo createInfo,
        int memTypeIndex,
        SuballocationType suballocType,
        DedicatedAllocationList dedicatedAllocations,
        BlockVector blockVector,
        out Allocation? allocation)
    {
        allocation = default;

        int allocationCount = 1;
        VkResult res = CalcMemTypeParams(
            ref createInfo,
            memTypeIndex,
            size,
            allocationCount);
        if (res != VkResult.Success)
        {
            return res;
        }

        if ((createInfo.Flags & AllocationCreateFlags.DedicatedMemory) != 0)
        {
            //return AllocateDedicatedMemory(
            //    pool,
            //    size,
            //    suballocType,
            //    dedicatedAllocations,
            //    memTypeIndex,
            //    (finalCreateInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0,
            //    (finalCreateInfo.flags & VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT) != 0,
            //    (finalCreateInfo.flags &
            //        (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)) != 0,
            //    (finalCreateInfo.flags & VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT) != 0,
            //    finalCreateInfo.pUserData,
            //    finalCreateInfo.priority,
            //    dedicatedBuffer,
            //    dedicatedImage,
            //    dedicatedBufferImageUsage,
            //    allocationCount,
            //    pAllocations,
            //    blockVector.GetAllocationNextPtr());
            throw new NotImplementedException();
        }
        else
        {
            bool canAllocateDedicated = (createInfo.Flags & AllocationCreateFlags.NeverAllocate) == 0
               && (pool == null || !blockVector.HasExplicitBlockSize);

            if (canAllocateDedicated)
            {
#if TODO
                // Heuristics: Allocate dedicated memory if requested size if greater than half of preferred block size.
                if (size > blockVector.PreferredBlockSize / 2)
                {
                    dedicatedPreferred = true;
                }

                // Protection against creating each allocation as dedicated when we reach or exceed heap size/budget,
                // which can quickly deplete maxMemoryAllocationCount: Don't prefer dedicated allocations when above
                // 3/4 of the maximum allocation count.
                if (_physicalDeviceProperties.limits.maxMemoryAllocationCount < uint.MaxValue / 4 &&
                    m_DeviceMemoryCount.load() > _physicalDeviceProperties.limits.maxMemoryAllocationCount * 3 / 4)
                {
                    dedicatedPreferred = false;
                }

                if (dedicatedPreferred)
                {
                    res = AllocateDedicatedMemory(
                        pool,
                        size,
                        suballocType,
                        dedicatedAllocations,
                        memTypeIndex,
                        (finalCreateInfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0,
                        (finalCreateInfo.flags & VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT) != 0,
                        (finalCreateInfo.flags &
                            (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)) != 0,
                        (finalCreateInfo.flags & VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT) != 0,
                        finalCreateInfo.pUserData,
                        finalCreateInfo.priority,
                        dedicatedBuffer,
                        dedicatedImage,
                        dedicatedBufferImageUsage,
                        allocationCount,
                        pAllocations,
                        blockVector.GetAllocationNextPtr());
                    if (res == VK_SUCCESS)
                    {
                        // Succeeded: AllocateDedicatedMemory function already filled pMemory, nothing more to do here.
                        Debug.WriteLine("    Allocated as DedicatedMemory");
                        return VK_SUCCESS;
                    }
                } 
#endif
            }

            res = blockVector.Allocate(
                size,
                alignment,
                createInfo,
                suballocType,
                out allocation);
            if (res == VK_SUCCESS)
                return VK_SUCCESS;
        }

        return VkResult.Success;
    }

    private VkResult CalcAllocationParams(ref AllocationCreateInfo createInfo, bool dedicatedRequired, bool dedicatedPreferred)
    {
        Debug.Assert((createInfo.Flags &
            (AllocationCreateFlags.HostAccessSequentialWrite | AllocationCreateFlags.HostAccessRandom)) !=
            (AllocationCreateFlags.HostAccessSequentialWrite | AllocationCreateFlags.HostAccessRandom),
            "Specifying both flags VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT and VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT is incorrect."
            );

        if (createInfo.Usage == MemoryUsage.Auto
            || createInfo.Usage == MemoryUsage.AutoPreferDevice
            || createInfo.Usage == MemoryUsage.AutoPreferHost)
        {
            if ((createInfo.Flags & AllocationCreateFlags.Mapped) != 0)
            {
                //VMA_ASSERT((inoutCreateInfo.flags & (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)) != 0 &&
                //    "When using VMA_ALLOCATION_CREATE_MAPPED_BIT and usage = VMA_MEMORY_USAGE_AUTO*, you must also specify VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT or VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT.");
            }
        }

        // If memory is lazily allocated, it should be always dedicated.
        if (dedicatedRequired ||
            createInfo.Usage == MemoryUsage.GpuLazilyAllocated)
        {
            createInfo.Flags |= AllocationCreateFlags.DedicatedMemory;
        }

        //if (createInfo.Pool != null)
        //{
        //    if (inoutCreateInfo.pool->m_BlockVector.HasExplicitBlockSize() &&
        //        (inoutCreateInfo.flags & VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) != 0)
        //    {
        //        VMA_ASSERT(0 && "Specifying VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT while current custom pool doesn't support dedicated allocations.");
        //        return VK_ERROR_FEATURE_NOT_PRESENT;
        //    }
        //    inoutCreateInfo.priority = inoutCreateInfo.pool->m_BlockVector.GetPriority();
        //}

        if ((createInfo.Flags & AllocationCreateFlags.DedicatedMemory) != 0
            && (createInfo.Flags & AllocationCreateFlags.NeverAllocate) != 0)
        {
            Log.Error("Specifying VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT together with VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT makes no sense.");
            return VkResult.ErrorFeatureNotPresent;
        }

        //if (VMA_DEBUG_ALWAYS_DEDICATED_MEMORY &&
        //(inoutCreateInfo.flags & VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT) != 0)
        //{
        //    inoutCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        //}

        // Non-auto USAGE values imply HOST_ACCESS flags.
        // And so does VMA_MEMORY_USAGE_UNKNOWN because it is used with custom pools.
        // Which specific flag is used doesn't matter. They change things only when used with VMA_MEMORY_USAGE_AUTO*.
        // Otherwise they just protect from assert on mapping.
        if (createInfo.Usage != MemoryUsage.Auto
            && createInfo.Usage != MemoryUsage.AutoPreferDevice
            && createInfo.Usage != MemoryUsage.AutoPreferHost)
        {
            if ((createInfo.Flags & (AllocationCreateFlags.HostAccessSequentialWrite | AllocationCreateFlags.HostAccessRandom)) == 0)
            {
                createInfo.Flags |= AllocationCreateFlags.HostAccessRandom;
            }
        }

        return VkResult.Success;
    }

    private VkResult CalcMemTypeParams(ref AllocationCreateInfo createInfo,
        int memTypeIndex,
        ulong size,
        int allocationCount)
    {
        // If memory type is not HOST_VISIBLE, disable MAPPED.
        if ((createInfo.Flags & AllocationCreateFlags.Mapped) != 0 &&
            (_memoryProperties.memoryTypes[memTypeIndex].propertyFlags & VkMemoryPropertyFlags.HostVisible) == 0)
        {
            createInfo.Flags &= ~AllocationCreateFlags.Mapped;
        }

        if ((createInfo.Flags & AllocationCreateFlags.DedicatedMemory) != 0 &&
            (createInfo.Flags & AllocationCreateFlags.WithinBudget) != 0)
        {
            int heapIndex = MemoryTypeIndexToHeapIndex(memTypeIndex);
            //VmaBudget heapBudget = { };
            //GetHeapBudgets(&heapBudget, heapIndex, 1);
            //if (heapBudget.usage + size * allocationCount > heapBudget.budget)
            //{
            //    return VkResult.ErrorOutOfDeviceMemory;
            //}
            throw new NotImplementedException();
        }
        return VkResult.Success;
    }

    public bool TryFindMemoryType(uint typeFilter, VkMemoryPropertyFlags properties, out uint typeIndex)
    {
        typeIndex = 0;

        for (int i = 0; i < _memoryProperties.memoryTypeCount; i++)
        {
            if (((typeFilter & (1 << i)) != 0)
                && (_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                typeIndex = (uint)i;
                return true;
            }
        }

        return false;
    }


    public VkResult AllocateMemory(
        out VkDeviceMemory deviceMemory,
        VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
        VkImage dedicatedImage, VkBuffer dedicatedBuffer,
        bool enableDeviceAddress = false, bool enableExportMemory = false)
    {
        // Find a memory space that satisfies the requirements
        if (!TryFindMemoryType(memRequirements.memoryTypeBits, properties, out uint memoryTypeIndex))
        {
            // This is incorrect; need better error reporting
            deviceMemory = VkDeviceMemory.Null;
            return VkResult.ErrorOutOfDeviceMemory;
        }

        // allocate memory
        VkMemoryAllocateFlagsInfo allocFlags = new();
        if (enableDeviceAddress)
            allocFlags.flags |= VkMemoryAllocateFlags.DeviceAddress;

        void* pNext = &allocFlags;

        // Dedicated memory
        VkMemoryDedicatedAllocateInfo dedicatedAllocation = new()
        {
            pNext = pNext,
            image = dedicatedImage,
            buffer = dedicatedBuffer
        };

        if (dedicatedImage.IsNotNull || dedicatedBuffer.IsNotNull)
        {
            // Append the VkMemoryDedicatedAllocateInfo structure to the chain
            pNext = &dedicatedAllocation;
        }

        VkExternalMemoryHandleTypeFlags handleType;
        if (OperatingSystem.IsWindows())
        {
            handleType = VkExternalMemoryHandleTypeFlags.OpaqueWin32;
        }
        else
        {
            handleType = VkExternalMemoryHandleTypeFlags.OpaqueFD;
        }

        VkExportMemoryAllocateInfo exportInfo = new()
        {
            pNext = pNext,
            handleTypes = handleType
        };

        if (enableExportMemory)
        {
            // Append the VkExportMemoryAllocateInfo structure to the chain
            pNext = &exportInfo;
        }

        VkMemoryAllocateInfo allocInfo = new();
        allocInfo.pNext = pNext;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        return vkAllocateMemory(_device, &allocInfo, null, out deviceMemory);
    }

    public VkDeviceMemory AllocateTextureMemory(VkImage image, bool shared, out ulong size)
    {
        // grab the image memory requirements
        vkGetImageMemoryRequirements(_device, image, out VkMemoryRequirements memRequirements);

        // allocate memory
        VkMemoryPropertyFlags memProperties = VkMemoryPropertyFlags.DeviceLocal;
        bool enableDeviceAddress = false;
        bool enableMemoryExport = shared;
        VkResult res = AllocateMemory(out VkDeviceMemory deviceMemory, memRequirements, memProperties, image, VkBuffer.Null, enableDeviceAddress, enableMemoryExport);
        res.CheckResult();

        vkBindImageMemory(_device.Handle, image, deviceMemory, 0);
        size = memRequirements.size;
        return deviceMemory;
    }

    public VkDeviceMemory AllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags memProperties, out ulong size, bool shared, bool enableDeviceAddress = false)
    {
        // grab the image memory requirements
        GetBufferMemoryRequirements(buffer, out VkMemoryRequirements memRequirements, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation);

        // allocate memory
        bool enableMemoryExport = shared;
        VkResult res = AllocateMemory(out VkDeviceMemory deviceMemory, memRequirements, memProperties, VkImage.Null, buffer, enableDeviceAddress, enableMemoryExport);
        res.CheckResult();

        vkBindBufferMemory(_device.Handle, buffer, deviceMemory, 0);
        size = memRequirements.size;
        return deviceMemory;
    }

    public void FreeMemory(VkDeviceMemory deviceMemory)
    {
        vkFreeMemory(_device.Handle, deviceMemory, null);
    }

    public void FreeTextureMemory(VkDeviceMemory deviceMemory)
    {
        FreeMemory(deviceMemory);
    }

    public void FreeBufferMemory(VkDeviceMemory deviceMemory)
    {
        FreeMemory(deviceMemory);
    }

    private uint CalculateGlobalMemoryTypeBits()
    {
        Debug.Assert(MemoryTypeCount > 0);

        uint memoryTypeBits = uint.MaxValue;

        if (!_useAmdDeviceCoherentMemory)
        {
            // Exclude memory types that have VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD.
            for (int index = 0; index < MemoryTypeCount; ++index)
            {
                if (((uint)_memoryProperties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD_COPY) != 0)
                {
                    memoryTypeBits &= ~(1u << index);
                }
            }
        }

        return memoryTypeBits;
    }

    private ulong CalcPreferredBlockSize(int memTypeIndex)
    {
        int heapIndex = MemoryTypeIndexToHeapIndex(memTypeIndex);
        ulong heapSize = _memoryProperties.memoryHeaps[heapIndex].size;
        bool isSmallHeap = heapSize <= SmallHeapMaxSize;
        return MathHelper.AlignUp(isSmallHeap ? (heapSize / 8) : _preferredLargeHeapBlockSize, 32);
    }

    public struct Statistics
    {
        public uint BlockCount;
        public uint AllocationCount;
        public ulong BlockBytes;
        public ulong AllocationBytes;
    }

    public struct MemoryBudget // VmaBudget
    {
        public Statistics Statistics;
        public ulong Usage;
        public ulong Budget;
    }

    enum SuballocationType
    {
        Free = 0,
        Unknown = 1,
        Buffer = 2,
        ImageUnknown = 3,
        ImageLinear = 4,
        ImageOptimal = 5
    };

    abstract class BlockMetadata
    {
        private readonly ulong _bufferImageGranularity;
        private readonly bool _isVirtual;

        public BlockMetadata(ulong size, ulong bufferImageGranularity, bool isVirtual)
        {
            Size = size;
            _bufferImageGranularity = bufferImageGranularity;
            _isVirtual = isVirtual;
        }

        public ulong Size { get; }
    }

    sealed class VmaBlockMetadata_TLSF : BlockMetadata
    {
        // According to original paper it should be preferable 4 or 5:
        // M. Masmano, I. Ripoll, A. Crespo, and J. Real "TLSF: a New Dynamic Memory Allocator for Real-Time Systems"
        // http://www.gii.upv.es/tlsf/files/ecrts04_tlsf.pdf
        private const byte SECOND_LEVEL_INDEX = 5;
        private const ushort SMALL_BUFFER_SIZE = 256;
        private const uint INITIAL_BLOCK_ALLOC_COUNT = 16;
        private const byte MEMORY_CLASS_SHIFT = 7;
        private const byte MAX_MEMORY_CLASSES = 65 - MEMORY_CLASS_SHIFT;

        public VmaBlockMetadata_TLSF(ulong size, ulong bufferImageGranularity, bool isVirtual)
            : base(size, bufferImageGranularity, isVirtual)
        {
        }
    }

    class DeviceMemoryBlock : IDisposable
    {
        private readonly VulkanAllocator _allocator;

        public DeviceMemoryBlock(VulkanAllocator allocator,
            /* VmaPool */ object? parentPool,
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

        public object? ParentPool { get; }
        public int MemoryTypeIndex { get; }
        public VkDeviceMemory Memory { get; }
        public uint Id { get; }

        public BlockMetadata MetaData { get; }

        public void Dispose() => throw new NotImplementedException();
    }

    class BlockVector : IDisposable
    {
        private readonly VulkanAllocator _allocator;
        private readonly ReaderWriterLockSlim _mutex = new(LockRecursionPolicy.NoRecursion);
        // Incrementally sorted by sumFreeSize, ascending.
        private readonly List<DeviceMemoryBlock> _blocks = new();
        private uint _nextBlockId;
        private readonly void* _pMemoryAllocateNext;

        public BlockVector(VulkanAllocator allocator,
            object? parentPool, // VmaPool
            int memoryTypeIndex,
            ulong preferredBlockSize,
            int minBlockCount,
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
            _pMemoryAllocateNext = pMemoryAllocateNext;
        }

        public object? ParentPool { get; }

        public int MemoryTypeIndex { get; }
        public ulong PreferredBlockSize { get; }

        public int MinBlockCount { get; }
        public nuint MaxBlockCount { get; }
        public ulong BufferImageGranularity { get; }

        public bool HasExplicitBlockSize { get; }
        public uint Algorithm { get; }

        public ulong MinAllocationAlignment { get; }

        public bool IncrementalSort { get; set; } = true;

        public void Dispose()
        {

        }

        public void CreateMinBlocks()
        {
            for (int i = 0; i < MinBlockCount; ++i)
            {
                VkResult res = CreateBlock(PreferredBlockSize, out _);

                if (res != VkResult.Success)
                {
                    //throw new AllocationException("Unable to allocate device memory block", res);
                }
            }
        }

        private VkResult CreateBlock(ulong blockSize, out int newBlockIndex)
        {
            VkMemoryAllocateInfo allocInfo = new();
            allocInfo.pNext = _pMemoryAllocateNext;
            allocInfo.memoryTypeIndex = (uint)MemoryTypeIndex;
            allocInfo.allocationSize = blockSize;

            // Every standalone block can potentially contain a buffer with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT - always enable the feature.
            VkMemoryAllocateFlagsInfo allocFlagsInfo = new();
            if (_allocator.UseKhrBufferDeviceAddress)
            {
                allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
                allocFlagsInfo.pNext = allocInfo.pNext;
                allocInfo.pNext = &allocFlagsInfo;
            }

#if VMA_MEMORY_PRIORITY
VkMemoryPriorityAllocateInfoEXT priorityInfo = { VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT };
if (m_hAllocator->m_UseExtMemoryPriority)
{
VMA_ASSERT(m_Priority >= 0.f && m_Priority <= 1.f);
priorityInfo.priority = m_Priority;
VmaPnextChainPushFront(&allocInfo, &priorityInfo);
}
#endif // VMA_MEMORY_PRIORITY

#if VMA_EXTERNAL_MEMORY
// Attach VkExportMemoryAllocateInfoKHR if necessary.
VkExportMemoryAllocateInfoKHR exportMemoryAllocInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR };
exportMemoryAllocInfo.handleTypes = m_hAllocator->GetExternalMemoryHandleTypeFlags(m_MemoryTypeIndex);
if (exportMemoryAllocInfo.handleTypes != 0)
{
VmaPnextChainPushFront(&allocInfo, &exportMemoryAllocInfo);
}
#endif // VMA_EXTERNAL_MEMORY

            VkResult res = _allocator.AllocateVulkanMemory(allocInfo, out VkDeviceMemory memory);
            if (res < 0)
            {
                newBlockIndex = -1;
                return res;
            }

            // New VkDeviceMemory successfully created.

            // Create new Allocation for it.
            DeviceMemoryBlock block = new(_allocator, ParentPool, MemoryTypeIndex, memory,
                allocInfo.allocationSize, _nextBlockId++, Algorithm, BufferImageGranularity);

            _blocks.Add(block);
            newBlockIndex = _blocks.Count - 1;

            return VkResult.Success;
        }

        public VkResult Allocate(ulong size, ulong alignment, in AllocationCreateInfo createInfo, SuballocationType suballocType, out Allocation? allocation)
        {
            allocation = default;

            VkResult res = VkResult.Success;

            alignment = Math.Max(alignment, MinAllocationAlignment);

            //if (IsCorruptionDetectionEnabled())
            //{
            //    size = VmaAlignUp<VkDeviceSize>(size, sizeof(VMA_CORRUPTION_DETECTION_MAGIC_VALUE));
            //    alignment = VmaAlignUp<VkDeviceSize>(alignment, sizeof(VMA_CORRUPTION_DETECTION_MAGIC_VALUE));
            //}

            _mutex.EnterWriteLock();

            try
            {
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
            finally
            {
                _mutex.ExitWriteLock();
            }

            if (res != VK_SUCCESS)
            {
                // Free all already created allocations.
                //while (allocIndex--)
                //{
                //    Free(pAllocations[allocIndex]);
                //}
            }

            return res;

            return VK_SUCCESS;
        }

        private VkResult AllocatePage(ulong size, ulong alignment, in AllocationCreateInfo createInfo, SuballocationType suballocType, out Allocation? allocation)
        {
            allocation = default;
            bool isUpperAddress = (createInfo.Flags & AllocationCreateFlags.VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT) != 0;

            ulong freeMemory;
            {
                int heapIndex = _allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex);
                Span<MemoryBudget> heapBudget = _allocator.GetHeapBudgets((int)heapIndex, 1);
                freeMemory = (heapBudget[0].Usage < heapBudget[0].Budget) ? (heapBudget[0].Budget - heapBudget[0].Usage) : 0;
            }

            bool canFallbackToDedicated = !HasExplicitBlockSize
               && (createInfo.Flags & AllocationCreateFlags.NeverAllocate) == 0;
            bool canCreateNewBlock =
                ((createInfo.Flags & AllocationCreateFlags.NeverAllocate) == 0)
                && (_blocks.Count < unchecked((uint)MaxBlockCount))
                && (freeMemory >= size || !canFallbackToDedicated);
            uint strategy = (uint)(createInfo.Flags & AllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MASK);

            // Upper address can only be used with linear allocator and within single memory block.
            if (isUpperAddress &&
                (Algorithm != VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT || MaxBlockCount > 1))
            {
                return VkResult.ErrorFeatureNotPresent;
            }

            // Early reject: requested allocation size is larger that maximum block size for this block vector.
            if (size + DebugMargin > PreferredBlockSize)
            {
                return VkResult.ErrorOutOfDeviceMemory;
            }

            // 1. Search existing allocations. Try to allocate.
            if (Algorithm == VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT)
            {
                // Use only last block.
                //if (!m_Blocks.empty())
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
                if (strategy != (uint)AllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT) // MIN_MEMORY or default
                {
                    bool isHostVisible = (_allocator.PhysicalDeviceMemoryProperties.memoryTypes[MemoryTypeIndex].propertyFlags & VkMemoryPropertyFlags.HostVisible) != 0;
                    if (isHostVisible)
                    {
                        bool isMappingAllowed = (createInfo.Flags & (AllocationCreateFlags.HostAccessSequentialWrite | AllocationCreateFlags.HostAccessRandom)) != 0;
                        // For non-mappable allocations, check blocks that are not mapped first.
                        // For mappable allocations, check blocks that are already mapped first.
                        // This way, having many blocks, we will separate mappable and non-mappable allocations,
                        // hopefully limiting the number of blocks that are mapped, which will help tools like RenderDoc.
                        for (int mappingI = 0; mappingI < 2; ++mappingI)
                        {
                            // Forward order in m_Blocks - prefer blocks with smallest amount of free space.
                            for (int blockIndex = 0; blockIndex < _blocks.Count; ++blockIndex)
                            {
                                DeviceMemoryBlock pCurrBlock = _blocks[blockIndex];
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
                            DeviceMemoryBlock pCurrBlock = _blocks[blockIndex];
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
                        DeviceMemoryBlock pCurrBlock = _blocks[blockIndex];
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
                    DeviceMemoryBlock block = _blocks[newBlockIndex];
                    Debug.Assert(block.MetaData.Size >= size);

#if TODO
            res = AllocateFromBlock(Block, size, alignment, createInfo.flags, createInfo.pUserData, suballocType, strategy, pAllocation);
            if (res == VK_SUCCESS)
            {
                //Debug.WriteLine("    Created new block #%" PRIu32 " Size=%" PRIu64, pBlock->GetId(), newBlockSize);
                IncrementallySortBlocks();
                return VK_SUCCESS;
            } 
            else
#endif
                    {
                        // Allocation from new block failed, possibly due to VMA_DEBUG_MARGIN or alignment.
                        return VkResult.ErrorOutOfDeviceMemory;
                    }
                }
            }

            return VkResult.ErrorOutOfDeviceMemory;
        }

        private ulong CalcMaxBlockSize()
        {
            ulong result = 0;

            for (int i = _blocks.Count - 1; i >= 0; --i)
            {
                var blockSize = _blocks[i].MetaData.Size;

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
                    //if (_blocks[i - 1]->m_pMetadata->GetSumFreeSize() > m_Blocks[i]->m_pMetadata->GetSumFreeSize())
                    //{
                    //    VMA_SWAP(m_Blocks[i - 1], m_Blocks[i]);
                    //    return;
                    //}
                }
            }
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

    class DedicatedAllocationList : IDisposable
    {
        public void Dispose() => throw new NotImplementedException();
    }

    class CurrentBudgetData
    {
        public readonly uint[] BlockCount = new uint[VK_MAX_MEMORY_HEAPS];
        public readonly uint[] AllocationCount = new uint[VK_MAX_MEMORY_HEAPS];

        public readonly ulong[] BlockBytes = new ulong[VK_MAX_MEMORY_HEAPS];
        public readonly ulong[] AllocationBytes = new ulong[VK_MAX_MEMORY_HEAPS];

        public uint OperationsSinceBudgetFetch;
        public readonly ReaderWriterLockSlim m_BudgetMutex = new(LockRecursionPolicy.NoRecursion);
        public readonly ulong[] VulkanUsage = new ulong[VK_MAX_MEMORY_HEAPS];
        public readonly ulong[] VulkanBudget = new ulong[VK_MAX_MEMORY_HEAPS];
        public readonly ulong[] BlockBytesAtBudgetFetch = new ulong[VK_MAX_MEMORY_HEAPS];

        public CurrentBudgetData()
        {
            for (int heapIndex = 0; heapIndex < VK_MAX_MEMORY_HEAPS; ++heapIndex)
            {
                BlockCount[heapIndex] = 0;
                AllocationCount[heapIndex] = 0;
                BlockBytes[heapIndex] = 0;
                AllocationBytes[heapIndex] = 0;
                VulkanUsage[heapIndex] = 0;
                VulkanBudget[heapIndex] = 0;
                BlockBytesAtBudgetFetch[heapIndex] = 0;
            }

            OperationsSinceBudgetFetch = 0;
        }

        public void AddAllocation(uint heapIndex, ulong allocationSize)
        {
            Interlocked.Add(ref AllocationBytes[heapIndex], allocationSize);
            Interlocked.Increment(ref AllocationCount[heapIndex]);
            Interlocked.Increment(ref OperationsSinceBudgetFetch);
        }

        public void RemoveAllocation(uint heapIndex, ulong allocationSize)
        {
            Debug.Assert(AllocationBytes[heapIndex] >= allocationSize);
            AllocationBytes[heapIndex] -= allocationSize;
            Debug.Assert(AllocationCount[heapIndex] > 0);
            Interlocked.Decrement(ref AllocationCount[heapIndex]);
            Interlocked.Increment(ref OperationsSinceBudgetFetch);
        }
    }
}

/// <summary>
/// The object containing details on a suballocation of Vulkan Memory
/// </summary>
public unsafe abstract class Allocation : IDisposable
{
    public void Dispose() => throw new NotImplementedException();
}

/// <summary>
/// Flags to be passed as <see cref="AllocationCreateInfo.Flags"/>.
/// </summary>
[Flags]
internal enum AllocationCreateFlags
{
    None = 0,
    DedicatedMemory = 0x00000001,
    NeverAllocate = 0x00000002,
    Mapped = 0x00000004,
    /** \deprecated Preserved for backward compatibility. Consider using vmaSetAllocationName() instead.

Set this flag to treat VmaAllocationCreateInfo::pUserData as pointer to a
null-terminated string. Instead of copying pointer value, a local copy of the
string is made and stored in allocation's `pName`. The string is automatically
freed together with the allocation. It is also used in vmaBuildStatsString().
*/
    VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT = 0x00000020,
    /** Allocation will be created from upper stack in a double stack pool.

    This flag is only allowed for custom pools created with #VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT flag.
    */
    VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT = 0x00000040,
    /** Create both buffer/image and allocation, but don't bind them together.
    It is useful when you want to bind yourself to do some more advanced binding, e.g. using some extensions.
    The flag is meaningful only with functions that bind by default: vmaCreateBuffer(), vmaCreateImage().
    Otherwise it is ignored.

    If you want to make sure the new buffer/image is not tied to the new memory allocation
    through `VkMemoryDedicatedAllocateInfoKHR` structure in case the allocation ends up in its own memory block,
    use also flag #VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT.
    */
    DontBind = 0x00000080,
    /** Create allocation only if additional device memory required for it, if any, won't exceed
    memory budget. Otherwise return `VK_ERROR_OUT_OF_DEVICE_MEMORY`.
    */
    WithinBudget = 0x00000100,
    /** \brief Set this flag if the allocated memory will have aliasing resources.

    Usage of this flag prevents supplying `VkMemoryDedicatedAllocateInfoKHR` when #VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT is specified.
    Otherwise created dedicated memory will not be suitable for aliasing resources, resulting in Vulkan Validation Layer errors.
    */
    CanAlias = 0x00000200,
    /**
    Requests possibility to map the allocation (using vmaMapMemory() or #VMA_ALLOCATION_CREATE_MAPPED_BIT).

    - If you use #VMA_MEMORY_USAGE_AUTO or other `VMA_MEMORY_USAGE_AUTO*` value,
      you must use this flag to be able to map the allocation. Otherwise, mapping is incorrect.
    - If you use other value of #VmaMemoryUsage, this flag is ignored and mapping is always possible in memory types that are `HOST_VISIBLE`.
      This includes allocations created in \ref custom_memory_pools.

    Declares that mapped memory will only be written sequentially, e.g. using `memcpy()` or a loop writing number-by-number,
    never read or accessed randomly, so a memory type can be selected that is uncached and write-combined.

    \warning Violating this declaration may work correctly, but will likely be very slow.
    Watch out for implicit reads introduced by doing e.g. `pMappedData[i] += x;`
    Better prepare your data in a local variable and `memcpy()` it to the mapped pointer all at once.
    */
    HostAccessSequentialWrite = 0x00000400,
    /**
    Requests possibility to map the allocation (using vmaMapMemory() or #VMA_ALLOCATION_CREATE_MAPPED_BIT).

    - If you use #VMA_MEMORY_USAGE_AUTO or other `VMA_MEMORY_USAGE_AUTO*` value,
      you must use this flag to be able to map the allocation. Otherwise, mapping is incorrect.
    - If you use other value of #VmaMemoryUsage, this flag is ignored and mapping is always possible in memory types that are `HOST_VISIBLE`.
      This includes allocations created in \ref custom_memory_pools.

    Declares that mapped memory can be read, written, and accessed in random order,
    so a `HOST_CACHED` memory type is preferred.
    */
    HostAccessRandom = 0x00000800,
    /**
    Together with #VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT or #VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
    it says that despite request for host access, a not-`HOST_VISIBLE` memory type can be selected
    if it may improve performance.

    By using this flag, you declare that you will check if the allocation ended up in a `HOST_VISIBLE` memory type
    (e.g. using vmaGetAllocationMemoryProperties()) and if not, you will create some "staging" buffer and
    issue an explicit transfer to write/read your data.
    To prepare for this possibility, don't forget to add appropriate flags like
    `VK_BUFFER_USAGE_TRANSFER_DST_BIT`, `VK_BUFFER_USAGE_TRANSFER_SRC_BIT` to the parameters of created buffer or image.
    */
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT = 0x00001000,
    /** Allocation strategy that chooses smallest possible free range for the allocation
    to minimize memory usage and fragmentation, possibly at the expense of allocation time.
    */
    VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT = 0x00010000,
    /** Allocation strategy that chooses first suitable free range for the allocation -
    not necessarily in terms of the smallest offset but the one that is easiest and fastest to find
    to minimize allocation time, possibly at the expense of allocation quality.
    */
    VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT = 0x00020000,
    /** Allocation strategy that chooses always the lowest offset in available space.
    This is not the most efficient strategy but achieves highly packed data.
    Used internally by defragmentation, not recommended in typical usage.
    */
    VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT = 0x00040000,
    /** Alias to #VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT.
    */
    VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
    /** Alias to #VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT.
    */
    VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
    /** A bit mask to extract only `STRATEGY` bits from entire set of flags.
    */
    VMA_ALLOCATION_CREATE_STRATEGY_MASK =
        VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT |
        VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT |
        VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,

    VMA_ALLOCATION_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
}

/// <summary>
/// Intended usage of the allocated memory.
/// </summary>
internal enum MemoryUsage
{
    /// <summary>
    /// No intended memory usage specified.
    /// Use other members of VmaAllocationCreateInfo to specify your requirements.
    /// </summary>
    Unknown = 0,
    /**
    \deprecated Obsolete, preserved for backward compatibility.
    Prefers `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.
    */
    GpuOnly = 1,
    /**
    \deprecated Obsolete, preserved for backward compatibility.
    Guarantees `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT` and `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`.
    */
    CpuOnly = 2,
    /**
    \deprecated Obsolete, preserved for backward compatibility.
    Guarantees `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, prefers `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.
    */
    CpuToGpu = 3,
    /**
    \deprecated Obsolete, preserved for backward compatibility.
    Guarantees `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`, prefers `VK_MEMORY_PROPERTY_HOST_CACHED_BIT`.
    */
    GpuToCpu = 4,
    /**
    \deprecated Obsolete, preserved for backward compatibility.
    Prefers not `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`.
    */
    CpuCopy = 5,
    /**
    Lazily allocated GPU memory having `VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT`.
    Exists mostly on mobile platforms. Using it on desktop PC or other GPUs with no such memory type present will fail the allocation.

    Usage: Memory for transient attachment images (color attachments, depth attachments etc.), created with `VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT`.

    Allocations with this usage are always created as dedicated - it implies #VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT.
    */
    GpuLazilyAllocated = 6,
    /**
    Selects best memory type automatically.
    This flag is recommended for most common use cases.

    When using this flag, if you want to map the allocation (using vmaMapMemory() or #VMA_ALLOCATION_CREATE_MAPPED_BIT),
    you must pass one of the flags: #VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT or #VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    in VmaAllocationCreateInfo::flags.

    It can be used only with functions that let the library know `VkBufferCreateInfo` or `VkImageCreateInfo`, e.g.
    vmaCreateBuffer(), vmaCreateImage(), vmaFindMemoryTypeIndexForBufferInfo(), vmaFindMemoryTypeIndexForImageInfo()
    and not with generic memory allocation functions.
    */
    Auto = 7,
    /**
    Selects best memory type automatically with preference for GPU (device) memory.

    When using this flag, if you want to map the allocation (using vmaMapMemory() or #VMA_ALLOCATION_CREATE_MAPPED_BIT),
    you must pass one of the flags: #VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT or #VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    in VmaAllocationCreateInfo::flags.

    It can be used only with functions that let the library know `VkBufferCreateInfo` or `VkImageCreateInfo`, e.g.
    vmaCreateBuffer(), vmaCreateImage(), vmaFindMemoryTypeIndexForBufferInfo(), vmaFindMemoryTypeIndexForImageInfo()
    and not with generic memory allocation functions.
    */
    AutoPreferDevice = 8,
    /**
    Selects best memory type automatically with preference for CPU (host) memory.

    When using this flag, if you want to map the allocation (using vmaMapMemory() or #VMA_ALLOCATION_CREATE_MAPPED_BIT),
    you must pass one of the flags: #VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT or #VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    in VmaAllocationCreateInfo::flags.

    It can be used only with functions that let the library know `VkBufferCreateInfo` or `VkImageCreateInfo`, e.g.
    vmaCreateBuffer(), vmaCreateImage(), vmaFindMemoryTypeIndexForBufferInfo(), vmaFindMemoryTypeIndexForImageInfo()
    and not with generic memory allocation functions.
    */
    AutoPreferHost = 9,
}

internal struct AllocationCreateInfo
{
    public AllocationCreateFlags Flags;
    /// <summary>
    /// Intended usage of memory.
    /// 
    /// You can leave <see cref="MemoryUsage.Unknown"/> if you specify memory requirements in other way.
    /// If `pool` is not null, this member is ignored.
    /// </summary>
    public MemoryUsage Usage;

    /** \brief Flags that must be set in a Memory Type chosen for an allocation.

    Leave 0 if you specify memory requirements in other way. \n
    If `pool` is not null, this member is ignored.*/
    public VkMemoryPropertyFlags RequiredFlags;
    /** \brief Flags that preferably should be set in a memory type chosen for an allocation.

    Set to 0 if no additional flags are preferred. \n
    If `pool` is not null, this member is ignored. */
    public VkMemoryPropertyFlags PreferredFlags;
    /** \brief Bitmask containing one bit set for every memory type acceptable for this allocation.

    Value 0 is equivalent to `UINT32_MAX` - it means any memory type is accepted if
    it meets other requirements specified by this structure, with no further
    restrictions on memory type index. \n
    If `pool` is not null, this member is ignored.
    */
    public uint MemoryTypeBits;

    public object? Pool;// VmaPool
    public object? UserData;

    public float Priority;
}
