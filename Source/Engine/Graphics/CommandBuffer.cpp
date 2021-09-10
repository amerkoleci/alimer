// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/CommandBuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"
using namespace rhi;

namespace Alimer
{
    GPUAllocation CommandBuffer::AllocateGPU(uint64_t size, uint64_t alignment)
    {
        ALIMER_ASSERT(size > 0 && "Allocation size must be greater than zero");

        // Align the allocation
        const uint64_t alignedSize = AlignTo(size, alignment);

        const u32 frameIndex = gGraphics().GetFrameIndex();
        const u64 frameCount = gGraphics().GetFrameCount();

        FrameAllocator& allocator = frameAllocators[frameIndex];
        if (frameCount != allocator.frameIndex)
        {
            allocator.frameIndex = frameCount;
            allocator.currentOffset = 0;
        }

        const uint64_t freeSpace = allocator.buffer == nullptr ? 0 : allocator.buffer->GetSize() - allocator.currentOffset;
        if (alignedSize > freeSpace)
        {
            // Resize or create
            //if (buffer.IsNotNull())
            //{
            //    size *= 2u;
            //}

            BufferDesc bufferDesc{};
            bufferDesc.label = "FrameAllocator - Buffer";
            bufferDesc.usage = BufferUsage::Vertex | BufferUsage::Index | BufferUsage::Uniform | BufferUsage::ShaderRead;
            bufferDesc.size = alignedSize;
            bufferDesc.heapType = HeapType::Upload;
            //allocator.buffer = Buffer::Create(bufferDesc);
            allocator.currentOffset = 0;
        }

        GPUAllocation allocation;
        allocation.buffer = allocator.buffer;
        allocation.offset = allocator.currentOffset;
        allocation.size = alignedSize;
        allocation.data = (uint8_t*)((size_t)allocator.buffer->MappedData() + allocator.currentOffset);

        allocator.currentOffset += alignedSize;

        ALIMER_ASSERT(allocation.data != nullptr && allocation.buffer != nullptr);
        return allocation;
    }

    void CommandBuffer::BindConstantBuffer(uint32_t binding, const IBuffer* buffer)
    {
        ALIMER_ASSERT(binding < kMaxUniformBufferBindings);

        BindConstantBufferCore(binding, buffer, 0, buffer->GetSize());
    }

    void CommandBuffer::BindConstantBuffer(uint32_t binding, const IBuffer* buffer, uint64_t offset, uint64_t range)
    {
        ALIMER_ASSERT(binding < kMaxUniformBufferBindings);

        BindConstantBufferCore(binding, buffer, offset, range);
    }

    void CommandBuffer::BindConstantBufferData(uint32_t binding, uint32_t size, const void* data)
    {
        ALIMER_ASSERT(binding < kMaxUniformBufferBindings);

        GPUAllocation allocation = AllocateGPU(size, 256);
        memcpy(allocation.data, data, size);

        BindConstantBufferCore(binding, allocation.buffer, allocation.offset, allocation.size);
    }
}
