// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GPUResource.h"

namespace Alimer
{
    struct BufferCreateInfo
    {
        const char* label = nullptr;
        uint64_t size = 0;
        BufferUsage usage = BufferUsage::None;
        MemoryUsage memoryUsage = MemoryUsage::GpuOnly;
        //PixelFormat format = PixelFormat::Undefined;
        uintptr_t handle = 0;
    };

	class ALIMER_API Buffer : public GPUResource, public RefCounted
	{
	public:
        [[nodiscard]] static BufferRef Create(const BufferCreateInfo& createInfo, const void* initialData = nullptr);
		[[nodiscard]] static BufferRef Create(const void* data, BufferUsage usage, uint64_t size, const char* label = nullptr);

		BufferUsage GetUsage() const noexcept { return usage; }
        uint64_t GetSize() const noexcept { return size; }
        MemoryUsage GetMemoryUsage() const noexcept { return memoryUsage; }

        virtual uint8_t* Map() = 0;
        virtual void Unmap() = 0;

	protected:
		/// Constructor.
		Buffer(const BufferCreateInfo& info);

		BufferUsage usage;
        uint64_t size;
        MemoryUsage memoryUsage;
	};
}
