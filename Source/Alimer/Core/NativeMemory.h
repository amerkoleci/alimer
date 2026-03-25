// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    class ALIMER_API NativeMemory
    {
    public:
        static void* AlignedAlloc(size_t size, size_t alignment);
        static void AlignedFree(void* ptr);

        static void* Alloc(size_t size);
        static void* Alloc(size_t elementCount, size_t elementSize);
        static void* AllocZeroed(size_t size);
        static void* AllocZeroed(size_t elementCount, size_t elementSize);
        static void Free(void* ptr);
        static void* Realloc(void* ptr, size_t size);
        static void Clear(void* ptr, size_t size);

    private:
        static size_t GetByteCount(size_t elementCount, size_t elementSize);
    };
}
