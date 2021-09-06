// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Buffer.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    Buffer::Buffer(const BufferCreateInfo& createInfo)
        : GraphicsResource(Type::Buffer)
        , usage(createInfo.usage)
        , size(createInfo.size)
    {

    }

    BufferRef Buffer::Create(const BufferCreateInfo& createInfo, const void* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());
        ALIMER_ASSERT(createInfo.size > 0);

        static constexpr uint64_t kMaxBufferSize = 128u * 1024u * 1024u;

        if (createInfo.size > kMaxBufferSize)
        {
            LOGE("Buffer size too large (size {})", createInfo.size);
            return nullptr;
        }

        return gGraphics().CreateBuffer(createInfo, initialData);
    }
}
