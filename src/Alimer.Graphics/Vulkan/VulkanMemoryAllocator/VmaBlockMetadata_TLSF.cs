// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

internal unsafe partial struct Pointer<T>(T* value)
    where T : unmanaged
{
    public T* Value = value;
}

internal sealed unsafe class VmaBlockMetadata_TLSF : IBlockMetadata
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
    private Block* _nullBlock;
    private readonly VmaBlockBufferImageGranularity _granularityHandler;

    public VmaBlockMetadata_TLSF(ulong size, ulong bufferImageGranularity, bool isVirtual)
    {
        Size = size;
        _granularityHandler = new(bufferImageGranularity);
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

        if (!IsVirtual)
            _granularityHandler.Init(size);

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
    public bool IsVirtual { get; }
    public bool IsEmpty => _nullBlock->offset == 0;

    public void Dispose()
    {
        DeleteArray(_freeList);
        //Helpers.DeleteArray(_freeList, _listsCount);
        _granularityHandler.Dispose();
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
        VmaBlockBufferImageGranularity.ValidationContext validateCtx = _granularityHandler.StartValidation(IsVirtual);

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
                    Debug.Assert(_granularityHandler.Validate(ref validateCtx, prev->offset, prev->size));
                }
            }

            if (prev->prevPhysical != null)
            {
                Debug.Assert(prev->prevPhysical->nextPhysical == prev);
            }
        }

        if (!IsVirtual)
        {
            Debug.Assert(_granularityHandler.FinishValidation(ref validateCtx));
        }

        Debug.Assert(nextOffset == 0);
        Debug.Assert(calculatedSize == Size);
        Debug.Assert(calculatedFreeSize == GetSumFreeSize());
        Debug.Assert(allocCount == _allocCount);
        Debug.Assert(freeCount == _blocksFreeCount);

        return true;
    }

    public void Alloc(in AllocationRequest request, VmaSuballocationType type, object? userData)
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

    public bool CreateAllocationRequest(ulong size, ulong alignment, bool upperAddress, VmaSuballocationType allocType, uint strategy, AllocationRequest* allocationRequest)
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
        if ((strategy & (uint)VmaAllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT) != 0)
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
        else if ((strategy & (uint)VmaAllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT) != 0)
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
        else if ((strategy & (uint)VmaAllocationCreateFlags.VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT) != 0)
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

    public void AddStatistics(ref VmaStatistics stats)
    {
        stats.BlockCount++;
        stats.AllocationCount += (uint)_allocCount;
        stats.BlockBytes += Size;
        stats.AllocationBytes += Size - GetSumFreeSize();
    }

    public void AddDetailedStatistics(ref VmaDetailedStatistics stats)
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

    private ulong GetDebugMargin() => IsVirtual ? (ulong)0 : VmaAllocator.DebugMargin;
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

    private bool CheckBlock(ref Block block, uint listIndex, ulong allocSize, ulong allocAlignment, VmaSuballocationType allocType, AllocationRequest* pAllocationRequest)
    {
        Debug.Assert(block.IsFree(), "Block is already taken!");

        ulong alignedOffset = VmaAlignUp(block.offset, allocAlignment);

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
