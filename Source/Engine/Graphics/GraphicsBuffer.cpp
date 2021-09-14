// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/GraphicsBuffer.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    GraphicsBuffer::GraphicsBuffer()
        : GraphicsResource(Type::Buffer)
    {
    }

    GraphicsBuffer::~GraphicsBuffer()
    {
        handle = {};
    }

    bool GraphicsBuffer::Create(const BufferDesc& desc, const void* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        static constexpr uint64_t c_maxBytes = 128 * 1024u * 1024u;
        static_assert(c_maxBytes <= UINT32_MAX, "Exceeded integer limits");

        if (desc.size > c_maxBytes)
        {
            LOGE("Buffer size too large (size {})", desc.size);
            return false;
        }

        handle = {};

        return gGraphics().CreateBuffer(&desc, initialData, &handle);
    }
}
