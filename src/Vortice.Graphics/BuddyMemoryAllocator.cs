// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    public sealed class BuddyMemoryAllocator
    {
        private readonly ulong _memoryBlockSize = 0;

        public BuddyMemoryAllocator(ulong maxSystemSize, ulong memoryBlockSize, ResourceHeapAllocator heapAllocator)
        {
            _memoryBlockSize = memoryBlockSize;
        }
    }
}
