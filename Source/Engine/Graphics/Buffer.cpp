// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Buffer.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    Buffer::Buffer(const BufferDesc& desc)
        : GraphicsResource(Type::Buffer)
        , usage(desc.usage)
        , size(desc.size)
    {

    }

    BufferRef Buffer::Create(const BufferDesc& desc, const void* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());
        ALIMER_ASSERT(desc.size > 0);

        static constexpr uint64_t kMaxBufferSize = 128u * 1024u * 1024u;

        if (desc.size > kMaxBufferSize)
        {
            LOGE("Buffer size too large (size {})", desc.size);
            return nullptr;
        }

        return gGraphics().CreateBuffer(desc, initialData);
    }


    BufferRef Buffer::Create(const void* data, uint64_t size, BufferUsage usage, const char* label)
    {
        BufferDesc desc;
        desc.label = label;
        desc.size = size;
        desc.usage = usage;

        return Create(desc, data);
    }
}
