// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using System.Numerics;
using XenoAtom.Collections;
using static Vortice.Vulkan.VmaUtils;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Vulkan;

public unsafe partial class VmaAllocator : IDisposable
{
    public const long DebugMargin = 0;                         // VMA_DEBUG_MARGIN
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

    private readonly VkExternalMemoryHandleTypeFlags[] _typeExternalMemoryHandleTypes = new VkExternalMemoryHandleTypeFlags[VK_MAX_MEMORY_TYPES];
    private readonly VmaBlockVector[] _blockVectors = new VmaBlockVector[VK_MAX_MEMORY_TYPES]; //Default Pools
    private readonly VmaDedicatedAllocationList[] _dedicatedAllocations = new VmaDedicatedAllocationList[VK_MAX_MEMORY_TYPES];

    private readonly ReaderWriterLockSlim _poolsMutex = new();
    private readonly UnsafeList<VmaPool> _pools = [];
    internal uint _nextPoolID;

    internal readonly CurrentBudgetData _budget = new();
    private uint _deviceMemoryCount; // Total number of VkDeviceMemory objects.

    // Each bit (1 << i) is set if HeapSizeLimit is enabled for that heap, so cannot allocate more than the heap size.
    private readonly uint _heapSizeLimitMask;

    private volatile uint _currentFrameIndex = 0;

    public VmaAllocator(in VmaAllocatorCreateInfo createInfo)
    {
        _device = createInfo.Device;
        _physicalDevice = createInfo.PhysicalDevice;
        VulkanApiVersion = createInfo.VulkanApiVersion;

        Debug.Assert(_device.IsNotNull);
        Debug.Assert(_physicalDevice.IsNotNull);

        UseMutex = (createInfo.Flags & VmaAllocatorCreateFlags.ExternallySynchronized) == 0;
        UseKhrDedicatedAllocation = true;
        UseKhrBindMemory2 = true;
        UseExtMemoryBudget = (createInfo.Flags & VmaAllocatorCreateFlags.ExtMemoryBudget) != 0;
        UseAmdDeviceCoherentMemory = (createInfo.Flags & VmaAllocatorCreateFlags.AMDDeviceCoherentMemory) != 0;
        UseKhrBufferDeviceAddress = (createInfo.Flags & VmaAllocatorCreateFlags.BufferDeviceAddress) != 0;
        UseExtMemoryPriority = (createInfo.Flags & VmaAllocatorCreateFlags.ExtMemoryPriority) != 0;
        UseKhrMaintenance4 = (createInfo.Flags & VmaAllocatorCreateFlags.KhrMaintenance4) != 0;
        UseKhrMaintenance5 = (createInfo.Flags & VmaAllocatorCreateFlags.KhrMaintenance5) != 0;

        vkGetPhysicalDeviceProperties(_physicalDevice, out _physicalDeviceProperties);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, out _memoryProperties);

        Debug.Assert(BitOperations.IsPow2(MinAlignment));
        Debug.Assert(BitOperations.IsPow2(DebugMinBufferImageGranularity));
        Debug.Assert(BitOperations.IsPow2(_physicalDeviceProperties.limits.bufferImageGranularity));
        Debug.Assert(BitOperations.IsPow2(_physicalDeviceProperties.limits.nonCoherentAtomSize));

        _preferredLargeHeapBlockSize = (createInfo.PreferredLargeHeapBlockSize != 0) ? createInfo.PreferredLargeHeapBlockSize : DefaultLargeHeapBlockSize;

        GlobalMemoryTypeBits = CalculateGlobalMemoryTypeBits();

        if (createInfo.pTypeExternalMemoryHandleTypes != null)
        {
            for (int i = 0; i < MemoryTypeCount; i++)
            {
                _typeExternalMemoryHandleTypes[i] = createInfo.pTypeExternalMemoryHandleTypes[i];
            }
        }

        if (createInfo.pHeapSizeLimit != null)
        {
            for (int heapIndex = 0; heapIndex < MemoryHeapCount; ++heapIndex)
            {
                ulong limit = createInfo.pHeapSizeLimit[heapIndex];
                if (limit != VK_WHOLE_SIZE)
                {
                    _heapSizeLimitMask |= 1u << heapIndex;
                    if (limit < _memoryProperties.memoryHeaps[heapIndex].size)
                    {
                        _memoryProperties.memoryHeaps[heapIndex].size = limit;
                    }
                }
            }
        }

        for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
        {
            // Create only supported types
            if ((GlobalMemoryTypeBits & (1u << memTypeIndex)) != 0)
            {
                ulong preferredBlockSize = CalcPreferredBlockSize(memTypeIndex);
                _blockVectors[memTypeIndex] = new VmaBlockVector(
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

        if (UseExtMemoryBudget)
        {
            UpdateVulkanBudget();
        }
    }

    /// <summary>
    /// Gets <c>true</c> if the object has been disposed; otherwise, <c>false</c>.
    /// </summary>
    public bool IsDisposed => _isDisposed != 0;

    public ref readonly VkPhysicalDevice PhysicalDevice => ref _physicalDevice;
    public ref readonly VkDevice Device => ref _device;

    public VkVersion VulkanApiVersion { get; }

    public uint MemoryHeapCount => _memoryProperties.memoryHeapCount;

    public uint MemoryTypeCount => _memoryProperties.memoryTypeCount;

    public ulong BufferImageGranularity => Math.Max(DebugMinBufferImageGranularity, _physicalDeviceProperties.limits.bufferImageGranularity);

    public uint GlobalMemoryTypeBits { get; }

    public bool UseMutex { get; }
    public bool UseKhrDedicatedAllocation { get; } = true;
    public bool UseKhrBindMemory2 { get; } = true;
    public bool UseExtMemoryBudget { get; }

    public bool UseAmdDeviceCoherentMemory { get; }
    public bool UseKhrBufferDeviceAddress { get; }
    public bool UseExtMemoryPriority { get; }
    public bool UseKhrMaintenance4 { get; }
    public bool UseKhrMaintenance5 { get; }

    // vmaSetCurrentFrameIndex
    public uint CurrentFrameIndex
    {
        get => _currentFrameIndex;
        set
        {
            _currentFrameIndex = value;

            if (UseExtMemoryBudget)
            {
                UpdateVulkanBudget();
            }
        }
    }

    // vmaGetPhysicalDeviceProperties
    public ref readonly VkPhysicalDeviceProperties PhysicalDeviceProperties => ref _physicalDeviceProperties;
    // vmaGetMemoryProperties
    public ref readonly VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties => ref _memoryProperties;

    /// <summary>
    /// Finalizes an instance of the <see cref="VmaAllocator" /> class.
    /// </summary>
    ~VmaAllocator() => Dispose(disposing: false);

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

    // vmaGetMemoryTypeProperties
    public VkMemoryPropertyFlags GetMemoryTypeProperties(int memoryTypeIndex)
    {
        Debug.Assert(memoryTypeIndex < MemoryTypeCount);
        return _memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
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

    // vmaGetHeapBudgets
    public void GetHeapBudgets(Span<VmaBudget> budgets, int firstHeap, int heapCount)
    {
        fixed (VmaBudget* budgetsPtr = budgets)
        {
            GetHeapBudgets(budgetsPtr, firstHeap, heapCount);
        }
    }

    public void GetHeapBudgets(VmaBudget* budgets, int firstHeap, int heapCount)
    {
        if (UseExtMemoryBudget)
        {
            if (_budget.OperationsSinceBudgetFetch < 30)
            {
                using VmaMutexLockRead lockRead = new(_budget.BudgetMutex, UseMutex);

                for (int i = 0; i < heapCount; ++i)
                {
                    int heapIndex = firstHeap + i;

                    budgets[i].Statistics.BlockCount = _budget.BlockCount[heapIndex];
                    budgets[i].Statistics.AllocationCount = _budget.AllocationCount[heapIndex];
                    budgets[i].Statistics.BlockBytes = _budget.BlockBytes[heapIndex];
                    budgets[i].Statistics.AllocationBytes = _budget.AllocationBytes[heapIndex];

                    if (_budget.VulkanUsage[heapIndex] + budgets[i].Statistics.BlockBytes > _budget.BlockBytesAtBudgetFetch[heapIndex])
                    {
                        budgets[i].Usage = _budget.VulkanUsage[heapIndex] +
                            budgets[i].Statistics.BlockBytes - _budget.BlockBytesAtBudgetFetch[heapIndex];
                    }
                    else
                    {
                        budgets[i].Usage = 0;
                    }

                    // Have to take MIN with heap size because explicit HeapSizeLimit is included in it.
                    budgets[i].Budget = Math.Min(_budget.VulkanBudget[heapIndex], _memoryProperties.memoryHeaps[heapIndex].size);
                }
            }
            else
            {
                UpdateVulkanBudget(); // Outside of mutex lock
                GetHeapBudgets(budgets, firstHeap, heapCount); // Recursion
            }
        }
        else
        {
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
        }
    }

    private void UpdateVulkanBudget()
    {
        Debug.Assert(UseExtMemoryBudget);

        VkPhysicalDeviceMemoryProperties2 memProps = new();
        VkPhysicalDeviceMemoryBudgetPropertiesEXT budgetProps = new();
        budgetProps.pNext = memProps.pNext;
        memProps.pNext = &budgetProps;
        vkGetPhysicalDeviceMemoryProperties2(_physicalDevice, &memProps);

        using VmaMutexLockWrite lockWrite = new(_budget.BudgetMutex, UseMutex);

        for (int heapIndex = 0; heapIndex < MemoryHeapCount; ++heapIndex)
        {
            _budget.VulkanUsage[heapIndex] = budgetProps.heapUsage[heapIndex];
            _budget.VulkanBudget[heapIndex] = budgetProps.heapBudget[heapIndex];
            _budget.BlockBytesAtBudgetFetch[heapIndex] = _budget.BlockBytes[heapIndex];

            // Some bugged drivers return the budget incorrectly, e.g. 0 or much bigger than heap size.
            if (_budget.VulkanBudget[heapIndex] == 0)
            {
                _budget.VulkanBudget[heapIndex] = _memoryProperties.memoryHeaps[heapIndex].size * 8 / 10; // 80% heuristics.
            }
            else if (_budget.VulkanBudget[heapIndex] > _memoryProperties.memoryHeaps[heapIndex].size)
            {
                _budget.VulkanBudget[heapIndex] = _memoryProperties.memoryHeaps[heapIndex].size;
            }
            if (_budget.VulkanUsage[heapIndex] == 0 && _budget.BlockBytesAtBudgetFetch[heapIndex] > 0)
            {
                _budget.VulkanUsage[heapIndex] = _budget.BlockBytesAtBudgetFetch[heapIndex];
            }
        }
        _budget.OperationsSinceBudgetFetch = 0;
    }

    // vmaCalculateStatistics
    public VmaTotalStatistics CalculateStatistics()
    {
        CalculateStatistics(out VmaTotalStatistics stats);
        return stats;
    }

    public void CalculateStatistics(out VmaTotalStatistics stats)
    {
        stats = new();
        // Process default pools.
        for (int memTypeIndex = 0; memTypeIndex < MemoryTypeCount; ++memTypeIndex)
        {
            VmaBlockVector blockVector = _blockVectors[memTypeIndex];
            blockVector?.AddDetailedStatistics(ref stats.MemoryType[memTypeIndex]);
        }

        // Process custom pools.
        {
            //using VmaMutexLockRead lockMutext = new(_poolsMutex, UseMutex);
            //for (VmaPool pool = _pools.Front(); pool != VMA_NULL; pool = m_Pools.GetNext(pool))
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
            VmaAddDetailedStatistics(ref stats.MemoryHeap[memHeapIndex], stats.MemoryType[memTypeIndex]);
        }

        // Sum from memory heaps to total.
        for (int memHeapIndex = 0; memHeapIndex < MemoryHeapCount; ++memHeapIndex)
            VmaAddDetailedStatistics(ref stats.Total, stats.MemoryHeap[memHeapIndex]);

        Debug.Assert(stats.Total.Statistics.AllocationCount == 0 || stats.Total.AllocationSizeMax >= stats.Total.AllocationSizeMin);
        Debug.Assert(stats.Total.UnusedRangeCount == 0 || stats.Total.UnusedRangeSizeMax >= stats.Total.UnusedRangeSizeMin);
    }

    // vmaGetAllocationInfo/vmaGetAllocationInfo2
    public VmaAllocationInfo GetAllocationInfo(VmaAllocation allocation)
    {
        ulong blockSize = 0;
        bool dedicatedMemory = false;
        switch (allocation.Type)
        {
            case VmaAllocationType.Block:
                blockSize = allocation.GetBlock().MetaData.Size;
                dedicatedMemory = false;
                break;
            case VmaAllocationType.Dedicated:
                //blockSize = allocation->allocationInfo.size;
                dedicatedMemory = true;
                break;
            default:
                Debug.Assert(false);
                break;
        }

        return new()
        {
            MemoryType = (uint)allocation.MemoryTypeIndex,
            DeviceMemory = allocation.GetMemory(),
            Offset = allocation.Offset,
            Size = allocation.Size,
            pMappedData = allocation.GetMappedData(),
            BlockSize = blockSize,
            DedicatedMemory = dedicatedMemory
        };
        //pAllocationInfo->pUserData = hAllocation->GetUserData();
        //pAllocationInfo->pName = hAllocation->GetName();
    }

    // vmaGetAllocationMemoryProperties
    public void GetAllocationMemoryProperties(VmaAllocation allocation, out VkMemoryPropertyFlags flags)
    {
        int memTypeIndex = allocation.MemoryTypeIndex;
        flags = _memoryProperties.memoryTypes[memTypeIndex].propertyFlags;
    }


    public VkExternalMemoryHandleTypeFlags GetExternalMemoryHandleTypeFlags(int memTypeIndex)
    {
        return _typeExternalMemoryHandleTypes[memTypeIndex];
    }

    #region Buffer
    public VkResult CreateBuffer(in VkBufferCreateInfo createInfo, VmaAllocationCreateInfo allocationCreateInfo, out VkBuffer buffer, out VmaAllocation? allocation)
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

        VkResult result = vkCreateBuffer(_device.Handle, in createInfo, null, out buffer);
        if (result >= VkResult.Success)
        {
            // 2. vkGetBufferMemoryRequirements.
            GetBufferMemoryRequirements(buffer, out VkMemoryRequirements vkMemReq, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation);

            // 3. Allocate memory using allocator.
            result = AllocateMemory(ref allocationCreateInfo, in vkMemReq, requiresDedicatedAllocation, prefersDedicatedAllocation,
                buffer, // dedicatedBuffer
                VkImage.Null, // dedicatedImage
                new VmaBufferImageUsage(in createInfo, UseKhrMaintenance5), // dedicatedBufferImageUsage
                VmaSuballocationType.Buffer,
                out allocation);

            if (result >= 0)
            {
                // 3. Bind buffer with memory.
                if ((allocationCreateInfo.Flags & VmaAllocationCreateFlags.DontBind) == 0)
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

    public void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
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

    public VkResult BindBufferMemory(VmaAllocation allocation, ulong allocationLocalOffset, VkBuffer buffer, void* pNext)
    {
        VkResult res = VkResult.ErrorUnknown;
        switch (allocation.Type)
        {
            case VmaAllocationType.Dedicated:
                res = BindVulkanBuffer(allocation.GetMemory(), allocationLocalOffset, buffer, pNext);
                break;
            case VmaAllocationType.Block:
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
            VmaPnextChainPushFront(&memReq2, &memDedicatedReq);

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
    public VkResult CreateImage(VkImageCreateInfo createInfo, VmaAllocationCreateInfo allocationCreateInfo, out VkImage image, out VmaAllocation? allocation)
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
            VmaSuballocationType suballocType = createInfo.tiling == VkImageTiling.Optimal ? VmaSuballocationType.ImageOptimal : VmaSuballocationType.ImageLinear;

            // 2. Allocate memory using allocator.
            GetImageMemoryRequirements(image, out VkMemoryRequirements vkMemReq, out bool requiresDedicatedAllocation, out bool prefersDedicatedAllocation);

            // 3. Allocate memory using allocator.
            result = AllocateMemory(ref allocationCreateInfo, in vkMemReq, requiresDedicatedAllocation, prefersDedicatedAllocation,
                VkBuffer.Null, // dedicatedBuffer
                image, // dedicatedImage
                new VmaBufferImageUsage(in createInfo), // dedicatedBufferImageUsage
                suballocType,
                out allocation);

            if (result >= 0)
            {
                // 3. Bind buffer with memory.
                if ((allocationCreateInfo.Flags & VmaAllocationCreateFlags.DontBind) == 0)
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

    public void DestroyImage(VkImage image, VmaAllocation allocation)
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

    public VkResult BindImageMemory(VmaAllocation allocation, ulong allocationLocalOffset, VkImage image, void* pNext)
    {
        switch (allocation.Type)
        {
            case VmaAllocationType.Dedicated:
                return BindVulkanImage(allocation.GetMemory(), allocationLocalOffset, image, pNext);
            case VmaAllocationType.Block:
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

    // vmaMapMemory
    public VkResult MapMemory(VmaAllocation allocation, void** ppData)
    {
        switch (allocation.Type)
        {
            case VmaAllocationType.Block:
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
            case VmaAllocationType.Dedicated:
                return allocation.DedicatedAllocMap(this, ppData);
            default:
                Debug.Assert(false);
                return VkResult.ErrorMemoryMapFailed;
        }
    }

    // vmaUnmapMemory
    public void UnmapMemory(VmaAllocation allocation)
    {
        switch (allocation.Type)
        {
            case VmaAllocationType.Block:
                {
                    VmaDeviceMemoryBlock block = allocation.GetBlock();
                    allocation.BlockAllocUnmap();
                    block.Unmap(this, 1);
                }
                break;
            case VmaAllocationType.Dedicated:
                allocation.DedicatedAllocUnmap(this);
                break;
            default:
                Debug.Assert(false);
                break;
        }
    }

    private VkResult FindMemoryTypeIndex(uint memoryTypeBits,
        in VmaAllocationCreateInfo createInfo,
        VmaBufferImageUsage bufferOrImageUsage,
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


    // vmaFindMemoryTypeIndex
    public VkResult FindMemoryTypeIndex(uint memoryTypeBits,
        in VmaAllocationCreateInfo allocationCreateInfo,
        out int memoryTypeIndex)
    {
        return FindMemoryTypeIndex(memoryTypeBits, in allocationCreateInfo, VmaBufferImageUsage.Unknown, out memoryTypeIndex);
    }

    // vmaFindMemoryTypeIndexForBufferInfo
    public VkResult FindMemoryTypeIndexForBufferInfo(
            in VkBufferCreateInfo bufferCreateInfo,
            in VmaAllocationCreateInfo allocationCreateInfo,
            out int memoryTypeIndex)
    {
        VkResult result;
        if (VulkanApiVersion >= VkVersion.Version_1_3 || UseKhrMaintenance4)
        {
            // Can query straight from VkBufferCreateInfo :)
            fixed (VkBufferCreateInfo* bufferCreateInfoPtr = &bufferCreateInfo)
            {
                VkDeviceBufferMemoryRequirements devBufMemReq = new()
                {
                    pCreateInfo = bufferCreateInfoPtr
                };

                VkMemoryRequirements2 memReq = new();
                if (VulkanApiVersion >= VkVersion.Version_1_3)
                    vkGetDeviceBufferMemoryRequirements(_device, &devBufMemReq, &memReq);
                else
                    vkGetDeviceBufferMemoryRequirementsKHR(_device, &devBufMemReq, &memReq);

                result = FindMemoryTypeIndex(
                    memReq.memoryRequirements.memoryTypeBits, allocationCreateInfo,
                    new VmaBufferImageUsage(in bufferCreateInfo, UseKhrMaintenance5),
                    out memoryTypeIndex);
            }
        }
        else
        {
            // Must create a dummy buffer to query :(
            VkBuffer hBuffer = VkBuffer.Null;
            result = vkCreateBuffer(_device, in bufferCreateInfo, null, &hBuffer);
            if (result == VkResult.Success)
            {
                vkGetBufferMemoryRequirements(_device, hBuffer, out VkMemoryRequirements memReq);

                result = FindMemoryTypeIndex(
                    memReq.memoryTypeBits, allocationCreateInfo,
                    new VmaBufferImageUsage(in bufferCreateInfo, UseKhrMaintenance5),
                    out memoryTypeIndex);

                vkDestroyBuffer(_device, hBuffer);
            }
        }

        memoryTypeIndex = int.MaxValue;
        return result;
    }

    // vmaFindMemoryTypeIndexForImageInfo
    public VkResult FindMemoryTypeIndexForImageInfo(
        in VkImageCreateInfo imageCreateInfo,
        in VmaAllocationCreateInfo allocationCreateInfo,
        out int memoryTypeIndex)
    {
        VkResult result;
        if (VulkanApiVersion >= VkVersion.Version_1_3 || UseKhrMaintenance4)
        {
            Debug.Assert(
                (int)imageCreateInfo.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT_COPY && ((uint)imageCreateInfo.flags & VK_IMAGE_CREATE_DISJOINT_BIT_COPY) == 0,
                "Cannot use this VkImageCreateInfo with vmaFindMemoryTypeIndexForImageInfo as I don't know what to pass as VkDeviceImageMemoryRequirements::planeAspect.");

            // Can query straight from VkImageCreateInfo  :)
            fixed (VkImageCreateInfo* imageCreateInfoPtr = &imageCreateInfo)
            {
                VkDeviceImageMemoryRequirements devImgMemReq = new()
                {
                    pCreateInfo = imageCreateInfoPtr
                };

                VkMemoryRequirements2 memReq = new();
                if (VulkanApiVersion >= VkVersion.Version_1_3)
                    vkGetDeviceImageMemoryRequirements(_device, &devImgMemReq, &memReq);
                else
                    vkGetDeviceImageMemoryRequirements(_device, &devImgMemReq, &memReq);

                result = FindMemoryTypeIndex(
                    memReq.memoryRequirements.memoryTypeBits, allocationCreateInfo,
                    new VmaBufferImageUsage(in imageCreateInfo),
                    out memoryTypeIndex);
            }
        }
        else
        {
            // Must create a dummy image to query :(
            VkImage hImage = VkImage.Null;
            result = vkCreateImage(_device, in imageCreateInfo, null, &hImage);
            if (result == VkResult.Success)
            {
                vkGetImageMemoryRequirements(_device, hImage, out VkMemoryRequirements memReq);

                result = FindMemoryTypeIndex(
                    memReq.memoryTypeBits, allocationCreateInfo,
                    new VmaBufferImageUsage(in imageCreateInfo),
                    out memoryTypeIndex);

                vkDestroyImage(_device, hImage);
            }
        }

        memoryTypeIndex = int.MaxValue;
        return result;
    }

    // vmaCreatePool
    public VkResult CreatePool(in VmaPoolCreateInfo createInfo, out VmaPool? pool)
    {
        VmaPoolCreateInfo newCreateInfo = createInfo;

        // Protection against uninitialized new structure member. If garbage data are left there, this pointer dereference would crash.
        if (createInfo.pMemoryAllocateNext != null)
        {
            Debug.Assert(((VkBaseInStructure*)createInfo.pMemoryAllocateNext)->sType != 0);
        }

        pool = default;
        if (newCreateInfo.MaxBlockCount == 0)
            newCreateInfo.MaxBlockCount = nuint.MaxValue;

        if (newCreateInfo.MinBlockCount > newCreateInfo.MaxBlockCount)
            return VkResult.ErrorInitializationFailed;

        // Memory type index out of range or forbidden.
        if (createInfo.MemoryTypeIndex >= MemoryTypeCount ||
            ((1u << createInfo.MemoryTypeIndex) & GlobalMemoryTypeBits) == 0)
        {
            return VK_ERROR_FEATURE_NOT_PRESENT;
        }
        if (newCreateInfo.MinAllocationAlignment > 0)
        {
            Debug.Assert(BitOperations.IsPow2(newCreateInfo.MinAllocationAlignment));
        }

        ulong preferredBlockSize = CalcPreferredBlockSize(newCreateInfo.MemoryTypeIndex);

        pool = new VmaPool(this, newCreateInfo, preferredBlockSize);

        VkResult res = pool.BlockVector.CreateMinBlocks();
        if (res != VkResult.Success)
        {
            pool.Dispose();
            pool = default;
            return res;
        }

        // Add to m_Pools.
        {
            using VmaMutexLockWrite lockMutex = new(_poolsMutex, UseMutex);
            pool.Id = _nextPoolID++;
            _pools.Add(pool);
        }

        return VkResult.Success;
    }

    // vmaDestroyPool
    public void DestroyPool(in VmaPool pool)
    {
        if (pool == null)
            return;

        // Remove from m_Pools.
        {
            using VmaMutexLockWrite lockMutext = new(_poolsMutex, UseMutex);
            _pools.Remove(pool);
        }

        pool.Dispose();
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
        in VmaAllocationCreateInfo createInfo,
        VmaBufferImageUsage bufImgUsage,
        out VkMemoryPropertyFlags requiredFlags,
        out VkMemoryPropertyFlags preferredFlags,
        out VkMemoryPropertyFlags notPreferredFlags)
    {
        requiredFlags = createInfo.RequiredFlags;
        preferredFlags = createInfo.PreferredFlags;
        notPreferredFlags = 0;

        switch (createInfo.Usage)
        {
            case VmaMemoryUsage.Unknown:
                break;
            case VmaMemoryUsage.GpuOnly:
                if (!isIntegratedGPU || (preferredFlags & VkMemoryPropertyFlags.HostVisible) == 0)
                {
                    preferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                }
                break;
            case VmaMemoryUsage.CpuOnly:
                requiredFlags |= VkMemoryPropertyFlags.HostVisible | VkMemoryPropertyFlags.HostCoherent;
                break;
            case VmaMemoryUsage.CpuToGpu:
                requiredFlags |= VkMemoryPropertyFlags.HostVisible;
                if (!isIntegratedGPU || (preferredFlags & VkMemoryPropertyFlags.HostVisible) == 0)
                {
                    preferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                }
                break;
            case VmaMemoryUsage.GpuToCpu:
                requiredFlags |= VkMemoryPropertyFlags.HostVisible;
                preferredFlags |= VkMemoryPropertyFlags.HostCached;
                break;
            case VmaMemoryUsage.CpuCopy:
                notPreferredFlags |= VkMemoryPropertyFlags.DeviceLocal;
                break;
            case VmaMemoryUsage.GpuLazilyAllocated:
                requiredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
                break;
            case VmaMemoryUsage.Auto:
            case VmaMemoryUsage.AutoPreferDevice:
            case VmaMemoryUsage.AutoPreferHost:
                {
                    if (bufImgUsage == VmaBufferImageUsage.Unknown)
                    {
                        //Log.Error("VMA_MEMORY_USAGE_AUTO* values can only be used with functions like vmaCreateBuffer, vmaCreateImage so that the details of the created resource are known.");
                        return false;
                    }

                    // This relies on values of VK_IMAGE_USAGE_TRANSFER* being the same VK_BUFFER_IMAGE_TRANSFER*.
                    bool deviceAccess = bufImgUsage.ContainsDeviceAccess;
                    bool hostAccessSequentialWrite = (createInfo.Flags & VmaAllocationCreateFlags.HostAccessSequentialWrite) != 0;
                    bool hostAccessRandom = (createInfo.Flags & VmaAllocationCreateFlags.HostAccessRandom) != 0;
                    bool hostAccessAllowTransferInstead = (createInfo.Flags & VmaAllocationCreateFlags.VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT) != 0;
                    bool preferDevice = createInfo.Usage == VmaMemoryUsage.AutoPreferDevice;
                    bool preferHost = createInfo.Usage == VmaMemoryUsage.AutoPreferHost;

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

    private VkResult AllocateMemory(ref VmaAllocationCreateInfo createInfo,
        in VkMemoryRequirements vkMemReq, bool requiresDedicatedAllocation, bool prefersDedicatedAllocation,
        VkBuffer dedicatedBuffer, VkImage dedicatedImage, VmaBufferImageUsage dedicatedBufferImageUsage,
        VmaSuballocationType suballocType,
        out VmaAllocation? allocation)
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
            VmaBlockVector blockVector = createInfo.Pool.BlockVector;
            return AllocateMemoryOfType(
                createInfo.Pool,
                vkMemReq.size,
                vkMemReq.alignment,
                prefersDedicatedAllocation,
                dedicatedBuffer,
                dedicatedImage,
                dedicatedBufferImageUsage,
                ref createInfo,
                blockVector.MemoryTypeIndex,
                suballocType,
                createInfo.Pool.DedicatedAllocations,
                blockVector,
                out allocation);

            allocation = default;
            return VkResult.ErrorOutOfDeviceMemory;
        }
        else
        {
            //  or this allocation.
            uint memoryTypeBits = vkMemReq.memoryTypeBits;
            result = FindMemoryTypeIndex(memoryTypeBits, in createInfo, dedicatedBufferImageUsage, out int memoryTypeIndex);
            // Can't find any single memory type matching requirements. res is VK_ERROR_FEATURE_NOT_PRESENT.
            if (result != VkResult.Success)
                return result;

            do
            {
                VmaBlockVector blockVector = _blockVectors[memoryTypeIndex];
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

    private VkResult AllocateMemoryOfType(
        VmaPool? pool,
        ulong size,
        ulong alignment,
        bool dedicatedPreferred,
        VkBuffer dedicatedBuffer,
        VkImage dedicatedImage,
        in VmaBufferImageUsage dedicatedBufferImageUsage,
        ref VmaAllocationCreateInfo createInfo,
        int memTypeIndex,
        VmaSuballocationType suballocType,
        VmaDedicatedAllocationList dedicatedAllocations,
        VmaBlockVector blockVector,
        out VmaAllocation? allocation)
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

        if ((createInfo.Flags & VmaAllocationCreateFlags.DedicatedMemory) != 0)
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
            bool canAllocateDedicated = (createInfo.Flags & VmaAllocationCreateFlags.NeverAllocate) == 0
               && (pool == null || !blockVector.HasExplicitBlockSize);

            if (canAllocateDedicated)
            {
                // Heuristics: Allocate dedicated memory if requested size if greater than half of preferred block size.
                if (size > blockVector.PreferredBlockSize / 2)
                {
                    dedicatedPreferred = true;
                }

                // Protection against creating each allocation as dedicated when we reach or exceed heap size/budget,
                // which can quickly deplete maxMemoryAllocationCount: Don't prefer dedicated allocations when above
                // 3/4 of the maximum allocation count.
                if (_physicalDeviceProperties.limits.maxMemoryAllocationCount < uint.MaxValue / 4 &&
                    _deviceMemoryCount > _physicalDeviceProperties.limits.maxMemoryAllocationCount * 3 / 4)
                {
                    dedicatedPreferred = false;
                }

#if TODO
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

    private VkResult CalcAllocationParams(ref VmaAllocationCreateInfo createInfo, bool dedicatedRequired, bool dedicatedPreferred)
    {
        Debug.Assert((createInfo.Flags &
            (VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.HostAccessRandom)) !=
            (VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.HostAccessRandom),
            "Specifying both flags VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT and VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT is incorrect."
            );

#if DEBUG
        if (createInfo.Usage == VmaMemoryUsage.Auto
            || createInfo.Usage == VmaMemoryUsage.AutoPreferDevice
            || createInfo.Usage == VmaMemoryUsage.AutoPreferHost)
        {
            if ((createInfo.Flags & VmaAllocationCreateFlags.Mapped) != 0)
            {
                Debug.Assert(
                    (createInfo.Flags & (VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.HostAccessRandom)) != 0,
                    $"When using VmaAllocationCreateFlags.Mapped and usage = VmaMemoryUsage.Auto*, you must also specify VmaAllocationCreateFlags.HostAccessSequentialWrite or VmaAllocationCreateFlags.HostAccessRandom.");
            }
        }
#endif

        // If memory is lazily allocated, it should be always dedicated.
        if (dedicatedRequired ||
            createInfo.Usage == VmaMemoryUsage.GpuLazilyAllocated)
        {
            createInfo.Flags |= VmaAllocationCreateFlags.DedicatedMemory;
        }

        if (createInfo.Pool != null)
        {
            if (createInfo.Pool.BlockVector.HasExplicitBlockSize
                && (createInfo.Flags & VmaAllocationCreateFlags.DedicatedMemory) != 0)
            {
                Debug.Fail("Specifying VmaAllocationCreateFlags.DedicatedMemory while current custom pool doesn't support dedicated allocations.");
                return VK_ERROR_FEATURE_NOT_PRESENT;
            }
            createInfo.Priority = createInfo.Pool.BlockVector.Priority;
        }

        if ((createInfo.Flags & VmaAllocationCreateFlags.DedicatedMemory) != 0
            && (createInfo.Flags & VmaAllocationCreateFlags.NeverAllocate) != 0)
        {
            //Log.Error("Specifying VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT together with VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT makes no sense.");
            return VkResult.ErrorFeatureNotPresent;
        }

#if VMA_DEBUG_ALWAYS_DEDICATED_MEMORY
        if ((createInfo.Flags &  VmaAllocationCreateFlags.NeverAllocate) != 0)
        {
            createInfo.Flags |= VmaAllocationCreateFlags.DedicatedMemory;
        }
#endif

        // Non-auto USAGE values imply HOST_ACCESS flags.
        // And so does VMA_MEMORY_USAGE_UNKNOWN because it is used with custom pools.
        // Which specific flag is used doesn't matter. They change things only when used with VMA_MEMORY_USAGE_AUTO*.
        // Otherwise they just protect from assert on mapping.
        if (createInfo.Usage != VmaMemoryUsage.Auto
            && createInfo.Usage != VmaMemoryUsage.AutoPreferDevice
            && createInfo.Usage != VmaMemoryUsage.AutoPreferHost)
        {
            if ((createInfo.Flags & (VmaAllocationCreateFlags.HostAccessSequentialWrite | VmaAllocationCreateFlags.HostAccessRandom)) == 0)
            {
                createInfo.Flags |= VmaAllocationCreateFlags.HostAccessRandom;
            }
        }

        return VkResult.Success;
    }

    private void FreeMemory(VmaAllocation allocation)
    {
#if VMA_DEBUG_INITIALIZE_ALLOCATIONS
        FillAllocation(allocation, VMA_ALLOCATION_FILL_PATTERN_DESTROYED); 
#endif

        switch (allocation.Type)
        {
            case VmaAllocationType.Block:
                {
                    VmaBlockVector? pBlockVector = null;
                    VmaPool? pool = allocation.ParentPool;
                    if (pool is not null)
                    {
                        pBlockVector = pool.BlockVector;
                    }
                    else
                    {
                        int memTypeIndex = allocation.MemoryTypeIndex;
                        pBlockVector = _blockVectors[memTypeIndex];
                        Debug.Assert(pBlockVector != null, "Trying to free memory of unsupported type!");
                    }
                    pBlockVector.Free(allocation);
                }
                break;
            case VmaAllocationType.Dedicated:
                //FreeDedicatedMemory(allocation);
                break;
            default:
                Debug.Assert(false);
                break;
        }
    }

#if VMA_DEBUG_INITIALIZE_ALLOCATIONS
    private const byte VMA_ALLOCATION_FILL_PATTERN_CREATED = 0xDC;
    private const byte VMA_ALLOCATION_FILL_PATTERN_DESTROYED = 0xEF;

    [Conditional("VMA_DEBUG_INITIALIZE_ALLOCATIONS")]
    private void FillAllocation(VmaAllocation allocation, byte pattern)
    {
        if (allocation.IsMappingAllowed
            && (_memoryProperties.memoryTypes[allocation.MemoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
        {
            void* pData = null;
            VkResult res = Map(allocation, &pData);
            if (res == VK_SUCCESS)
            {
                memset(pData, (int)pattern, (nuint)allocation.Size);
                FlushOrInvalidateAllocation(allocation, 0, VK_WHOLE_SIZE, CacheOperation.Flush);
                Unmap(allocation);
            }
            else
            {
                Debug.Fail("VMA_DEBUG_INITIALIZE_ALLOCATIONS is enabled, but couldn't map memory to fill allocation.");
            }
        }
    } 
#endif

    private VkResult CalcMemTypeParams(ref VmaAllocationCreateInfo createInfo,
        int memTypeIndex,
        ulong size,
        int allocationCount)
    {
        // If memory type is not HOST_VISIBLE, disable MAPPED.
        if ((createInfo.Flags & VmaAllocationCreateFlags.Mapped) != 0 &&
            (_memoryProperties.memoryTypes[memTypeIndex].propertyFlags & VkMemoryPropertyFlags.HostVisible) == 0)
        {
            createInfo.Flags &= ~VmaAllocationCreateFlags.Mapped;
        }

        if ((createInfo.Flags & VmaAllocationCreateFlags.DedicatedMemory) != 0 &&
            (createInfo.Flags & VmaAllocationCreateFlags.WithinBudget) != 0)
        {
            int heapIndex = MemoryTypeIndexToHeapIndex(memTypeIndex);
            VmaBudget heapBudget = default;
            GetHeapBudgets(&heapBudget, heapIndex, 1);
            if (heapBudget.Usage + size * (ulong)allocationCount > heapBudget.Budget)
            {
                return VkResult.ErrorOutOfDeviceMemory;
            }

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

        if (!UseAmdDeviceCoherentMemory)
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
        return VmaAlignUp(isSmallHeap ? (heapSize / 8) : _preferredLargeHeapBlockSize, 32);
    }

    private enum CacheOperation
    {
        Flush,
        Invalidate
    };

    private VkResult FlushOrInvalidateAllocation(VmaAllocation allocation,
        ulong offset, ulong size,
        CacheOperation op)
    {
        if (GetFlushOrInvalidateRange(allocation, offset, size, out VkMappedMemoryRange memRange))
        {
            switch (op)
            {
                case CacheOperation.Flush:
                    return vkFlushMappedMemoryRanges(Device, 1, &memRange);
                case CacheOperation.Invalidate:
                    return vkInvalidateMappedMemoryRanges(Device, 1, &memRange);
                default:
                    Debug.Assert(false);
                    return VkResult.Success;
            }
        }

        return VkResult.Success;
    }

    private bool GetFlushOrInvalidateRange(VmaAllocation allocation, ulong offset, ulong size, out VkMappedMemoryRange range)
    {
        int memTypeIndex = allocation.MemoryTypeIndex;
        if (size > 0 && IsMemoryTypeNonCoherent(memTypeIndex))
        {
            ulong nonCoherentAtomSize = _physicalDeviceProperties.limits.nonCoherentAtomSize;
            ulong allocationSize = allocation.Size;
            Debug.Assert(offset <= allocationSize);

            range = new VkMappedMemoryRange();
            range.memory = allocation.GetMemory();

            switch (allocation.Type)
            {
                case VmaAllocationType.Dedicated:
                    range.offset = VmaAlignDown(offset, nonCoherentAtomSize);
                    if (size == VK_WHOLE_SIZE)
                    {
                        range.size = allocationSize - range.offset;
                    }
                    else
                    {
                        Debug.Assert(offset + size <= allocationSize);
                        range.size = Math.Min(
                            VmaAlignUp(size + (offset - range.offset), nonCoherentAtomSize),
                            allocationSize - range.offset);
                    }
                    break;
                case VmaAllocationType.Block:
                    {
                        // 1. Still within this allocation.
                        range.offset = VmaAlignDown(offset, nonCoherentAtomSize);
                        if (size == VK_WHOLE_SIZE)
                        {
                            size = allocationSize - offset;
                        }
                        else
                        {
                            Debug.Assert(offset + size <= allocationSize);
                        }
                        range.size = VmaAlignUp(size + (offset - range.offset), nonCoherentAtomSize);

                        // 2. Adjust to whole block.
                        ulong allocationOffset = allocation.Offset;
                        Debug.Assert(allocationOffset % nonCoherentAtomSize == 0);
                        ulong blockSize = allocation.GetBlock().MetaData.Size;
                        range.offset += allocationOffset;
                        range.size = Math.Min(range.size, blockSize - range.offset);

                        break;
                    }
                default:
                    Debug.Assert(false);
                    return false;
            }

            return true;
        }

        range = default;
        return false;
    }

    internal class CurrentBudgetData
    {
        public readonly uint[] BlockCount = new uint[VK_MAX_MEMORY_HEAPS];
        public readonly uint[] AllocationCount = new uint[VK_MAX_MEMORY_HEAPS];

        public readonly ulong[] BlockBytes = new ulong[VK_MAX_MEMORY_HEAPS];
        public readonly ulong[] AllocationBytes = new ulong[VK_MAX_MEMORY_HEAPS];

        public uint OperationsSinceBudgetFetch;
        public readonly ReaderWriterLockSlim BudgetMutex = new(LockRecursionPolicy.NoRecursion);
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
    public void Alloc(in AllocationRequest request, VmaSuballocationType type, object? userData);

    // Frees suballocation assigned to given memory region.
    public void Free(ulong allocHandle);

    public unsafe bool CreateAllocationRequest(ulong size, ulong alignment, bool upperAddress,
        VmaSuballocationType allocType,
        // Always one of VMA_ALLOCATION_CREATE_STRATEGY_* or VMA_ALLOCATION_INTERNAL_STRATEGY_* flags.
        uint strategy,
        AllocationRequest* allocationRequest);

    void AddStatistics(ref VmaStatistics stats);
    void AddDetailedStatistics(ref VmaDetailedStatistics stats);
}
