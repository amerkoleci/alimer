// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandBuffer.h"
#include "VulkanUtils.h"

namespace Alimer
{
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
        void BindConstantBufferCore(uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range) override;
        void BindTextureCore(uint32_t binding, const TextureView* textureView) override;
        void BindSamplerCore(uint32_t binding, const Sampler* sampler) override;

        void PushConstants(const void* data, uint32_t size) override;

        void BindPipeline(const Pipeline* pipeline) override;

        void DrawCore(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void DrawIndexedCore(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
        void DrawIndirectCore(_In_ Buffer* indirectBuffer, uint64_t indirectOffset) override;
        void DrawIndexedIndirectCore(_In_ Buffer* indirectBuffer, uint64_t indirectOffset) override;

        VkCommandBuffer GetHandle() const { return handle; }

    private:
        void Flush(VkPipelineBindPoint bindPoint);
        void FlushDescriptorState(VkPipelineBindPoint bindPoint);

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

        struct DescriptorBinderPool
        {
            VulkanGraphics* device;
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            uint32_t poolSize = 256;

            void Init(VulkanGraphics* device);
            void Destroy();
            void Reset();
        } binderPools[kMaxFramesInFlight];

        static constexpr uint32_t kUAVCount = 16u;

        struct DescriptorBindingTable
        {
            VkDescriptorBufferInfo buffers[kMaxConstantBufferBindings] = {};

            const VulkanTextureView* textureSRVViews[kMaxSRVBindings] = {};
            const VulkanTextureView* textureUAVViews[kUAVCount] = {};
            //int UAV_index[DESCRIPTORBINDER_UAV_COUNT] = {};
            const VulkanSampler* samplers[kMaxSamplerBindings] = {};
        };

        struct DescriptorBinder
        {
            VulkanGraphics* device = nullptr;
            DescriptorBindingTable table;

            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            uint32_t poolSize = 256u;

            std::vector<VkWriteDescriptorSet> descriptorWrites;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkBufferView> texelBufferViews;
            std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelerationStructureViews;
            bool dirty = false;

            void Init(VulkanGraphics* device);
            void Reset();
        } binder;
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

