// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/RefCount.h"
#include "Core/Assert.h"

namespace Alimer
{
    RefCounted::RefCounted()
        : refCount(new RefCount())
    {
        // Hold a weak ref to self to avoid possible double delete of the refcount
        refCount->weakRefs++;
    }

    RefCounted::~RefCounted()
    {
        ALIMER_ASSERT(refCount != nullptr);
        ALIMER_ASSERT(refCount->refs == 0);
        ALIMER_ASSERT(refCount->weakRefs > 0);

        //Scripting::Callback(CALLBACK_ADD_DELETE, this);

        // Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
        refCount->refs = -1;

        uint32_t result = --refCount->weakRefs;
        if (result == 0)
        {
            delete refCount;
        }

        refCount = nullptr;
    }

    uint32_t RefCounted::AddRef()
    {
        uint32_t refs = ++refCount->refs;
        ALIMER_ASSERT(refs > 0);
        //Scripting::Callback(CALLBACK_ADD_REF, this);

        return refs;
    }

    uint32_t RefCounted::Release()
    {
        uint32_t refs = --refCount->refs;
        ALIMER_ASSERT(refs >= 0);
        if (refs == 0)
        {
            delete this;
        }

        return refs;
    }
}
