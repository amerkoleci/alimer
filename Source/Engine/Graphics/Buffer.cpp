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

    BufferRef Buffer::Create(uint64_t size, BufferUsage usage, const void* data, const char* label)
	{
        ALIMER_ASSERT(data != nullptr);
		ALIMER_ASSERT(gGraphics().IsInitialized());

        BufferCreateInfo info;
        info.label = label;
        info.size = size;
        info.usage = usage;
		return gGraphics().CreateBuffer(&info, data);
	}

    BufferRef Buffer::CreateUpload(uint64_t size, const char* label)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        BufferCreateInfo info;
        info.label = label;
        info.cpuAccess = CpuAccessMode::Write;
        info.size = size;
        return Create(&info, nullptr);
    }

    BufferRef Buffer::CreateReadback(uint64_t size, const char* label)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        BufferCreateInfo info;
        info.label = label;
        info.cpuAccess = CpuAccessMode::Read;
        info.size = size;
        return Create(&info, nullptr);
    }
}
