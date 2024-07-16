// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

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
        if (!_itemBlocks.empty())
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
