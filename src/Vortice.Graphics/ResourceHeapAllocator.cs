// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public abstract class ResourceHeapAllocator
    {
        public abstract IResourceHeap Allocate(ulong size);
        public abstract void Deallocate(IResourceHeap heap);
    }
}
