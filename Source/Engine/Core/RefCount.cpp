// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/RefCount.h"

namespace Alimer
{
    uint32_t RefCounted::AddRef()
    {
        return ++refCount;
    }

    uint32_t RefCounted::Release()
    {
        uint32_t result = --refCount;
        if (result == 0) {
            delete this;
        }
        return result;
    }
}
