// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandBuffer.h"
#include "VulkanUtils.h"
#include <map>

namespace Alimer
{
    template <class T>
    using BindingMap = std::map<uint32_t, T>;

    struct VulkanResourceInfo
    {
        bool dirty = false;

        const Buffer* buffer = nullptr;
        VkDeviceSize offset = 0;
        VkDeviceSize range = 0;

        const TextureView* texture = nullptr;
        Sampler* sampler = nullptr;
    };

    class VulkanResourceSet
    {
    public:
        void Reset();
        bool IsDirty() const;

        void ClearDirty();
        void ClearDirty(uint32_t binding);
        bool SetBuffer(uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range);
        bool SetTexture(uint32_t binding, const TextureView* texture);
        bool SetSampler(uint32_t binding, _In_ Sampler* sampler);

        const BindingMap<VulkanResourceInfo>& GetResourceBindings() const
        {
            return bindings;
        }

    private:
        bool dirty = false;

        BindingMap<VulkanResourceInfo> bindings;
    };

    class VulkanResourceBindingState
    {
    public:
        void Reset();
        bool IsDirty();
        void ClearDirty();
        void ClearDirty(uint32_t set);

        void BindBuffer(const Buffer* buffer, uint64_t offset, uint64_t range, uint32_t set, uint32_t binding);
        void SetTexture(uint32_t set, uint32_t binding, const TextureView* view);
        void SetSampler(uint32_t set, uint32_t binding, _In_ Sampler* sampler);

        const VulkanResourceSet& GetSet(uint32_t index) const
        {
            return sets.find(index)->second;
        }

        const std::unordered_map<uint32_t, VulkanResourceSet>& GetSets() const
        {
            return sets;
        }

    private:
        bool dirty = false;
        std::unordered_map<uint32_t, VulkanResourceSet> sets;
    };

    class VulkanSwapChain;

    class VulkanCommandBuffer final : public CommandBuffer
    {
        friend class VulkanGraphics;
    public:
        VulkanCommandBuffer(VulkanGraphics& device, QueueType queueType, uint8_t index);
        ~VulkanCommandBuffer() override;

        void Reset(uint32_t frameIndex) override;
        void End();

        void PushDebugGroup(const std::string_view& name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const std::string_view& name) override;

        void UpdateBufferCore(const Buffer* buffer, const void* data, uint64_t offset, uint64_t size) override;
        void CopyBufferCore(const Buffer* source, uint64_t sourceOffset, const Buffer* destination, uint64_t destinationOffset, uint64_t size) override;

        void BeginRenderPassCore(_In_ SwapChain* swapChain, const Color& clearColor) override;
        void BeginRenderPassCore(const RenderPassInfo& info) override;
        void EndRenderPassCore() override;

        //void SetViewport(const Rect& rect) override;
        void SetViewport(const Viewport& viewport) override;
        void SetViewports(const Viewport* viewports, uint32_t count) override;

        void SetScissorRect(const Rect& rect) override;
        void SetScissorRects(const Rect* rects, uint32_t count) override;

        void SetStencilReference(uint32_t value) override;
        void SetBlendColor(const Color& color) override;
        void SetBlendColor(const float blendColor[4]) override;

        void SetVertexBuffersCore(uint32_t startSlot, uint32_t count, const Buffer* const* buffers, const uint64_t* offsets) override;
        void SetIndexBufferCore(const Buffer* buffer, IndexType indexType, uint64_t offset = 0) override;
        void BindBufferCore(uint32_t set, uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range) override;
        void SetTextureCore(uint32_t set, uint32_t binding, const TextureView* texture) override;

        void PushConstants(const void* data, uint32_t size) override;

        void SetPipeline(const Pipeline* pipeline) override;

        void DrawCore(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void DrawIndexedCore(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
        void DrawIndirectCore(_In_ Buffer* indirectBuffer, uint64_t indirectOffset) override;
        void DrawIndexedIndirectCore(_In_ Buffer* indirectBuffer, uint64_t indirectOffset) override;

        VkCommandBuffer GetHandle() const { return handle; }

    private:
        void Flush(VkPipelineBindPoint bindPoint);
        void FlushDescriptorState(VkPipelineBindPoint bindPoint);
        void FlushPushConstants();

        VulkanGraphics& device;
        QueueType queue;
        uint8_t index;

        bool debugUtilsSupported;

        VkCommandPool commandPools[kMaxFramesInFlight];
        VkCommandBuffer commandBuffers[kMaxFramesInFlight];
        VkCommandBuffer handle{ VK_NULL_HANDLE }; // Active command buffer

        std::array<VkClearValue, kMaxColorAttachments + 1> renderPassClearValues{};
        std::vector<VulkanSwapChain*> swapChains;
        std::set<VulkanTexture*> swapChainTextures;

        uint32_t colorAttachmentCount = 0;
        VkRenderPass boundRenderPass = VK_NULL_HANDLE;
        const VulkanPipeline* boundPipeline = nullptr;
        VulkanResourceBindingState bindingState;

        static constexpr uint32_t kUniformBufferCount = 15u;
        static constexpr uint32_t kSRVCount = 64u;
        static constexpr uint32_t kUAVCount = 8u;
        static constexpr uint32_t kSamplerCount = 16u;
        uint32_t poolSize = 256u;

        struct DescriptorBinder
        {
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            bool dirty = false;

        } descriptors[kMaxFramesInFlight];

        struct PushConstantData
        {
            uint8_t data[128];
            uint32_t size;
        } pushConstants = {};
    };

    constexpr VulkanCommandBuffer* ToVulkan(CommandBuffer* resource)
    {
        return static_cast<VulkanCommandBuffer*>(resource);
    }

    constexpr const VulkanCommandBuffer* ToVulkan(const CommandBuffer* resource)
    {
        return static_cast<const VulkanCommandBuffer*>(resource);
    }
}

