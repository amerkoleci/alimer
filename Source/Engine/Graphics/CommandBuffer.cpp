// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
	void CommandBuffer::Reset(uint32_t frameIndex_)
	{
		frameIndex = frameIndex_;
		frameAllocators[frameIndex_].Reset();
	}

	GPUAllocation CommandBuffer::Allocate(uint64_t size, uint64_t alignment)
	{
		return frameAllocators[frameIndex].Allocate(size, alignment);
	}

	void CommandBuffer::UpdateBuffer(const Buffer* buffer, const void* data, uint64_t offset, uint64_t size)
	{
		ALIMER_ASSERT(!insideRenderPass);
		ALIMER_ASSERT(buffer != nullptr);

		if (size == 0)
		{
			size = buffer->GetSize() - offset;
		}

		if (offset >= size)
		{
			LOGW("UpdateBuffer - offset is larger than the buffer size.");
			return;
		}

		if (offset + size > size)
		{
            LOGW("UpdateBuffer - offset + size bigger that buffer size. Clamping size");
			size -= offset;
		}

		UpdateBufferCore(buffer, data, offset, size);
	}

	void CommandBuffer::CopyBuffer(const Buffer* source, const Buffer* destination)
	{
		ALIMER_ASSERT(!insideRenderPass);
		ALIMER_ASSERT(source != nullptr);
		ALIMER_ASSERT(destination != nullptr);

		CopyBufferCore(source, 0, destination, 0, source->GetSize());
	}

	void CommandBuffer::CopyBuffer(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size)
	{
		ALIMER_ASSERT(!insideRenderPass);
		ALIMER_ASSERT(source != nullptr);
		ALIMER_ASSERT(destination != nullptr);
		ALIMER_ASSERT(sourceOffset + size <= source->GetSize());
		ALIMER_ASSERT(destinationOffset + size <= destination->GetSize());

		CopyBufferCore(source, sourceOffset, destination, destinationOffset, size);
	}

	void CommandBuffer::BeginRenderPass(const RenderPassInfo& info)
	{
		//ALIMER_ASSERT_MSG(!insideRenderPass, "Cannot begin render pass while inside render pass");

		BeginRenderPassCore(info);
		insideRenderPass = true;
	}

	void CommandBuffer::EndRenderPass()
	{
		//ALIMER_ASSERT_MSG(insideRenderPass, "Cannot end render pass without begin first");

		EndRenderPassCore();
		insideRenderPass = false;
	}

	void CommandBuffer::SetVertexBuffer(uint32_t slot, const Buffer* buffer, uint64_t offset)
	{
		ALIMER_ASSERT(slot < kMaxVertexBufferBindings);

		SetVertexBuffersCore(slot, 1, &buffer, &offset);
	}

	void CommandBuffer::SetVertexBuffers(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets)
	{
		ALIMER_ASSERT(startSlot < kMaxVertexBufferBindings);
		ALIMER_ASSERT((startSlot + count) < kMaxVertexBufferBindings);

		SetVertexBuffersCore(startSlot, count, buffers, offsets);
	}

	void CommandBuffer::SetIndexBuffer(const Buffer* buffer, IndexType indexType, uint64_t offset)
	{
		ALIMER_ASSERT(buffer != nullptr);
		//ALIMER_ASSERT_MSG(any(buffer->GetUsage() & BufferUsage::Index), "Buffer created without Index usage");

		SetIndexBufferCore(buffer, indexType, offset);
	}

	void CommandBuffer::SetDynamicVertexBuffer(uint32_t slot, uint32_t vertexCount, uint32_t vertexStride, const void* data)
	{
		uint64_t bufferSize = (uint64_t)(vertexCount * vertexStride);

		GPUAllocation allocation = Allocate(bufferSize, 4);
		memcpy(allocation.data, data, bufferSize);

		SetVertexBuffer(slot, allocation.buffer, allocation.offset);
	}

	void CommandBuffer::SetDynamicIndexBuffer(uint32_t indexCount, IndexType indexType, const void* data)
	{
		uint64_t indexSizeInBytes = indexType == IndexType::UInt16 ? 2 : 4;
		uint64_t bufferSize = indexCount * indexSizeInBytes;
		
		GPUAllocation allocation = Allocate(bufferSize, 4);
		memcpy(allocation.data, data, bufferSize);
		SetIndexBufferCore(allocation.buffer, indexType, allocation.offset);
	}

	void CommandBuffer::BindBuffer(uint32_t set, uint32_t binding, const Buffer* buffer)
	{
		BindBuffer(set, binding, buffer, 0, buffer->GetSize());
	}

	void CommandBuffer::BindBuffer(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range)
	{
		//ALIMER_ASSERT(set < kMaxDescriptorSets);
		//ALIMER_ASSERT(binding < kMaxDescriptorBindings);

		BindBufferCore(set, binding, buffer, offset, range);
	}

	void CommandBuffer::BindUniformBufferData(uint32_t set, uint32_t binding, uint32_t size, const void* data)
	{
		//ALIMER_ASSERT(set < kMaxDescriptorSets);
		//ALIMER_ASSERT(binding < kMaxDescriptorBindings);

		auto allocation = Allocate(size, 256);
		memcpy(allocation.data, data, size);

		BindBufferCore(set, binding, allocation.buffer, allocation.offset, allocation.size);
	}

	void CommandBuffer::SetTexture(uint32_t set, uint32_t binding, const TextureView* texture)
	{
		//ALIMER_ASSERT(set < kMaxDescriptorSets);
		//ALIMER_ASSERT(binding < kMaxDescriptorBindings);
		ALIMER_ASSERT(texture != nullptr);

		SetTextureCore(set, binding, texture);
	}

	void CommandBuffer::Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance)
	{
		//ALIMER_ASSERT_MSG(insideRenderPass, "Cannot Draw outside render pass");

		DrawCore(vertexStart, vertexCount, instanceCount, baseInstance);
	}

	void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
	{
		//ALIMER_ASSERT_MSG(insideRenderPass, "Cannot Draw outside render pass");

		DrawIndexedCore(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
	}

	GPUAllocation CommandBuffer::ResourceFrameAllocator::Allocate(uint64_t size_, uint64_t alignment)
	{
		ALIMER_ASSERT(size_ > 0 && "Allocation size must be greater than zero");

		// Align the allocation
		const uint64_t alignedSize = AlignUp(size_, alignment);

		//if (buffer.IsNull() || (alignedSize > size))
		//{
		//	// Resize or create
		//	if (buffer.IsNotNull())
		//	{
		//		size *= 2u;
		//	}
        //
        //    BufferCreateInfo bufferInfo{};
        //    bufferInfo.label = "ResourceFrameAllocator - Buffer";
        //    bufferInfo.memoryUsage = MemoryUsage::CpuToGpu;
        //    bufferInfo.usage = BufferUsage::Vertex | BufferUsage::Index | BufferUsage::Uniform;
        //    bufferInfo.size = size;
		//	buffer = Buffer::Create(bufferInfo);
        //
		//	mappedData = buffer->Map();
		//}

		currentOffset = AlignUp(currentOffset, alignment);

		GPUAllocation allocation;
		allocation.buffer = buffer;
		allocation.offset = currentOffset;
		allocation.size = alignedSize;
		allocation.data = mappedData + currentOffset;

		currentOffset += alignedSize;

		return allocation;
	}

	void CommandBuffer::ResourceFrameAllocator::Reset()
	{
		currentOffset = 0;
	}
}
