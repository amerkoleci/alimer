// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Buffer.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
	Buffer::Buffer(const BufferCreateInfo* info)
		: GPUResource(Type::Buffer)
		, size(info->size)
		, usage(info->usage)
	{

	}

    BufferRef Buffer::Create(const BufferCreateInfo* createInfo, const void* initialData)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        ALIMER_ASSERT(createInfo->size > 0);

        static constexpr uint64_t kMaxBufferSize = 128u * 1024u * 1024u;

        if (createInfo->size > kMaxBufferSize)
        {
            LOGE("Buffer size too large (size {})", createInfo->size);
            return nullptr;
        }

        return gGraphics().CreateBuffer(createInfo, initialData);
    }

    BufferRef Buffer::Create(const void* data, BufferUsage usage, uint64_t size, const char* label)
	{
        ALIMER_ASSERT(data != nullptr);
		ALIMER_ASSERT(gGraphics().IsInitialized());

        BufferCreateInfo info;
        info.label = label;
        info.usage = usage;
        info.size = size;
		return gGraphics().CreateBuffer(&info, data);
	}

    BufferRef Buffer::CreateUpload(uint64_t size, const char* label)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        BufferCreateInfo info;
        info.label = label;
        info.heapType = HeapType::Upload;
        info.size = size;
        return Create(&info, nullptr);
    }

    BufferRef Buffer::CreateReadback(uint64_t size, const char* label)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        BufferCreateInfo info;
        info.label = label;
        info.heapType = HeapType::Readback;
        info.size = size;
        return Create(&info, nullptr);
    }
}
