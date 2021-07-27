// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using TerraFX.Interop;
using static Vortice.MathUtilities;

namespace Vortice.Graphics
{
    public sealed class BuddyAllocator
    {
        private const ulong kInvalidOffset = ulong.MaxValue;

        private readonly ulong _maxBlockSize;
        private readonly List<BuddyBlock> _freeLists;
        private readonly BuddyBlock _root;

        public BuddyAllocator(ulong maxSize)
        {
            Guard.Assert(IsPow2(maxSize));

            _maxBlockSize = maxSize;

            _freeLists = new List<BuddyBlock>((int)Math.Log2(_maxBlockSize) + 1);
            // Insert the level0 free block.
            _root = new BuddyBlock(maxSize, 0);
            _freeLists[0] = _root;
        }

#if TODO
        public ulong Allocate(ulong allocationSize, ulong alignment)
        {
            if (allocationSize == 0 || allocationSize > _maxBlockSize)
            {
                return kInvalidOffset;
            }

            // Compute the level
            uint allocationSizeToLevel = ComputeLevelFromBlockSize(allocationSize);

            Guard.Assert(allocationSizeToLevel < _freeLists.Count);

            ulong currBlockLevel = GetNextFreeAlignedBlock(allocationSizeToLevel, alignment);
            // Error when no free blocks exist (allocator is full)
            if (currBlockLevel == kInvalidOffset)
            {
                return kInvalidOffset;
            }

            BuddyBlock currBlock = _freeLists[(int)currBlockLevel];
            for (; currBlockLevel < allocationSizeToLevel; currBlockLevel++)
            {
                //ASSERT(currBlock->mState == BlockState::Free);

                // Remove curr block (about to be split).
                //RemoveFreeBlock(currBlock, currBlockLevel);

                // Create two free child blocks (the buddies).
                ulong nextLevelSize = currBlock.Size / 2;
                BuddyBlock leftChildBlock = new BuddyBlock(nextLevelSize, currBlock.Offset);
                BuddyBlock rightChildBlock = new BuddyBlock(nextLevelSize, currBlock.Offset + nextLevelSize);

                // Remember the parent to merge these back upon de-allocation.
                rightChildBlock->pParent = currBlock;
                leftChildBlock->pParent = currBlock;
                // Make them buddies.
                leftChildBlock->pBuddy = rightChildBlock;
                rightChildBlock->pBuddy = leftChildBlock;
            }
        } 
#endif

        private uint ComputeLevelFromBlockSize(ulong blockSize)
        {
            // Every level in the buddy system can be indexed by order-n where n = log2(blockSize).
            // However, mFreeList zero-indexed by level.
            // For example, blockSize=4 is Level1 if MAX_BLOCK is 8.
            return (uint)Math.Log2(_maxBlockSize) - (uint)Math.Log2(blockSize);
        }

        private ulong GetNextFreeAlignedBlock(nuint allocationBlockLevel, ulong alignment)
        {
            for (nuint ii = 0; ii <= allocationBlockLevel; ++ii)
            {
                nuint currLevel = allocationBlockLevel - ii;
                BuddyBlock freeBlock = _freeLists[(int)currLevel];
                if ((freeBlock.Offset % alignment == 0))
                {
                    return currLevel;
                }
            }
            return kInvalidOffset;  // No free block exists at any level.
        }

        readonly struct BuddyBlock
        {
            public readonly ulong Offset;
            public readonly ulong Size;

            // Pointer to this block's buddy, iff parent is split.
            // Used to quickly merge buddy blocks upon de-allocate.
            //BuddyBlock buddy;
            //BuddyBlock parent = nullptr;
            // Track whether this block has been split or not.
            //BlockState mState;

            public BuddyBlock(ulong size, ulong offset)
            {
                Offset = offset;
                Size = size;

                //free.pPrev = nullptr;
                //free.pNext = nullptr;
            }
        }
    }
}
