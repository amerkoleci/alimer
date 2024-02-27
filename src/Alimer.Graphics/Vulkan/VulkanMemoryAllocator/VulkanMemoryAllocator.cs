// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice.Mathematics;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

internal unsafe class VulkanMemoryAllocator : IDisposable
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

    private volatile uint _isDisposed = 0;
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

    public VulkanMemoryAllocator(VkDevice device, VkPhysicalDevice physicalDevice, ulong preferredLargeHeapBlockSize = 0)
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

    public ref readonly VkDevice Device => ref _device;

    public uint MemoryHeapCount => _memoryProperties.memoryHeapCount;

    public uint MemoryTypeCount => _memoryProperties.memoryTypeCount;

    public ulong BufferImageGranularity => Math.Max(DebugMinBufferImageGranularity, _physicalDeviceProperties.limits.bufferImageGranularity);

    public uint GlobalMemoryTypeBits { get; }

    public bool UseKhrDedicatedAllocation { get; set; } = true;
    public bool UseKhrBindMemory2 { get; set; } = true;
    public bool UseKhrBufferDeviceAddress { get; set; }

    public ref readonly VkPhysicalDeviceProperties PhysicalDeviceProperties => ref _physicalDeviceProperties;
    public ref readonly VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties => ref _memoryProperties;

    /// <summary>
    /// Gets <c>true</c> if the object has been disposed; otherwise, <c>false</c>.
    /// </summary>
    public bool IsDisposed => _isDisposed != 0;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanMemoryAllocator" /> class.
    /// </summary>
    ~VulkanMemoryAllocator() => Dispose(disposing: false);

    /// <inheritdoc />
    public void Dispose()
    {
        if (Interlocked.Exchange(ref _isDisposed, 1) == 0)
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    /// <inheritdoc cref="Dispose()" />
    /// <param name="disposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
    private void Dispose(bool disposing)
    {
        if (disposing)
        {
            //VMA_ASSERT(m_Pools.IsEmpty());

            for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
            {
                _blockVectors[memTypeIndex].Dispose();
            }
        }
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

    public TotalStatistics CalculateStatistics()
    {
        TotalStatistics result = new();
        // Process default pools.
        for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
        {
            BlockVector blockVector = _blockVectors[memTypeIndex];
            blockVector?.AddDetailedStatistics(ref result.MemoryType[memTypeIndex]);
        }

        // Process custom pools.
        {
            //VmaMutexLockRead lock (m_PoolsMutex, m_UseMutex);
            //for (VmaPool pool = m_Pools.Front(); pool != VMA_NULL; pool = m_Pools.GetNext(pool))
            //{
            //    VmaBlockVector & blockVector = pool->m_BlockVector;
            //    const uint32_t memTypeIndex = blockVector.GetMemoryTypeIndex();
            //    blockVector.AddDetailedStatistics(pStats->memoryType[memTypeIndex]);
            //    pool->m_DedicatedAllocations.AddDetailedStatistics(pStats->memoryType[memTypeIndex]);
            //}
        }

        // Process dedicated allocations.
        for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
        {
            //_dedicatedAllocations[memTypeIndex].AddDetailedStatistics(pStats->memoryType[memTypeIndex]);
        }

        // Sum from memory types to memory heaps.
        for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
        {
            int memHeapIndex = (int)_memoryProperties.memoryTypes[memTypeIndex].heapIndex;
            VmaAddDetailedStatistics(ref result.MemoryHeap[memHeapIndex], result.MemoryType[memTypeIndex]);
        }

        // Sum from memory heaps to total.
        for (int memHeapIndex = 0; memHeapIndex < MemoryHeapCount; ++memHeapIndex)
            VmaAddDetailedStatistics(ref result.Total, result.MemoryHeap[memHeapIndex]);

        Debug.Assert(result.Total.Statistics.AllocationCount == 0 || result.Total.AllocationSizeMax >= result.Total.AllocationSizeMin);
        Debug.Assert(result.Total.UnusedRangeCount == 0 || result.Total.UnusedRangeSizeMax >= result.Total.UnusedRangeSizeMin);

        return result;
    }

    public AllocationInfo GetAllocationInfo(Allocation allocation)
    {
        return new()
        {
            MemoryType = (uint)allocation.MemoryTypeIndex,
            DeviceMemory = allocation.GetMemory(),
            Offset = allocation.Offset,
            Size = allocation.Size,
            pMappedData = allocation.GetMappedData()
        };
        //pAllocationInfo->pUserData = hAllocation->GetUserData();
        //pAllocationInfo->pName = hAllocation->GetName();
    }

    #region Buffer
    public VkResult CreateBuffer(VkBufferCreateInfo createInfo, AllocationCreateInfo allocationCreateInfo, out VkBuffer buffer, out Allocation? allocation)
    {
        buffer = VkBuffer.Null;
        allocation = default;

        if (createInfo.size == 0)
        {
            return VkResult.ErrorInitializationFailed;
        }

        if (((uint)createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_COPY) != 0 &&
            !UseKhrBufferDeviceAddress)
        {
            //Log.Error("Creating a buffer with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT is not valid if VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT was not used.");
            return VkResult.ErrorInitializationFailed;
        }

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
                    result = BindBufferMemory(allocation!, 0, buffer, null);
                }

                if (result >= 0)
                {
                    // All steps succeeded.
                    //if (pAllocationInfo != VMA_NULL)
                    //{
                    //    allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
                    //}

                    return VkResult.Success;
                }

                FreeMemory(allocation!);
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

    public void DestroyBuffer(VkBuffer buffer, Allocation allocation)
    {
        if (buffer.IsNull && allocation == null)
        {
            return;
        }

        //VMA_DEBUG_LOG("vmaDestroyBuffer");
        //VMA_DEBUG_GLOBAL_MUTEX_LOCK

        if (buffer.IsNotNull)
        {
            vkDestroyBuffer(_device, buffer, null);
        }

        if (allocation != null)
        {
            FreeMemory(allocation);
        }
    }

    public VkResult BindVulkanBuffer(VkDeviceMemory memory, ulong memoryOffset, VkBuffer buffer, void* pNext)
    {
        if (pNext != null)
        {
            if (UseKhrBindMemory2)
            {
                VkBindBufferMemoryInfo bindBufferMemoryInfo = new();
                bindBufferMemoryInfo.pNext = pNext;
                bindBufferMemoryInfo.buffer = buffer;
                bindBufferMemoryInfo.memory = memory;
                bindBufferMemoryInfo.memoryOffset = memoryOffset;
                return vkBindBufferMemory2(_device, 1, &bindBufferMemoryInfo);
            }
            else
            {
                return VkResult.ErrorExtensionNotPresent;
            }
        }
        else
        {
            return vkBindBufferMemory(_device, buffer, memory, memoryOffset);
        }
    }

    public VkResult BindBufferMemory(Allocation allocation, ulong allocationLocalOffset, VkBuffer buffer, void* pNext)
    {
        VkResult res = VkResult.ErrorUnknown;
        switch (allocation.AllocationType)
        {
            case AllocationType.Dedicated:
                res = BindVulkanBuffer(allocation.GetMemory(), allocationLocalOffset, buffer, pNext);
                break;
            case AllocationType.Block:
                {
                    VmaDeviceMemoryBlock pBlock = allocation.GetBlock();
                    Debug.Assert(pBlock != null, "Binding buffer to allocation that doesn't belong to any block.");
                    res = pBlock.BindBufferMemory(this, allocation, allocationLocalOffset, buffer, pNext);
                    break;
                }
            default:
                throw new InvalidOperationException();
        }
        return res;
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
    #endregion

    #region Image
    public VkResult CreateImage(VkImageCreateInfo createInfo, AllocationCreateInfo allocationCreateInfo, out VkImage image, out Allocation? allocation)
    {
        image = VkImage.Null;
        allocation = default;

        if (createInfo.extent.width == 0 ||
            createInfo.extent.height == 0 ||
            createInfo.extent.depth == 0 ||
            createInfo.mipLevels == 0 ||
            createInfo.arrayLayers == 0)
        {
            return VkResult.ErrorInitializationFailed;
        }

        VkResult result = vkCreateImage(_device.Handle, &createInfo, null, out image);
        if (result >= VkResult.Success)
        {
            SuballocationType suballocType = createInfo.tiling == VkImageTiling.Optimal ? SuballocationType.ImageOptimal : SuballocationType.ImageLinear;

            // 2. Allocate memory using allocator.
            GetImageMemoryRequirements(image, out VkMemoryRequirements vkMemReq, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation);

            // 3. Allocate memory using allocator.
            result = AllocateMemory(ref allocationCreateInfo, in vkMemReq, requiresDedicatedAllocation, prefersDedicatedAllocation,
                VkBuffer.Null, // dedicatedBuffer
                image, // dedicatedImage
                (uint)createInfo.usage, // dedicatedBufferImageUsage
                suballocType,
                out allocation);

            if (result >= 0)
            {
                // 3. Bind buffer with memory.
                if ((allocationCreateInfo.Flags & AllocationCreateFlags.DontBind) == 0)
                {
                    result = BindImageMemory(allocation!, 0, image, null);
                }

                if (result >= 0)
                {
                    // All steps succeeded.
                    //if (pAllocationInfo != VMA_NULL)
                    //{
                    //    allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
                    //}

                    return VkResult.Success;
                }

                FreeMemory(allocation!);

                allocation = default;
                vkDestroyImage(_device, image, null);
                image = VkImage.Null;

                return result;
            }

            vkDestroyImage(_device, image, null);
            image = VkImage.Null;
            return result;
        }

        return result;
    }

    public void DestroyImage(VkImage image, Allocation allocation)
    {
        if (image.IsNull && allocation == null)
        {
            return;
        }

        //VMA_DEBUG_LOG("vmaDestroyImage");
        //VMA_DEBUG_GLOBAL_MUTEX_LOCK

        if (image.IsNotNull)
        {
            vkDestroyImage(_device, image, null);
        }

        if (allocation != null)
        {
            FreeMemory(allocation);
        }
    }

    public VkResult BindImageMemory(Allocation allocation, ulong allocationLocalOffset, VkImage image, void* pNext)
    {
        switch (allocation.AllocationType)
        {
            case AllocationType.Dedicated:
                return BindVulkanImage(allocation.GetMemory(), allocationLocalOffset, image, pNext);
            case AllocationType.Block:
                VmaDeviceMemoryBlock pBlock = allocation.GetBlock();
                Debug.Assert(pBlock != null, "Binding image to allocation that doesn't belong to any block.");
                return pBlock.BindImageMemory(this, allocation, allocationLocalOffset, image, pNext);
            default:
                return VkResult.ErrorUnknown;
        }
    }

    public VkResult BindVulkanImage(VkDeviceMemory memory, ulong memoryOffset, VkImage image, void* pNext)
    {
        if (pNext != null)
        {
            if (UseKhrBindMemory2)
            {
                VkBindImageMemoryInfo bindImageMemoryInfo = new();
                bindImageMemoryInfo.pNext = pNext;
                bindImageMemoryInfo.image = image;
                bindImageMemoryInfo.memory = memory;
                bindImageMemoryInfo.memoryOffset = memoryOffset;
                return vkBindImageMemory2(_device, 1, &bindImageMemoryInfo);
            }
            else
            {
                return VkResult.ErrorExtensionNotPresent;
            }
        }
        else
        {
            return vkBindImageMemory(_device, image, memory, memoryOffset);
        }
    }

    public void GetImageMemoryRequirements(VkImage image, out VkMemoryRequirements memReq, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation)
    {
        // UseKhrDedicatedAllocation is always true
        if (UseKhrDedicatedAllocation)
        {
            VkImageMemoryRequirementsInfo2 memReqInfo = new();
            memReqInfo.image = image;

            VkMemoryDedicatedRequirements memDedicatedReq = new();

            VkMemoryRequirements2 memReq2 = new VkMemoryRequirements2();
            //VmaPnextChainPushFront(&memReq2, &memDedicatedReq);
            memDedicatedReq.pNext = memReq2.pNext;
            memReq2.pNext = &memDedicatedReq;

            vkGetImageMemoryRequirements2(_device, &memReqInfo, &memReq2);

            memReq = memReq2.memoryRequirements;
            requiresDedicatedAllocation = memDedicatedReq.requiresDedicatedAllocation != VkBool32.False;
            prefersDedicatedAllocation = memDedicatedReq.prefersDedicatedAllocation != VkBool32.False;
        }
        else
        {
            vkGetImageMemoryRequirements(_device, image, out memReq);
            requiresDedicatedAllocation = false;
            prefersDedicatedAllocation = false;
        }
    }
    #endregion

    public VkResult Map(Allocation allocation, void** ppData)
    {
        switch (allocation.AllocationType)
        {
            case AllocationType.Block:
                {
                    VmaDeviceMemoryBlock block = allocation.GetBlock();
                    byte* pBytes = null;
                    VkResult res = block.Map(this, 1, (void**)&pBytes);
                    if (res == VK_SUCCESS)
                    {
                        *ppData = pBytes + allocation.Offset;
                        allocation.BlockAllocMap();
                    }
                    return res;
                }
            //VMA_FALLTHROUGH; // Fallthrough
            case AllocationType.Dedicated:
                return allocation.DedicatedAllocMap(this, ppData);
            default:
                Debug.Assert(false);
                return VkResult.ErrorMemoryMapFailed;
        }
    }

    public void Unmap(Allocation allocation)
    {
        switch (allocation.AllocationType)
        {
            case AllocationType.Block:
                {
                    VmaDeviceMemoryBlock block = allocation.GetBlock();
                    allocation.BlockAllocUnmap();
                    block.Unmap(this, 1);
                }
                break;
            case AllocationType.Dedicated:
                allocation.DedicatedAllocUnmap(this);
                break;
            default:
                Debug.Assert(false);
                break;
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
        ulong prevDeviceMemoryCount = Interlocked.Decrement(ref _deviceMemoryCount);
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
        _ = Interlocked.Increment(ref _budget.BlockCount[heapIndex]);

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

    // Call to Vulkan function vkFreeMemory with accompanying bookkeeping.
    public void FreeVulkanMemory(int memoryType, ulong size, VkDeviceMemory memory)
    {
        // Informative callback.
        //if (m_DeviceMemoryCallbacks.pfnFree != VMA_NULL)
        //{
        //    (*m_DeviceMemoryCallbacks.pfnFree)(this, memoryType, hMemory, size, m_DeviceMemoryCallbacks.pUserData);
        //}

        // VULKAN CALL vkFreeMemory.
        vkFreeMemory(_device, memory, null);

        int heapIndex = MemoryTypeIndexToHeapIndex(memoryType);
        _ = Interlocked.Increment(ref _budget.BlockCount[heapIndex]);
        //m_Budget.m_BlockBytes[heapIndex] -= size;

        _ = Interlocked.Decrement(ref _deviceMemoryCount);
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
                        //Log.Error("VMA_MEMORY_USAGE_AUTO* values can only be used with functions like vmaCreateBuffer, vmaCreateImage so that the details of the created resource are known.");
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
                // Heuristics: Allocate dedicated memory if requested size if greater than half of preferred block size.
                if (size > blockVector.PreferredBlockSize / 2)
                {
                    dedicatedPreferred = true;
                }

#if TODO
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
            //Log.Error("Specifying VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT together with VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT makes no sense.");
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

    private void FreeMemory(Allocation allocation)
    {
        //if (VMA_DEBUG_INITIALIZE_ALLOCATIONS)
        //{
        //    FillAllocation(allocation, VMA_ALLOCATION_FILL_PATTERN_DESTROYED);
        //}

        switch (allocation.AllocationType)
        {
            case AllocationType.Block:
                {
                    BlockVector? pBlockVector = null;
                    //VmaPool hPool = allocation->GetParentPool();
                    //if (hPool != VK_NULL_HANDLE)
                    //{
                    //    pBlockVector = &hPool->m_BlockVector;
                    //}
                    //else
                    //{
                    int memTypeIndex = allocation.MemoryTypeIndex;
                    pBlockVector = _blockVectors[memTypeIndex];
                    Debug.Assert(pBlockVector != null, "Trying to free memory of unsupported type!");
                    //}
                    pBlockVector.Free(allocation);
                }
                break;
            case AllocationType.Dedicated:
                //FreeDedicatedMemory(allocation);
                break;
            default:
                Debug.Assert(false);
                break;
        }
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

    internal enum SuballocationType
    {
        Free = 0,
        Unknown = 1,
        Buffer = 2,
        ImageUnknown = 3,
        ImageLinear = 4,
        ImageOptimal = 5
    }

    internal unsafe partial struct Pointer<T>(T* value)
        where T : unmanaged
    {
        public T* Value = value;
    }

    internal unsafe partial struct VmaVector<T> : IDisposable
        where T : unmanaged
    {
        private T* m_pArray;

        private nuint m_Count;

        private nuint m_Capacity;

        public VmaVector()
        {
        }

        public VmaVector(nuint count)
        {
            m_pArray = (count != 0) ? AllocateArray<T>(count) : null;
            m_Count = count;
            m_Capacity = count;
        }

        public VmaVector(in VmaVector<T> src)
        {
            m_pArray = (src.m_Count != 0) ? AllocateArray<T>(src.m_Count) : null;
            m_Count = src.m_Count;
            m_Capacity = src.m_Count;

            if (m_Count > 0)
            {
                Unsafe.CopyBlock(m_pArray, src.m_pArray, (uint)(m_Count * __sizeof<T>()));
            }
        }

        public void Dispose()
        {
            Free(m_pArray);
        }

        public readonly bool empty()
        {
            return m_Count == 0;
        }

        public readonly nuint size()
        {
            return m_Count;
        }

        public T* data()
        {
            return m_pArray;
        }

        public void clear(bool freeMemory = false)
        {
            resize(0, freeMemory);
        }

        public T* begin()
        {
            return m_pArray;
        }

        public T* end()
        {
            return m_pArray + m_Count;
        }

        public T* rend()
        {
            return begin() - 1;
        }

        public T* rbegin()
        {
            return end() - 1;
        }

        public readonly T* cbegin()
        {
            return m_pArray;
        }

        public readonly T* cend()
        {
            return m_pArray + m_Count;
        }

        public readonly T* crbegin()
        {
            return cend() - 1;
        }

        public readonly T* crend()
        {
            return cbegin() - 1;
        }

        public void push_front(in T src)
        {
            insert(0, src);
        }

        public void push_back(in T src)
        {
            nuint newIndex = size();
            resize(newIndex + 1);
            m_pArray[newIndex] = src;
        }

        public void pop_front()
        {
            Debug.Assert(m_Count > 0);
            remove(0);
        }

        public void pop_back()
        {
            Debug.Assert(m_Count > 0);
            resize(size() - 1);
        }

        public ref T front()
        {
            Debug.Assert(m_Count > 0);
            return ref m_pArray[0];
        }

        public ref T back()
        {
            Debug.Assert(m_Count > 0);
            return ref m_pArray[m_Count - 1];
        }

        public void reserve(nuint newCapacity, bool freeMemory = false)
        {
            newCapacity = Math.Max(newCapacity, m_Count);

            if ((newCapacity < m_Capacity) && !freeMemory)
            {
                newCapacity = m_Capacity;
            }

            if (newCapacity != m_Capacity)
            {
                T* newArray = (newCapacity != 0) ? AllocateArray<T>(newCapacity) : null;

                if (m_Count != 0)
                {
                    Unsafe.CopyBlock(newArray, m_pArray, (uint)(m_Count * __sizeof<T>()));
                }
                Free(m_pArray);

                m_Capacity = newCapacity;
                m_pArray = newArray;
            }
        }

        public void resize(nuint newCount, bool freeMemory = false)
        {
            nuint newCapacity = m_Capacity;

            if (newCount > m_Capacity)
            {
                newCapacity = Math.Max(newCount, Math.Max(m_Capacity * 3 / 2, 8));
            }
            else if (freeMemory)
            {
                newCapacity = newCount;
            }

            if (newCapacity != m_Capacity)
            {
                T* newArray = (newCapacity != 0) ? AllocateArray<T>(newCapacity) : null;
                nuint elementsToCopy = Math.Min(m_Count, newCount);

                if (elementsToCopy != 0)
                {
                    Unsafe.CopyBlock(newArray, m_pArray, (uint)(elementsToCopy * __sizeof<T>()));
                }
                Free(m_pArray);

                m_Capacity = newCapacity;
                m_pArray = newArray;
            }

            m_Count = newCount;
        }

        public void insert(nuint index, in T src)
        {
            Debug.Assert(index <= m_Count);

            nuint oldCount = size();
            resize(oldCount + 1);

            if (index < oldCount)
            {
                _ = memmove(m_pArray + (index + 1), m_pArray + index, (oldCount - index) * __sizeof<T>());
            }
            m_pArray[index] = src;
        }

        public void remove(nuint index)
        {
            Debug.Assert(index < m_Count);
            nuint oldCount = size();

            if (index < oldCount - 1)
            {
                _ = memmove(m_pArray + index, m_pArray + (index + 1), (oldCount - index - 1) * __sizeof<T>());
            }
            resize(oldCount - 1);
        }

        public ref T this[nuint index]
        {
            get
            {
                Debug.Assert(index < m_Count);
                return ref m_pArray[index];
            }
        }
    }

    internal unsafe partial struct VmaPoolAllocator<T> : IDisposable
        where T : unmanaged, IDisposable
    {
        public uint FirstBlockCapacity { get; }
        private VmaVector<ItemBlock> _itemBlocks;

        public VmaPoolAllocator(uint firstBlockCapacity)
        {
            Debug.Assert(firstBlockCapacity > 1);
            FirstBlockCapacity = firstBlockCapacity;

            _itemBlocks = new VmaVector<ItemBlock>();
        }

        public void Dispose()
        {
            Clear();
            _itemBlocks.Dispose();
        }

        public void Clear()
        {
            for (nuint i = _itemBlocks.size(); i-- != 0;)
            {
                VmaUtils.Free(_itemBlocks[i].pItems);
            }
            _itemBlocks.clear(true);
        }

        public T* Alloc()
        {
            for (nuint i = _itemBlocks.size(); i-- != 0;)
            {
                ref ItemBlock block = ref _itemBlocks[i];

                // This block has some free items: Use first one.
                if (block.FirstFreeIndex != uint.MaxValue)
                {
                    Item* pItem = &block.pItems[block.FirstFreeIndex];
                    block.FirstFreeIndex = pItem->NextFreeIndex;

                    T* result = (T*)&pItem->Value;
                    return result;
                }
            }

            {
                // No block has free item: Create new one and use it.
                ref ItemBlock newBlock = ref CreateNewBlock();

                Item* pItem = &newBlock.pItems[0];
                newBlock.FirstFreeIndex = pItem->NextFreeIndex;

                T* result = &pItem->Value;
                return result;
            }
        }

        public void Free(T* ptr)
        {
            // Search all memory blocks to find ptr.
            for (nuint i = _itemBlocks.size(); i-- != 0;)
            {
                ref ItemBlock block = ref _itemBlocks[i];

                Item* pItemPtr;
                _ = memcpy(&pItemPtr, &ptr, __sizeof<nuint>());

                // Check if pItemPtr is in address range of this block.
                if ((pItemPtr >= block.pItems) && (pItemPtr < block.pItems + block.Capacity))
                {
                    ptr->Dispose(); // Explicit destructor call.
                    uint index = (uint)(pItemPtr - block.pItems);

                    pItemPtr->NextFreeIndex = block.FirstFreeIndex;
                    block.FirstFreeIndex = index;

                    return;
                }
            }

            Debug.Assert(false, "Pointer doesn't belong to this memory pool.");
        }

        private ref ItemBlock CreateNewBlock()
        {
            uint newBlockCapacity = _itemBlocks.empty() ? FirstBlockCapacity : (_itemBlocks.back().Capacity * 3 / 2);

            ItemBlock newBlock = new()
            {
                pItems = AllocateArray<Item>(newBlockCapacity),
                Capacity = newBlockCapacity,
                FirstFreeIndex = 0u,
            };
            _itemBlocks.push_back(newBlock);

            // Setup singly-linked list of all free items in this block.
            for (uint i = 0; i < newBlockCapacity - 1; ++i)
            {
                newBlock.pItems[i].NextFreeIndex = i + 1;
            }

            newBlock.pItems[newBlockCapacity - 1].NextFreeIndex = uint.MaxValue;
            return ref _itemBlocks.back();
        }
        internal partial struct Item
        {
            [UnscopedRef]
            public ref uint NextFreeIndex
            {
                [MethodImpl(MethodImplOptions.AggressiveInlining)]
                get
                {
                    // uint.MaxValue means end of list.
                    return ref Unsafe.As<T, uint>(ref Value);
                }
            }

            public T Value;
        }

        internal partial struct ItemBlock
        {
            public Item* pItems;

            public uint Capacity;

            public uint FirstFreeIndex;
        }
    }

    internal enum VmaAllocationRequestType
    {
        Normal,
        TLSF,
        // Used by "Linear" algorithm.
        UpperAddress,
        EndOf1st,
        EndOf2nd,
    }

    internal struct AllocationRequest
    {
        public ulong allocHandle;
        public ulong size;
        //VmaSuballocationList::iterator item;
        //void* customData;
        public ulong algorithmData;
        public VmaAllocationRequestType type;
    }

    internal interface IBlockMetadata : IDisposable
    {
        public bool IsVirtual { get; }
        public ulong Size { get; }
        public bool IsEmpty { get; }

        public bool Validate();

        public ulong GetSumFreeSize();

        public ulong GetAllocationOffset(ulong allocHandle);

        // Makes actual allocation based on request. Request must already be checked and valid.
        public void Alloc(in AllocationRequest request, SuballocationType type, object? userData);

        // Frees suballocation assigned to given memory region.
        public void Free(ulong allocHandle);

        public bool CreateAllocationRequest(ulong size, ulong alignment, bool upperAddress,
            SuballocationType allocType,
            // Always one of VMA_ALLOCATION_CREATE_STRATEGY_* or VMA_ALLOCATION_INTERNAL_STRATEGY_* flags.
            uint strategy,
            AllocationRequest* allocationRequest);

        void AddDetailedStatistics(ref DetailedStatistics stats);
    }

    sealed class VmaBlockMetadata_TLSF : IBlockMetadata
    {
        // According to original paper it should be preferable 4 or 5:
        // M. Masmano, I. Ripoll, A. Crespo, and J. Real "TLSF: a New Dynamic Memory Allocator for Real-Time Systems"
        // http://www.gii.upv.es/tlsf/files/ecrts04_tlsf.pdf
        private const byte SECOND_LEVEL_INDEX = 5;
        private const ushort SMALL_BUFFER_SIZE = 256;
        private const uint INITIAL_BLOCK_ALLOC_COUNT = 16;
        private const byte MEMORY_CLASS_SHIFT = 7;
        private const byte MAX_MEMORY_CLASSES = 65 - MEMORY_CLASS_SHIFT;

        private nuint _allocCount;
        // Total number of free blocks besides null block
        private nuint _blocksFreeCount;
        // Total size of free blocks excluding null block
        private ulong _blocksFreeSize;
        private uint _isFreeBitmap;
        private byte _memoryClasses;

        [InlineArray(MAX_MEMORY_CLASSES)]
        struct _m_InnerIsFreeBitmap_e__FixedBuffer
        {
            public uint e0;
        }

        private _m_InnerIsFreeBitmap_e__FixedBuffer _innerIsFreeBitmap;
        private uint _listsCount;

        ///
        /// 0: 0-3 lists for small buffers
        /// 1+: 0-(2^SLI-1) lists for normal buffers
        private Pointer<Block>* _freeList = null;
        private VmaPoolAllocator<Block> _blockAllocator;
        //VmaPoolAllocator<Block> m_BlockAllocator;
        private Block* _nullBlock;
        //VmaBlockBufferImageGranularity m_GranularityHandler;

        public VmaBlockMetadata_TLSF(ulong size, ulong bufferImageGranularity, bool isVirtual)
        {
            Size = size;
            BufferImageGranularity = bufferImageGranularity;
            IsVirtual = isVirtual;

            _allocCount = 0;
            _blocksFreeCount = 0;
            _blocksFreeSize = 0;
            _isFreeBitmap = 0;
            _memoryClasses = 0;

            MemoryMarshal.CreateSpan(ref _innerIsFreeBitmap[0], MAX_MEMORY_CLASSES).Clear();
            _listsCount = 0;

            _freeList = null;
            _blockAllocator = new VmaPoolAllocator<Block>(INITIAL_BLOCK_ALLOC_COUNT);
            _nullBlock = null;
            // VmaBlockMetadata_TLSF::Init(VkDeviceSize size)

            //if (!IsVirtual)
            //    m_GranularityHandler.Init(GetAllocationCallbacks(), size);

            _nullBlock = _blockAllocator.Alloc();
            _nullBlock->_ctor();
            _nullBlock->size = size;
            _nullBlock->offset = 0;
            _nullBlock->prevPhysical = null;
            _nullBlock->nextPhysical = null;
            _nullBlock->MarkFree();
            _nullBlock->NextFree() = null;
            _nullBlock->PrevFree() = null;

            byte memoryClass = SizeToMemoryClass(size);
            ushort sli = SizeToSecondIndex(size, memoryClass);

            _listsCount = ((memoryClass == 0u) ? 0u : ((memoryClass - 1u) * (1u << SECOND_LEVEL_INDEX) + sli)) + 1u;

            if (isVirtual)
            {
                _listsCount += 1u << SECOND_LEVEL_INDEX;
            }
            else
            {
                _listsCount += 4;
            }

            _memoryClasses = (byte)(memoryClass + 2);
            fixed (void* innerIsFreeBitmapPtr = &_innerIsFreeBitmap[0])
            {
                _ = memset(innerIsFreeBitmapPtr, 0, MAX_MEMORY_CLASSES * sizeof(uint));
            }

            _freeList = AllocateArray<Pointer<Block>>(_listsCount);
            _ = memset(_freeList, 0, _listsCount * __sizeof<Pointer<Block>>());
        }

        public ulong Size { get; }
        public ulong BufferImageGranularity { get; }
        public bool IsVirtual { get; }
        public bool IsEmpty => _nullBlock->offset == 0;

        public void Dispose()
        {
            DeleteArray(_freeList);
            //Helpers.DeleteArray(_freeList, _listsCount);
            //m_GranularityHandler.Destroy(GetAllocationCallbacks());
        }

        public ulong GetAllocationOffset(ulong allocHandle)
        {
            return ((Block*)allocHandle)->offset;
        }

        private static byte SizeToMemoryClass(ulong size)
        {
            if (size > SMALL_BUFFER_SIZE)
                return (byte)(VmaBitScanMSB(size) - MEMORY_CLASS_SHIFT);
            return 0;
        }

        private ushort SizeToSecondIndex(ulong size, byte memoryClass)
        {
            if (memoryClass == 0)
            {
                if (IsVirtual)
                    return (ushort)((size - 1) / 8);
                else
                    return (ushort)((size - 1) / 64);
            }
            return (ushort)((size >> (memoryClass + MEMORY_CLASS_SHIFT - SECOND_LEVEL_INDEX)) ^ (1U << SECOND_LEVEL_INDEX));
        }

        private uint GetListIndex(byte memoryClass, ushort secondIndex)
        {
            if (memoryClass == 0)
                return secondIndex;

            uint index = (uint)(memoryClass - 1) * (1 << SECOND_LEVEL_INDEX) + secondIndex;
            if (IsVirtual)
                return index + (1 << SECOND_LEVEL_INDEX);
            else
                return index + 4;
        }

        private uint GetListIndex(ulong size)
        {
            byte memoryClass = SizeToMemoryClass(size);
            return GetListIndex(memoryClass, SizeToSecondIndex(size, memoryClass));
        }

        public bool Validate()
        {
            Debug.Assert(GetSumFreeSize() <= Size);

            ulong calculatedSize = _nullBlock->size;
            ulong calculatedFreeSize = _nullBlock->size;
            nuint allocCount = 0;
            nuint freeCount = 0;

            // Check integrity of free lists
            for (uint list = 0; list < _listsCount; ++list)
            {
                Block* block = _freeList[list].Value;
                if (block != null)
                {
                    Debug.Assert(block->IsFree());
                    Debug.Assert(block->PrevFree() == null);
                    while (block->NextFree() != null)
                    {
                        Debug.Assert(block->NextFree()->IsFree());
                        Debug.Assert(block->NextFree()->PrevFree() == block);
                        block = block->NextFree();
                    }
                }
            }

            ulong nextOffset = _nullBlock->offset;
            //auto validateCtx = m_GranularityHandler.StartValidation(GetAllocationCallbacks(), IsVirtual());

            Debug.Assert(_nullBlock->nextPhysical == null);
            if (_nullBlock->prevPhysical != null)
            {
                Debug.Assert(_nullBlock->prevPhysical->nextPhysical == _nullBlock);
            }
            // Check all blocks
            for (Block* prev = _nullBlock->prevPhysical; prev != null; prev = prev->prevPhysical)
            {
                Debug.Assert(prev->offset + prev->size == nextOffset);
                nextOffset = prev->offset;
                calculatedSize += prev->size;

                uint listIndex = GetListIndex(prev->size);
                if (prev->IsFree())
                {
                    ++freeCount;
                    // Check if free block belongs to free list
                    Block* freeBlock = _freeList[listIndex].Value;
                    Debug.Assert(freeBlock != null);

                    bool found = false;
                    do
                    {
                        if (freeBlock == prev)
                            found = true;

                        freeBlock = freeBlock->NextFree();
                    } while (!found && freeBlock != null);

                    Debug.Assert(found);
                    calculatedFreeSize += prev->size;
                }
                else
                {
                    ++allocCount;
                    // Check if taken block is not on a free list
                    Block* freeBlock = _freeList[listIndex].Value;
                    while (freeBlock != null)
                    {
                        Debug.Assert(freeBlock != prev);
                        freeBlock = freeBlock->NextFree();
                    }

                    if (!IsVirtual)
                    {
                        //Debug.Assert(m_GranularityHandler.Validate(validateCtx, prev->offset, prev->size));
                    }
                }

                if (prev->prevPhysical != null)
                {
                    Debug.Assert(prev->prevPhysical->nextPhysical == prev);
                }
            }

            if (!IsVirtual)
            {
                //Debug.Assert(m_GranularityHandler.FinishValidation(validateCtx));
            }

            Debug.Assert(nextOffset == 0);
            Debug.Assert(calculatedSize == Size);
            Debug.Assert(calculatedFreeSize == GetSumFreeSize());
            Debug.Assert(allocCount == _allocCount);
            Debug.Assert(freeCount == _blocksFreeCount);

            return true;
        }

        public void Alloc(in AllocationRequest request, SuballocationType type, object? userData)
        {
            Debug.Assert(request.type == VmaAllocationRequestType.TLSF);

            // Get block and pop it from the free list
            Block* currentBlock = (Block*)request.allocHandle;
            ulong offset = request.algorithmData;
            Debug.Assert(currentBlock != null);
            Debug.Assert(currentBlock->offset <= offset);

            if (currentBlock != _nullBlock)
            {
                RemoveFreeBlock(currentBlock);
            }

            ulong debugMargin = GetDebugMargin();
            ulong misssingAlignment = offset - currentBlock->offset;

            // Append missing alignment to prev block or create new one
            if (misssingAlignment != 0)
            {
                Block* prevBlock = currentBlock->prevPhysical;
                Debug.Assert(prevBlock != null, "There should be no missing alignment at offset 0!");

                if (prevBlock->IsFree() && prevBlock->size != debugMargin)
                {
                    uint oldList = GetListIndex(prevBlock->size);
                    prevBlock->size += misssingAlignment;
                    // Check if new size crosses list bucket
                    if (oldList != GetListIndex(prevBlock->size))
                    {
                        prevBlock->size -= misssingAlignment;
                        RemoveFreeBlock(prevBlock);
                        prevBlock->size += misssingAlignment;
                        InsertFreeBlock(prevBlock);
                    }
                    else
                    {
                        _blocksFreeSize += misssingAlignment;
                    }
                }
                else
                {
                    Block* newBlock = _blockAllocator.Alloc();
                    newBlock->_ctor();
                    currentBlock->prevPhysical = newBlock;
                    prevBlock->nextPhysical = newBlock;
                    newBlock->prevPhysical = prevBlock;
                    newBlock->nextPhysical = currentBlock;
                    newBlock->size = misssingAlignment;
                    newBlock->offset = currentBlock->offset;
                    newBlock->MarkTaken();

                    InsertFreeBlock(newBlock);
                }

                currentBlock->size -= misssingAlignment;
                currentBlock->offset += misssingAlignment;
            }

            ulong size = request.size + debugMargin;
            if (currentBlock->size == size)
            {
                if (currentBlock == _nullBlock)
                {
                    // Setup new null block
                    _nullBlock = _blockAllocator.Alloc();
                    _nullBlock->_ctor();
                    _nullBlock->size = 0;
                    _nullBlock->offset = currentBlock->offset + size;
                    _nullBlock->prevPhysical = currentBlock;
                    _nullBlock->nextPhysical = null;
                    _nullBlock->MarkFree();
                    _nullBlock->PrevFree() = null;
                    _nullBlock->NextFree() = null;
                    currentBlock->nextPhysical = _nullBlock;
                    currentBlock->MarkTaken();
                }
            }
            else
            {
                Debug.Assert(currentBlock->size > size, "Proper block already found, shouldn't find smaller one!");

                // Create new free block
                Block* newBlock = _blockAllocator.Alloc();
                newBlock->_ctor();
                newBlock->size = currentBlock->size - size;
                newBlock->offset = currentBlock->offset + size;
                newBlock->prevPhysical = currentBlock;
                newBlock->nextPhysical = currentBlock->nextPhysical;
                currentBlock->nextPhysical = newBlock;
                currentBlock->size = size;

                if (currentBlock == _nullBlock)
                {
                    _nullBlock = newBlock;
                    _nullBlock->MarkFree();
                    _nullBlock->NextFree() = null;
                    _nullBlock->PrevFree() = null;
                    currentBlock->MarkTaken();
                }
                else
                {
                    newBlock->nextPhysical->prevPhysical = newBlock;
                    newBlock->MarkTaken();
                    InsertFreeBlock(newBlock);
                }
            }
            //currentBlock->UserData() = userData;

            if (debugMargin > 0)
            {
                currentBlock->size -= debugMargin;
                Block* newBlock = _blockAllocator.Alloc();
                newBlock->_ctor();
                newBlock->size = debugMargin;
                newBlock->offset = currentBlock->offset + currentBlock->size;
                newBlock->prevPhysical = currentBlock;
                newBlock->nextPhysical = currentBlock->nextPhysical;
                newBlock->MarkTaken();
                currentBlock->nextPhysical->prevPhysical = newBlock;
                currentBlock->nextPhysical = newBlock;
                InsertFreeBlock(newBlock);
            }

            if (!IsVirtual)
            {
                //m_GranularityHandler.AllocPages((uint8_t)(uintptr_t)request.customData, currentBlock->offset, currentBlock->size);

            }
            ++_allocCount;
        }

        public void Free(ulong allocHandle)
        {
            Block* block = (Block*)allocHandle;
            Block* next = block->nextPhysical;
            Debug.Assert(!block->IsFree(), "Block is already free!");

            if (!IsVirtual)
            {
                //m_GranularityHandler.FreePages(block->offset, block->size);
            }

            --_allocCount;

            ulong debugMargin = GetDebugMargin();
            if (debugMargin > 0)
            {
                RemoveFreeBlock(next);
                MergeBlock(next, block);
                block = next;
                next = next->nextPhysical;
            }

            // Try merging
            Block* prev = block->prevPhysical;
            if (prev != null && prev->IsFree() && prev->size != debugMargin)
            {
                RemoveFreeBlock(prev);
                MergeBlock(block, prev);
            }

            if (!next->IsFree())
                InsertFreeBlock(block);
            else if (next == _nullBlock)
                MergeBlock(_nullBlock, block);
            else
            {
                RemoveFreeBlock(next);
                MergeBlock(next, block);
                InsertFreeBlock(next);
            }
        }

        private void RemoveFreeBlock(Block* block)
        {
            Debug.Assert(block != _nullBlock);
            Debug.Assert(block->IsFree());

            if (block->NextFree() != null)
                block->NextFree()->PrevFree() = block->PrevFree();
            if (block->PrevFree() != null)
                block->PrevFree()->NextFree() = block->NextFree();
            else
            {
                byte memClass = SizeToMemoryClass(block->size);
                ushort secondIndex = SizeToSecondIndex(block->size, memClass);
                uint index = GetListIndex(memClass, secondIndex);
                Debug.Assert(_freeList[index].Value == block);
                _freeList[index].Value = block->NextFree();
                if (block->NextFree() == null)
                {
                    _innerIsFreeBitmap[memClass] &= ~(1U << secondIndex);
                    if (_innerIsFreeBitmap[memClass] == 0)
                        _isFreeBitmap &= ~(1u << memClass);
                }
            }
            block->MarkTaken();
            //block->UserData() = null;
            --_blocksFreeCount;
            _blocksFreeSize -= block->size;
        }

        private void InsertFreeBlock(Block* block)
        {
            Debug.Assert(block != _nullBlock);
            Debug.Assert(!block->IsFree(), "Cannot insert block twice!");

            byte memClass = SizeToMemoryClass(block->size);
            ushort secondIndex = SizeToSecondIndex(block->size, memClass);
            uint index = GetListIndex(memClass, secondIndex);
            Debug.Assert(index < _listsCount);
            block->PrevFree() = null;
            block->NextFree() = _freeList[index].Value;
            _freeList[index].Value = block;
            if (block->NextFree() != null)
            {
                block->NextFree()->PrevFree() = block;
            }
            else
            {
                _innerIsFreeBitmap[memClass] |= 1U << secondIndex;
                _isFreeBitmap |= 1u << memClass;
            }
            ++_blocksFreeCount;
            _blocksFreeSize += block->size;
        }

        private void MergeBlock(Block* block, Block* prev)
        {
            Debug.Assert(block->prevPhysical == prev, "Cannot merge separate physical regions!");
            Debug.Assert(!prev->IsFree(), "Cannot merge block that belongs to free list!");

            block->offset = prev->offset;
            block->size += prev->size;
            block->prevPhysical = prev->prevPhysical;
            if (block->prevPhysical != null)
                block->prevPhysical->nextPhysical = block;
            _blockAllocator.Free(prev);
        }

        public bool CreateAllocationRequest(ulong size, ulong alignment, bool upperAddress, SuballocationType allocType, uint strategy, AllocationRequest* allocationRequest)
        {
            Debug.Assert(size > 0);
            Debug.Assert(!upperAddress, "VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT can be used only with linear algorithm.");

            // For small granularity round up
            if (!IsVirtual)
            {
                //m_GranularityHandler.RoundupAllocRequest(allocType, allocSize, allocAlignment);
            }

            size += GetDebugMargin();
            // Quick check for too small pool
            if (size > GetSumFreeSize())
            {
                allocationRequest = default;
                return false;
            }

            // If no free blocks in pool then check only null block
            if (_blocksFreeCount == nuint.Zero)
            {
                return CheckBlock(ref *_nullBlock, _listsCount, size, alignment, allocType, allocationRequest);
            }

            // Round up to the next block
            ulong sizeForNextList = size;
            ulong smallSizeStep = (ulong)(SMALL_BUFFER_SIZE / (IsVirtual ? 1 << SECOND_LEVEL_INDEX : 4));
            if (size > SMALL_BUFFER_SIZE)
            {
                sizeForNextList += (ulong)(1 << (VmaBitScanMSB(size) - SECOND_LEVEL_INDEX));
            }
            else if (size > SMALL_BUFFER_SIZE - smallSizeStep)
            {
                sizeForNextList = SMALL_BUFFER_SIZE + 1;
            }
            else
            {
                sizeForNextList += smallSizeStep;
            }

            uint nextListIndex = _listsCount;
            uint prevListIndex = _listsCount;
            Block* nextListBlock = null;
            Block* prevListBlock = null;

            // Check blocks according to strategies
            if ((strategy & (uint)AllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT) != 0)
            {
                // Quick check for larger block first
                nextListBlock = FindFreeBlock(sizeForNextList, ref nextListIndex);
                if (nextListBlock != null && CheckBlock(ref *nextListBlock, nextListIndex, size, alignment, allocType, allocationRequest))
                    return true;

                // If not fitted then null block
                if (CheckBlock(ref *_nullBlock, _listsCount, size, alignment, allocType, allocationRequest))
                    return true;

                // Null block failed, search larger bucket
                while (nextListBlock != null)
                {
                    if (CheckBlock(ref *nextListBlock, nextListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    nextListBlock = nextListBlock->NextFree();
                }

                // Failed again, check best fit bucket
                prevListBlock = FindFreeBlock(size, ref prevListIndex);
                while (prevListBlock != null)
                {
                    if (CheckBlock(ref *prevListBlock, prevListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    prevListBlock = prevListBlock->NextFree();
                }
            }
            else if ((strategy & (uint)AllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT) != 0)
            {
                // Check best fit bucket
                prevListBlock = FindFreeBlock(size, ref prevListIndex);
                while (prevListBlock != null)
                {
                    if (CheckBlock(ref *prevListBlock, prevListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    prevListBlock = prevListBlock->NextFree();
                }

                // If failed check null block
                if (CheckBlock(ref *_nullBlock, _listsCount, size, alignment, allocType, allocationRequest))
                    return true;

                // Check larger bucket
                nextListBlock = FindFreeBlock(sizeForNextList, ref nextListIndex);
                while (nextListBlock != null)
                {
                    if (CheckBlock(ref *nextListBlock, nextListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    nextListBlock = nextListBlock->NextFree();
                }
            }
            else if ((strategy & (uint)AllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT) != 0)
            {
                // Perform search from the start
                using VmaVector<Pointer<Block>> blockList = new VmaVector<Pointer<Block>>(_blocksFreeCount);

                nuint i = _blocksFreeCount;
                for (Block* block = _nullBlock->prevPhysical; block != null; block = block->prevPhysical)
                {
                    if (block->IsFree() && block->size >= size)
                        blockList[--i].Value = block;
                }

                for (; i < _blocksFreeCount; ++i)
                {
                    ref Block block = ref *blockList[i].Value;
                    if (CheckBlock(ref block, GetListIndex(block.size), size, alignment, allocType, allocationRequest))
                        return true;
                }

                // If failed check null block
                if (CheckBlock(ref *_nullBlock, _listsCount, size, alignment, allocType, allocationRequest))
                    return true;

                // Whole range searched, no more memory
                return false;
            }
            else
            {
                // Check larger bucket
                nextListBlock = FindFreeBlock(sizeForNextList, ref nextListIndex);
                while (nextListBlock != null)
                {
                    if (CheckBlock(ref *nextListBlock, nextListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    nextListBlock = nextListBlock->NextFree();
                }

                // If failed check null block
                if (CheckBlock(ref *_nullBlock, _listsCount, size, alignment, allocType, allocationRequest))
                    return true;

                // Check best fit bucket
                prevListBlock = FindFreeBlock(size, ref prevListIndex);
                while (prevListBlock != null)
                {
                    if (CheckBlock(ref *prevListBlock, prevListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    prevListBlock = prevListBlock->NextFree();
                }
            }

            // Worst case, full search has to be done
            while (++nextListIndex < _listsCount)
            {
                nextListBlock = _freeList[nextListIndex].Value;
                while (nextListBlock != null)
                {
                    if (CheckBlock(ref *nextListBlock, nextListIndex, size, alignment, allocType, allocationRequest))
                        return true;
                    nextListBlock = nextListBlock->NextFree();
                }
            }

            // No more memory sadly
            return false;
        }

        public void AddDetailedStatistics(ref DetailedStatistics stats)
        {
            stats.Statistics.BlockCount++;
            stats.Statistics.BlockBytes += Size;
            if (_nullBlock->size > 0)
                VmaAddDetailedStatisticsUnusedRange(ref stats, _nullBlock->size);

            for (Block* block = _nullBlock->prevPhysical; block != null; block = block->prevPhysical)
            {
                if (block->IsFree())
                    VmaAddDetailedStatisticsUnusedRange(ref stats, block->size);
                else
                    VmaAddDetailedStatisticsAllocation(ref stats, block->size);
            }
        }

        private ulong GetDebugMargin() => IsVirtual ? (ulong)0 : DebugMargin;
        public ulong GetSumFreeSize() => _blocksFreeSize + _nullBlock->size;

        private Block* FindFreeBlock(ulong size, ref uint listIndex)
        {
            byte memoryClass = SizeToMemoryClass(size);
            uint innerFreeMap = _innerIsFreeBitmap[memoryClass] & (~0U << SizeToSecondIndex(size, memoryClass));

            if (innerFreeMap == 0)
            {
                // Check higher levels for avaiable blocks
                uint freeMap = _isFreeBitmap & (~0u << (memoryClass + 1));

                if (freeMap == 0)
                {
                    return null; // No more memory avaible
                }

                // Find lowest free region
                memoryClass = VmaBitScanLSB(freeMap);
                innerFreeMap = _innerIsFreeBitmap[memoryClass];

                Debug.Assert(innerFreeMap != 0);
            }
            // Find lowest free subregion
            listIndex = GetListIndex(memoryClass, VmaBitScanLSB(innerFreeMap));
            return _freeList[listIndex].Value;
        }

        private bool CheckBlock(ref Block block, uint listIndex, ulong allocSize, ulong allocAlignment, SuballocationType allocType, AllocationRequest* pAllocationRequest)
        {
            Debug.Assert(block.IsFree(), "Block is already taken!");

            ulong alignedOffset = MathHelper.AlignUp(block.offset, allocAlignment);

            if (block.size < (allocSize + alignedOffset - block.offset))
            {
                return false;
            }

            // Alloc successful
            pAllocationRequest->type = VmaAllocationRequestType.TLSF;
            pAllocationRequest->allocHandle = (ulong)(Unsafe.AsPointer(ref block));
            pAllocationRequest->size = allocSize - GetDebugMargin();
            pAllocationRequest->algorithmData = alignedOffset;

            // Place block at the start of list if it's normal block
            if ((listIndex != _listsCount) && (block.PrevFree() != null))
            {
                block.PrevFree()->NextFree() = block.NextFree();

                if (block.NextFree() != null)
                {
                    block.NextFree()->PrevFree() = block.PrevFree();
                }

                block.PrevFree() = null;
                block.NextFree() = _freeList[listIndex].Value;

                _freeList[listIndex].Value = (Block*)(Unsafe.AsPointer(ref block));

                if (block.NextFree() != null)
                {
                    block.NextFree()->PrevFree() = (Block*)(Unsafe.AsPointer(ref block));
                }
            }

            return true;
        }

        internal partial struct Block : IDisposable
        {
            public ulong offset;
            public ulong size;

            public Block* prevPhysical;

            public Block* nextPhysical;

            private Block* prevFree; // Address of the same block here indicates that block is taken

            private _Anonymous_e__Union Anonymous;

            internal void _ctor()
            {
                offset = 0;
                size = 0;
                prevPhysical = null;
                nextPhysical = null;
                prevFree = null;
                Anonymous = new _Anonymous_e__Union();
            }

            void IDisposable.Dispose()
            {
            }

            public void MarkFree()
            {
                prevFree = null;
            }

            public void MarkTaken()
            {
                prevFree = (Block*)(Unsafe.AsPointer(ref this));
            }

            public readonly bool IsFree()
            {
                return prevFree != (Block*)(Unsafe.AsPointer(ref Unsafe.AsRef(in this)));
            }

            [UnscopedRef]
            public ref void* PrivateData()
            {
                Debug.Assert(!IsFree());
                return ref Anonymous.privateData;
            }

            [UnscopedRef]
            public ref Block* PrevFree()
            {
                return ref prevFree;
            }

            [UnscopedRef]
            public ref Block* NextFree()
            {
                Debug.Assert(IsFree());
                return ref Anonymous.nextFree;
            }

            [StructLayout(LayoutKind.Explicit)]
            private partial struct _Anonymous_e__Union
            {
                [FieldOffset(0)]
                public Block* nextFree;

                [FieldOffset(0)]
                public void* privateData;
            }
        }
    }

    internal partial struct VmaMappingHysteresis
    {
        private const int COUNTER_MIN_EXTRA_MAPPING = 7;

        private uint _minorCounter = 0;
        private uint _majorCounter = 0;

        public VmaMappingHysteresis()
        {

        }

        public uint ExtraMapping { get; private set; }

        // Call when Map was called.
        // Returns true if switched to extra +1 mapping reference count.
        public bool PostMap()
        {
            if (ExtraMapping == 0)
            {
                ++_majorCounter;
                if (_majorCounter >= COUNTER_MIN_EXTRA_MAPPING)
                {
                    ExtraMapping = 1;
                    _majorCounter = 0;
                    _minorCounter = 0;
                    return true;
                }
            }
            else // m_ExtraMapping == 1
            {
                PostMinorCounter();

            }

            return false;
        }

        // Call when Unmap was called.
        public void PostUnmap()
        {
            if (ExtraMapping == 0)
                ++_majorCounter;
            else // m_ExtraMapping == 1
                PostMinorCounter();
        }

        // Call when allocation was made from the memory block.
        public void PostAlloc()
        {
            if (ExtraMapping == 1)
                ++_majorCounter;
            else // m_ExtraMapping == 0
                PostMinorCounter();
        }

        // Call when allocation was freed from the memory block.
        // Returns true if switched to extra -1 mapping reference count.
        public bool PostFree()
        {
            if (ExtraMapping == 1)
            {
                ++_majorCounter;
                if (_majorCounter >= COUNTER_MIN_EXTRA_MAPPING &&
                    _majorCounter > _minorCounter + 1)
                {
                    ExtraMapping = 0;
                    _majorCounter = 0;
                    _minorCounter = 0;
                    return true;
                }
            }
            else // m_ExtraMapping == 0
            {
                PostMinorCounter();
            }

            return false;
        }


        private void PostMinorCounter()
        {
            if (_minorCounter < _majorCounter)
            {
                ++_minorCounter;
            }
            else if (_majorCounter > 0)
            {
                --_majorCounter;
                --_minorCounter;
            }
        }
    }

    internal unsafe partial class VmaDeviceMemoryBlock : IDisposable
    {
        private readonly VulkanMemoryAllocator _allocator;
        private readonly VmaMappingHysteresis _mappingHysteresis = new();
        private uint _mapCount;
        private void* _pMappedData;
        private readonly object _syncLock = new object();

        public VmaDeviceMemoryBlock(VulkanMemoryAllocator allocator,
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

        public void PostAlloc(VulkanMemoryAllocator allocator)
        {
            lock (_syncLock)
            {
                _mappingHysteresis.PostAlloc();
            }
        }

        public void PostFree(VulkanMemoryAllocator allocator)
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

        public void Destroy(VulkanMemoryAllocator allocator)
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

        public VkResult Map(VulkanMemoryAllocator allocator, uint count, void** ppData)
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
                        _allocator._device,
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

        public void Unmap(VulkanMemoryAllocator allocator, uint count)
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
                        vkUnmapMemory(allocator._device, Memory);
                    }
                    _mappingHysteresis.PostUnmap();
                }
                else
                {
                    Debug.Assert(false, "VkDeviceMemory block is being unmapped while it was not previously mapped.");
                }
            }
        }

        public VkResult BindBufferMemory(VulkanMemoryAllocator allocator, Allocation allocation, ulong allocationLocalOffset, VkBuffer buffer, void* pNext)
        {
            Debug.Assert(allocation.AllocationType == AllocationType.Block && allocation.GetBlock() == this);
            Debug.Assert(allocationLocalOffset < allocation.Size, "Invalid allocationLocalOffset. Did you forget that this offset is relative to the beginning of the allocation, not the whole memory block?");
            ulong memoryOffset = allocation.Offset + allocationLocalOffset;
            // This lock is important so that we don't call vkBind... and/or vkMap... simultaneously on the same VkDeviceMemory from multiple threads.
            lock (_syncLock)
            {
                return allocator.BindVulkanBuffer(Memory, memoryOffset, buffer, pNext);
            }
        }

        public VkResult BindImageMemory(VulkanMemoryAllocator allocator, Allocation allocation, ulong allocationLocalOffset, VkImage image, void* pNext)
        {
            Debug.Assert(allocation.AllocationType == AllocationType.Block && allocation.GetBlock() == this);
            Debug.Assert(allocationLocalOffset < allocation.Size, "Invalid allocationLocalOffset. Did you forget that this offset is relative to the beginning of the allocation, not the whole memory block?");
            ulong memoryOffset = allocation.Offset + allocationLocalOffset;
            // This lock is important so that we don't call vkBind... and/or vkMap... simultaneously on the same VkDeviceMemory from multiple threads.
            lock (_syncLock)
            {
                return allocator.BindVulkanImage(Memory, memoryOffset, image, pNext);
            }
        }
    }

    class BlockVector : IDisposable
    {
        private readonly VulkanMemoryAllocator _allocator;
        private readonly ReaderWriterLockSlim _mutex = new(LockRecursionPolicy.NoRecursion);
        // Incrementally sorted by sumFreeSize, ascending.
        private readonly List<VmaDeviceMemoryBlock> _blocks = new();
        private uint _nextBlockId;
        private readonly void* _pMemoryAllocateNext;

        public BlockVector(VulkanMemoryAllocator allocator,
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
            for (int i = _blocks.Count; i-- != 0;)
            {
                _blocks[i].Destroy(_allocator);
                _blocks[i].Dispose();
            }
        }

        public void Free(Allocation allocation)
        {
            VmaDeviceMemoryBlock? blockToDelete = default;

            bool budgetExceeded = false;
            {
                int heapIndex = _allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex);
                Span<MemoryBudget> heapBudget = _allocator.GetHeapBudgets(heapIndex, 1);
                budgetExceeded = heapBudget[0].Usage >= heapBudget[0].Budget;
            }

            // Scope for lock.
            {
                // VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT
                //VmaMutexLockWrite lock (m_Mutex, m_hAllocator->m_UseMutex);
                _mutex.EnterWriteLock();

                try
                {
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

                    bool canDeleteBlock = _blocks.Count > MinBlockCount;
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
                finally
                {
                    _mutex.ExitWriteLock();
                }
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

        public void AddDetailedStatistics(ref DetailedStatistics stats)
        {
            _mutex.EnterReadLock();
            try
            {
                int blockCount = _blocks.Count;
                for (int blockIndex = 0; blockIndex < blockCount; ++blockIndex)
                {
                    VmaDeviceMemoryBlock pBlock = _blocks[blockIndex];
                    Debug.Assert(pBlock != null);
                    Debug.Assert(pBlock.Validate());
                    pBlock.MetaData.AddDetailedStatistics(ref stats);
                }
            }
            finally
            {
                _mutex.ExitReadLock();
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
            VmaDeviceMemoryBlock block = new(_allocator, ParentPool, MemoryTypeIndex, memory,
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

            if (res != VkResult.Success)
            {
                // Free the already created allocation.
                Free(allocation!);
            }

            return res;
        }

        private VkResult AllocatePage(ulong size, ulong alignment, in AllocationCreateInfo createInfo, SuballocationType suballocType, out Allocation? allocation)
        {
            allocation = default;
            bool isUpperAddress = (createInfo.Flags & AllocationCreateFlags.UpperAddress) != 0;

            ulong freeMemory;
            {
                int heapIndex = _allocator.MemoryTypeIndexToHeapIndex(MemoryTypeIndex);
                Span<MemoryBudget> heapBudget = _allocator.GetHeapBudgets(heapIndex, 1);
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
            AllocationCreateFlags allocFlags,
            object? userData,
            SuballocationType suballocType,
            uint strategy,
            out Allocation? allocation)
        {
            bool isUpperAddress = (allocFlags & AllocationCreateFlags.UpperAddress) != 0;

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
            AllocationCreateFlags allocFlags,
            object? userData,
            SuballocationType suballocType,
            out Allocation? allocation)
        {
            bool mapped = (allocFlags & AllocationCreateFlags.Mapped) != 0;
            //bool isUserDataString = (allocFlags & AllocationCreateFlags.u VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT) != 0;
            bool isMappingAllowed = (allocFlags & (AllocationCreateFlags.HostAccessSequentialWrite | AllocationCreateFlags.HostAccessRandom)) != 0;

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

        public void AddAllocation(int heapIndex, ulong allocationSize)
        {
            _ = Interlocked.Add(ref AllocationBytes[heapIndex], allocationSize);
            _ = Interlocked.Increment(ref AllocationCount[heapIndex]);
            _ = Interlocked.Increment(ref OperationsSinceBudgetFetch);
        }

        public void RemoveAllocation(int heapIndex, ulong allocationSize)
        {
            Debug.Assert(AllocationBytes[heapIndex] >= allocationSize);
            AllocationBytes[heapIndex] -= allocationSize;
            Debug.Assert(AllocationCount[heapIndex] > 0);
            _ = Interlocked.Decrement(ref AllocationCount[heapIndex]);
            _ = Interlocked.Increment(ref OperationsSinceBudgetFetch);
        }
    }
}

public enum AllocationType
{
    None,
    Block,
    Dedicated
}

/// <summary>
/// The object containing details on a suballocation of Vulkan Memory
/// </summary>
public unsafe class Allocation : IDisposable
{
    private Flags _flags;
    // Reference counter for vmaMapMemory()/vmaUnmapMemory().
    private byte _mapCount;
    // Allocation out of VmaDeviceMemoryBlock.
    //struct BlockAllocation
    //{
    //    VmaDeviceMemoryBlock* m_Block;
    //    VmaAllocHandle m_AllocHandle;
    //};
    private VulkanMemoryAllocator.VmaDeviceMemoryBlock? _block;
    private ulong _allocHandle;

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
    public Allocation(bool mappingAllowed)
    {
        if (mappingAllowed)
            _flags |= Flags.MappingAllowed;
    }

    public void Dispose()
    {
        Debug.Assert(_mapCount == 0, "Allocation was not unmapped before destruction.");
    }

    public ulong Alignment { get; private set; } = 1;
    public ulong Size { get; private set; }
    public ulong Offset
    {
        get
        {
            switch (AllocationType)
            {
                case AllocationType.Block:
                    return _block!.MetaData.GetAllocationOffset(_allocHandle);
                case AllocationType.Dedicated:
                    return 0;
                default:
                    Debug.Assert(false);
                    return 0;
            }
        }
    }
    public AllocationType AllocationType { get; private set; }
    public int MemoryTypeIndex { get; private set; }

    public bool IsPersistentMap => (_flags & Flags.PersistentMap) != 0;
    public bool IsMappingAllowed => (_flags & Flags.MappingAllowed) != 0;

    internal VulkanMemoryAllocator.SuballocationType SuballocationType { get; private set; }

    internal void InitBlockAllocation(VulkanMemoryAllocator.VmaDeviceMemoryBlock block,
        ulong allocHandle,
        ulong alignment,
        ulong size,
        int memoryTypeIndex,
        VulkanMemoryAllocator.SuballocationType suballocationType,
        bool mapped)
    {
        Debug.Assert(AllocationType == AllocationType.None);
        Debug.Assert(block != null);
        AllocationType = AllocationType.Block;
        Alignment = alignment;
        Size = size;
        MemoryTypeIndex = memoryTypeIndex;
        if (mapped)
        {
            Debug.Assert(IsMappingAllowed, "Mapping is not allowed on this allocation! Please use one of the new VMA_ALLOCATION_CREATE_HOST_ACCESS_* flags when creating it.");
            _flags |= Flags.PersistentMap;
        }

        SuballocationType = suballocationType;
        _block = block;
        _allocHandle = allocHandle;
    }

    internal VulkanMemoryAllocator.VmaDeviceMemoryBlock GetBlock() => _block!;
    internal ulong GetAllocHandle() => _allocHandle;

    public VkDeviceMemory GetMemory()
    {
        switch (AllocationType)
        {
            case AllocationType.Block:
                return _block!.Memory;
            case AllocationType.Dedicated:
                return _dedicatedAllocationMemory;
            default:
                Debug.Assert(false);
                return VkDeviceMemory.Null;
        }
    }

    public void* GetMappedData()
    {
        switch (AllocationType)
        {
            case AllocationType.Block:
                if (_mapCount != 0 || IsPersistentMap)
                {
                    void* pBlockData = _block!.GetMappedData();
                    Debug.Assert(pBlockData != null);
                    return (char*)pBlockData + Offset;
                }
                return null;

            case AllocationType.Dedicated:
                Debug.Assert((_dedicatedAllocationMappedData != null) == (_mapCount != 0 || IsPersistentMap));
                return _dedicatedAllocationMappedData;
            default:
                Debug.Assert(false);
                return null;
        }
    }

    public void BlockAllocMap()
    {
        Debug.Assert(AllocationType == AllocationType.Block);
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
        Debug.Assert(AllocationType == AllocationType.Block);

        if (_mapCount > 0)
        {
            --_mapCount;
        }
        else
        {
            Debug.Assert(false, "Unmapping allocation not previously mapped.");
        }
    }

    internal VkResult DedicatedAllocMap(VulkanMemoryAllocator allocator, void** ppData)
    {
        Debug.Assert(AllocationType == AllocationType.Dedicated);
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

    internal void DedicatedAllocUnmap(VulkanMemoryAllocator allocator)
    {
        Debug.Assert(AllocationType == AllocationType.Dedicated);

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

internal readonly struct AllocationInfo
{
    public uint MemoryType { get; init; }
    public VkDeviceMemory DeviceMemory { get; init; }
    public ulong Offset { get; init; }
    public ulong Size { get; init; }
    public unsafe void* pMappedData { get; init; }
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
    /** Allocation will be created from upper stack in a double stack pool.

    This flag is only allowed for custom pools created with #VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT flag.
    */
    UpperAddress = 0x00000040,
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

