// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Color.h"
#include "Math/Viewport.h"
#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"

namespace Alimer
{
	struct GPUAllocation
	{
		/// The buffer associated with this memory.
		const Buffer* buffer = nullptr;

		/// Offset from start of buffer resource.
		uint64_t offset = 0;	
		/// Reserved size of this allocation.
		uint64_t size = 0;
		/// The CPU-writeable address.
		uint8_t* data = nullptr;			
	};


    struct RenderPassColorAttachment
    {
        TextureView* view = nullptr;
        TextureView* resolveView = nullptr;
        LoadAction loadAction = LoadAction::Discard;
        StoreAction storeAction = StoreAction::Store;
        Color clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct RenderPassDepthStencilAttachment
    {
        TextureView* view = nullptr;
        LoadAction depthLoadAction = LoadAction::Clear;
        StoreAction depthStoreAction = StoreAction::Discard;
        float clearDepth = 1.0;
        bool depthReadOnly = false;
        LoadAction stencilLoadAction = LoadAction::Discard;
        StoreAction stencilStoreAction = StoreAction::Discard;
        uint8_t clearStencil = 0;
        bool stencilReadOnly = false;
    };

    struct RenderPassInfo
    {
        RenderPassColorAttachment colorAttachments[kMaxColorAttachments] = {};
        RenderPassDepthStencilAttachment depthStencilAttachment;
    };

	class ALIMER_API CommandBuffer
	{
	public:
		/// Destructor.
		virtual ~CommandBuffer() = default;

		virtual void PushDebugGroup(const std::string_view& name) = 0;
		virtual void PopDebugGroup() = 0;
		virtual void InsertDebugMarker(const std::string_view& name) = 0;

		/// Update a buffer.
		void UpdateBuffer(const Buffer* buffer, const void* data, uint64_t offset = 0, uint64_t size = 0);
		void CopyBuffer(const Buffer* source, const Buffer* destination);
		void CopyBuffer(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size);

		virtual GPUAllocation Allocate(uint64_t size, uint64_t alignment);

        void BeginRenderPass(_In_ SwapChain* swapChain, const Color& clearColor);
		void BeginRenderPass(const RenderPassInfo& info);
		void EndRenderPass();

		void SetViewport(const Rect& rect);
        void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
		virtual void SetViewport(const Viewport& viewport) = 0;
		virtual void SetViewports(const Viewport* viewports, uint32_t count) = 0;

        void SetScissorRect(float width, float height);
        void SetScissorRect(float x, float y, float width, float height);
		virtual void SetScissorRect(const Rect& rect) = 0;
		virtual void SetScissorRects(const Rect* rects, uint32_t count) = 0;

		virtual void SetStencilReference(uint32_t value) = 0;
		virtual void SetBlendColor(const Color& color) = 0;
		virtual void SetBlendColor(const float blendColor[4]) = 0;

		void SetVertexBuffer(uint32_t slot, const Buffer* buffer, uint64_t offset = 0);
		void SetVertexBuffers(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets);
		void SetIndexBuffer(const Buffer* buffer, IndexType indexType, uint64_t offset = 0);

		/// Set dynamic vertex buffer data to the rendering pipeline.
		void SetDynamicVertexBuffer(uint32_t slot, uint32_t vertexCount, uint32_t vertexStride, const void* data);

		template<typename T>
		void SetDynamicVertexBuffer(uint32_t slot, _In_reads_(count) T const* data, uint32_t count)
		{
			SetDynamicVertexBuffer(slot, count, sizeof(T), data);
		}

		template<typename T>
		void SetDynamicVertexBuffer(uint32_t slot, const std::vector<T>& data)
		{
			SetDynamicVertexBuffer(slot, static_cast<uint32_t>(data.size()), sizeof(T), data.data());
		}

		/// Set dynamic index buffer data to the rendering pipeline.
		void SetDynamicIndexBuffer(uint32_t indexCount, IndexType indexType, const void* data);

		template<typename T>
		void SetDynamicIndexBuffer(_In_reads_(count) T const* data, uint32_t count)
		{
			static_assert(sizeof(T) == 2 || sizeof(T) == 4);

			const IndexType indexType = (sizeof(T) == 2) ? IndexType::UInt16 : IndexType::UInt32;
			SetDynamicIndexBuffer(count, indexType, data);
		}

		template<typename T>
		void SetDynamicIndexBuffer(const std::vector<T>& data)
		{
			static_assert(sizeof(T) == 2 || sizeof(T) == 4);

			const IndexType indexType = (sizeof(T) == 2) ? IndexType::UInt16 : IndexType::UInt32;
			SetDynamicIndexBuffer(static_cast<uint32_t>(data.size()), indexType, data.data());
		}

		void BindBuffer(uint32_t set, uint32_t binding, const Buffer* buffer);
		void BindBuffer(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range);
		void BindUniformBufferData(uint32_t set, uint32_t binding, uint32_t size, const void* data);
		void SetTexture(uint32_t set, uint32_t binding, const TextureView* texture);

		virtual void PushConstants(const void* data, uint32_t size) = 0;

		virtual void SetPipeline(const Pipeline* pipeline) = 0;
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t baseVertex = 0, uint32_t firstInstance = 0);
        void DrawIndirect(_In_ Buffer* indirectBuffer, uint64_t indirectOffset);
        void DrawIndexedIndirect(_In_ Buffer* indirectBuffer, uint64_t indirectOffset);

	private:
		virtual void UpdateBufferCore(const Buffer* buffer, const void* data, uint64_t offset, uint64_t size) = 0;
		virtual void CopyBufferCore(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size) = 0;

		virtual void BeginRenderPassCore(_In_ SwapChain* swapChain, const Color& clearColor) = 0;
        virtual void BeginRenderPassCore(const RenderPassInfo& info) = 0;
		virtual void EndRenderPassCore() = 0;

		virtual void SetVertexBuffersCore(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets) = 0;
		virtual void SetIndexBufferCore(const Buffer* buffer, IndexType indexType, uint64_t offset) = 0;
		virtual void BindBufferCore(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range) = 0;
		virtual void SetTextureCore(uint32_t set, uint32_t binding, const TextureView* texture) = 0;

		virtual void DrawCore(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
		virtual void DrawIndexedCore(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) = 0;
        virtual void DrawIndirectCore(_In_ Buffer* indirectBuffer, uint64_t indirectOffset) = 0;
        virtual void DrawIndexedIndirectCore(_In_ Buffer* indirectBuffer, uint64_t indirectOffset) = 0;

	protected:
		CommandBuffer() = default;

		bool insideRenderPass{ false };
		uint32_t frameIndex{ 0 };

		virtual void Reset(uint32_t frameIndex);

		struct ResourceFrameAllocator
		{
			uint32_t size = 1024 * 1024; // Start with 1MB
            BufferRef buffer;
			uint8_t* mappedData = nullptr;

			// Current offset, it increases on every allocation
			uint64_t currentOffset = 0;

			GPUAllocation Allocate(uint64_t size, uint64_t alignment);
			void Reset();

		} frameAllocators[kMaxFramesInFlight];
	};
}
