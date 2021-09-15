// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandBuffer.h"
#include "D3D12Utils.h"
#include <array>
#include <map>

namespace Alimer
{
	template <class T>
	using BindingMap = std::map<uint32_t, T>;

	struct D3D12ResourceInfo
	{
		bool dirty = false;

		const Buffer* buffer = nullptr;
		uint64_t offset = 0;
		uint64_t range = 0;

		const TextureView* texture= nullptr;
		const Sampler* sampler= nullptr;
	};

	class D3D12ResourceSet
	{
	public:
		void Reset();
		bool IsDirty() const;

		void ClearDirty();
		void ClearDirty(uint32_t binding);
		bool SetBuffer(const Buffer* buffer, uint64_t offset, uint64_t range, uint32_t binding);
		bool SetTexture(uint32_t binding, const TextureView* texture);

		const BindingMap<D3D12ResourceInfo>& GetResourceBindings() const
		{
			return bindings;
		}

	private:
		bool dirty = false;

		BindingMap<D3D12ResourceInfo> bindings;
	};

	class D3D12ResourceBindingState
	{
	public:
		void Reset();
		bool IsDirty();
		void ClearDirty();
		void ClearDirty(uint32_t set);

		void BindBuffer(const Buffer* buffer, uint64_t offset, uint64_t range, uint32_t set, uint32_t binding);
		void SetTexture(uint32_t set, uint32_t binding, const TextureView* texture);

		const std::unordered_map<uint32_t, D3D12ResourceSet>& GetSets() const
		{
			return sets;
		}

	private:
		bool dirty = false;
		std::unordered_map<uint32_t, D3D12ResourceSet> sets;
	};

	class D3D12CommandBuffer final : public CommandBuffer
	{
	public:
		D3D12CommandBuffer(D3D12CommandQueue& queue);
		~D3D12CommandBuffer() override;

		void Reset(uint32_t frameIndex) override;
		ID3D12GraphicsCommandList4* FinishAndReturn(uint64_t fenceValue);

		void PushDebugGroup(const std::string& name) override;
		void PopDebugGroup() override;
		void InsertDebugMarker(const std::string& name) override;

		void UpdateBufferCore(const Buffer* buffer, const void* data, uint64_t offset, uint64_t size) override;
		void CopyBufferCore(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size) override;

		void BeginRenderPassCore(const RenderPassInfo& info) override;
		void EndRenderPassCore() override;

		//void SetViewport(const Rect& rect) override;
		void SetViewport(const Viewport& viewport) override;
		void SetViewports(const Viewport* viewports, uint32_t count) override;

		//void SetScissorRect(const Rect& rect) override;
		//void SetScissorRects(const Rect* rects, uint32_t count) override;

		void SetStencilReference(uint32_t value) override;
		void SetBlendColor(const Color& color) override;
		void SetBlendColor(const float blendColor[4]) override;

		void SetVertexBuffersCore(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets) override;
		void SetIndexBufferCore(const Buffer* buffer, IndexType indexType, uint64_t offset = 0) override;
		void BindBufferCore(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range) override;
		void SetTextureCore(uint32_t set, uint32_t binding, const TextureView* texture) override;

		void PushConstants(const void* data, uint32_t size) override;

		void SetPipeline(const Pipeline* pipeline) override;

		void FlushDraw();
		void FlushDescriptorState(bool graphics);
		void FlushPushConstants();

		void DrawCore(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance) override;
		void DrawIndexedCore(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;

		void TransitionResource(const D3D12GpuResource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void BeginResourceTransition(const D3D12GpuResource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void InsertUAVBarrier(const D3D12GpuResource* resource, bool flushImmediate = false);
		void InsertAliasBarrier(const D3D12GpuResource* before, const D3D12GpuResource* after, bool flushImmediate = false);
		void FlushResourceBarriers();

		auto GetHandle() const noexcept { return handle.Get(); }

	private:
		D3D12Graphics& device;
		D3D12CommandQueue& queue;

		ID3D12CommandAllocator* _currentAllocator;
		ComPtr<ID3D12GraphicsCommandList4> handle;
		bool supportsRenderPass;

		bool typedUAVLoadAdditionalFormats{ false };
		bool standardSwizzle64KBSupported{ false };

		static constexpr uint32_t kMaxResourceBarriers = 16u;
		uint32_t numBarriersToFlush{ 0 };
		D3D12_RESOURCE_BARRIER resourceBarriers[kMaxResourceBarriers];

		D3D12_RENDER_PASS_RENDER_TARGET_DESC rtvDescs[kMaxColorAttachments] = {};
		D3D12_CPU_DESCRIPTOR_HANDLE colorRTVHandles[kMaxColorAttachments] = {};
		D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS subresourceParameters[kMaxColorAttachments] = {};

		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		std::array<D3D12_VERTEX_BUFFER_VIEW, kMaxVertexBufferBindings> vboViews = {};

		std::vector<D3D12Texture*> swapChainTextures;
		const D3D12Pipeline* boundPipeline = nullptr;
		D3D12ResourceBindingState bindingState;

		struct PushConstantData
		{
			uint8_t data[128];
			uint32_t size;
		} pushConstants = {};
	};

	constexpr D3D12CommandBuffer* ToD3D12(CommandBuffer* resource)
	{
		return static_cast<D3D12CommandBuffer*>(resource);
	}
}

