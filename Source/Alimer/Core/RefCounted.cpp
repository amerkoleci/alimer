// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/RefCounted.h"
#if defined(ALIMER_CSHARP_BINDINGS)
#include "Alimer/CSharp/Bindings.h"
#endif

using namespace Alimer;

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

#if defined(ALIMER_CSHARP_BINDINGS)
    // TODO: Understand rbfx SetScriptObject
    Scripting::Callback(RefCounted_Delete, this);
#endif

    // Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
    refCount->refs.store(static_cast<uint32_t>(-1));

    if (refCount->weakRefs.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        delete refCount;
    }

    refCount = nullptr;
}

int32_t RefCounted::AddRef()
{
    // No barrier required.
    (void)refCount->refs.fetch_add(1, std::memory_order_relaxed);
    int32_t refs = refCount->refs.load(std::memory_order_relaxed);
    ALIMER_ASSERT(refs > 0);
#if defined(ALIMER_CSHARP_BINDINGS)
    Scripting::Callback(RefCounted_AddRef, this);
#endif
    return refs;
}

int32_t RefCounted::ReleaseRef()
{
    if (refCount->refs.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        delete this;
        return 0;
    }

    int32_t refs = refCount->refs.load(std::memory_order_relaxed);
    ALIMER_ASSERT(refs >= 0);
    return refs;
}

int32_t RefCounted::Refs() const
{
    return refCount->refs.load(std::memory_order_relaxed);
}

int32_t RefCounted::WeakRefs() const
{
    // Subtract one to not return the internally held reference
    return refCount->weakRefs.load(std::memory_order_relaxed) - 1;
}
