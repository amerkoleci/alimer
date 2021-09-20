// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineLayout.h"
#include <queue>

namespace Alimer
{
    struct VulkanUploadContext
    {
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        uint64_t target = 0;
        BufferRef uploadBuffer;
        uint8_t* data = nullptr;
    };

	class VulkanGraphics final : public Graphics
	{
        friend class VulkanCommandBuffer;
        friend class VulkanTexture;

	public:
        VkPhysicalDeviceProperties2 properties2 = {};
        VkPhysicalDeviceVulkan11Properties properties_1_1 = {};
        VkPhysicalDeviceVulkan12Properties properties_1_2 = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_structure_properties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_properties = {};
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_properties = {};

        VkPhysicalDeviceFeatures2 features2 = {};
        VkPhysicalDeviceVulkan11Features features_1_1 = {};
        VkPhysicalDeviceVulkan12Features features_1_2 = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {};
        VkPhysicalDeviceRayQueryFeaturesKHR raytracing_query_features = {};
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features = {};
        VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = {};

        std::vector<VkDynamicState> pso_dynamicStates;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

		// Null objects for descriptor sets
        VkBuffer		nullBuffer = VK_NULL_HANDLE;
        VmaAllocation	nullBufferAllocation = VK_NULL_HANDLE;
        VkBufferView	nullBufferView = VK_NULL_HANDLE;
		VkImage			nullImage1D = VK_NULL_HANDLE;
		VkImage			nullImage2D = VK_NULL_HANDLE;
		VkImage			nullImage3D = VK_NULL_HANDLE;
		VmaAllocation	nullImageAllocation1D = VK_NULL_HANDLE;
		VmaAllocation	nullImageAllocation2D = VK_NULL_HANDLE;
		VmaAllocation	nullImageAllocation3D = VK_NULL_HANDLE;
		VkImageView		nullImageView1D = VK_NULL_HANDLE;
		VkImageView		nullImageView1DArray = VK_NULL_HANDLE;
		VkImageView		nullImageView2D = VK_NULL_HANDLE;
		VkImageView		nullImageView2DArray = VK_NULL_HANDLE;
		VkImageView		nullImageViewCube = VK_NULL_HANDLE;
		VkImageView		nullImageViewCubeArray = VK_NULL_HANDLE;
		VkImageView		nullImageView3D = VK_NULL_HANDLE;
		SamplerRef		nullSampler;

	public:
		VulkanGraphics(ValidationMode validationMode);
		~VulkanGraphics() override;

		void WaitIdle() override;
        bool BeginFrame() override;
		void EndFrame() override;
        void SubmitCommandBuffers();

        CommandBuffer* BeginCommandBuffer(QueueType queueType = QueueType::Graphics) override;

		void SetObjectName(VkObjectType type, uint64_t handle, const std::string_view& name);

        VulkanUploadContext UploadBegin(uint64_t size);
        void UploadEnd(VulkanUploadContext context);

		VkRenderPass GetVkRenderPass(const VulkanRenderPassKey& key);
		VkFramebuffer GetVkFramebuffer(uint64_t hash, const VulkanFboKey& key);

		VulkanDescriptorSetLayout& RequestDescriptorSetLayout(const uint32_t setIndex, const std::vector<ShaderResource>& resources);
		VulkanPipelineLayout& RequestPipelineLayout(const std::vector<VulkanShader*>& shaders);

		void DeferDestroy(VkImage texture, VmaAllocation allocation);
		void DeferDestroy(VkBuffer buffer, VmaAllocation allocation);
		void DeferDestroy(VkImageView view);
		void DeferDestroy(VkSampler resource, uint32_t bindlessIndex);
		void DeferDestroy(VkShaderModule resource);
		void DeferDestroy(VkPipeline resource);
		void DeferDestroy(VkDescriptorPool resource);

		bool DebugUtilsSupported() const noexcept { return debugUtils; }
		VkInstance GetInstance() const noexcept { return instance; }
		VkPhysicalDevice GetPhysicalDevice() const noexcept { return physicalDevice; }
		VmaAllocator GetAllocator() const { return allocator; }
		VkDevice GetHandle() const { return device; }

		uint32_t GetGraphicsQueueFamily() const noexcept { return graphicsQueueFamily; }
		uint32_t GetComputeQueueFamily() const noexcept { return computeQueueFamily; }
		uint32_t GetCopyQueueFamily() const noexcept { return copyQueueFamily; }

		uint32_t AllocateSRV();
        VkDescriptorSetLayout GetBindlessSampledImageDescriptorSetLayout() const;
        VkDescriptorSet GetBindlessSampledImageDescriptorSet() const;

        uint32_t AllocateUAV();

	private:
		void ProcessDeletionQueue();

        TextureRef CreateTextureCore(const TextureCreateInfo& info, const TextureData* initialData) override;
		BufferRef CreateBuffer(const BufferCreateInfo* info, const void* initialData) override;
        ShaderRef CreateShader(ShaderStages stage, const void* byteCode, size_t byteCodeLength, const std::string& entryPoint) override;
		SamplerRef CreateSampler(const SamplerDesc& desc) override;
		PipelineRef CreateRenderPipeline(const RenderPipelineStateCreateInfo* info) override;
		PipelineRef CreateComputePipeline(const ComputePipelineCreateInfo* info) override;
        SwapChainRef CreateSwapChain(void* windowHandle, const SwapChainCreateInfo& info) override;

		void* GetNativeHandle() const noexcept { return device; }

		bool debugUtils = false;
		VkInstance instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		uint32_t graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
		uint32_t computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
		uint32_t copyQueueFamily = VK_QUEUE_FAMILY_IGNORED;

		VkDevice device = VK_NULL_HANDLE;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;
        VkQueue copyQueue = VK_NULL_HANDLE;

		VmaAllocator allocator = VK_NULL_HANDLE;

        struct CommandQueue
        {
            VkQueue queue = VK_NULL_HANDLE;
            VkSemaphore semaphore = VK_NULL_HANDLE;
            std::vector<VulkanSwapChain*> submit_swapchains;
            std::vector<VkSwapchainKHR> submitVkSwapchains;
            std::vector<uint32_t> submit_swapChainImageIndices;
            std::vector<VkResult> submit_swapChainResults;
            std::vector<VkPipelineStageFlags> submit_waitStages;
            std::vector<VkSemaphore> submit_waitSemaphores;
            std::vector<uint64_t> submit_waitValues;
            std::vector<VkSemaphore> submit_signalSemaphores;
            std::vector<uint64_t> submit_signalValues;
            std::vector<VkCommandBuffer> submit_cmds;

            void Submit(VkFence fence)
            {
                VkTimelineSemaphoreSubmitInfo timelineInfo { VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
                timelineInfo.waitSemaphoreValueCount = (uint32_t)submit_waitValues.size();
                timelineInfo.pWaitSemaphoreValues = submit_waitValues.data();
                timelineInfo.signalSemaphoreValueCount = (uint32_t)submit_signalValues.size();
                timelineInfo.pSignalSemaphoreValues = submit_signalValues.data();

                VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
                submitInfo.pNext = &timelineInfo;
                submitInfo.waitSemaphoreCount = (uint32_t)submit_waitSemaphores.size();
                submitInfo.pWaitSemaphores = submit_waitSemaphores.data();
                submitInfo.pWaitDstStageMask = submit_waitStages.data();
                submitInfo.commandBufferCount = (uint32_t)submit_cmds.size();
                submitInfo.pCommandBuffers = submit_cmds.data();
                submitInfo.signalSemaphoreCount = (uint32_t)submit_signalSemaphores.size();
                submitInfo.pSignalSemaphores = submit_signalSemaphores.data();

                VkResult res = vkQueueSubmit(queue, 1, &submitInfo, fence);
                assert(res == VK_SUCCESS);

                if (!submit_swapchains.empty())
                {
                    VkPresentInfoKHR presentInfo = {};
                    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                    presentInfo.waitSemaphoreCount = (uint32_t)submit_signalSemaphores.size();
                    presentInfo.pWaitSemaphores = submit_signalSemaphores.data();
                    presentInfo.swapchainCount = (uint32_t)submit_swapchains.size();
                    presentInfo.pSwapchains = submitVkSwapchains.data();
                    presentInfo.pImageIndices = submit_swapChainImageIndices.data();
                    presentInfo.pResults = submit_swapChainResults.data();
                    vkQueuePresentKHR(queue, &presentInfo);

                    for (size_t i = 0; i < submit_swapchains.size(); ++i)
                    {
                        submit_swapchains[i]->AfterPresent(submit_swapChainResults[i]);
                    }
                }

                submit_swapchains.clear();
                submitVkSwapchains.clear();
                submit_swapChainImageIndices.clear();
                submit_swapChainResults.clear();
                submit_waitStages.clear();
                submit_waitSemaphores.clear();
                submit_waitValues.clear();
                submit_signalSemaphores.clear();
                submit_signalValues.clear();
                submit_cmds.clear();
            }

        } queues[(uint8_t)QueueType::Count];

        struct CopyAllocator
        {
            VulkanGraphics* device = nullptr;
            VkSemaphore semaphore = VK_NULL_HANDLE;
            uint64_t fenceValue = 0;
            std::mutex locker;

            std::vector<VulkanUploadContext> freeList; // available
            std::vector<VulkanUploadContext> workList; // in progress
            std::vector<VkCommandBuffer> submitCommandBuffers; // for next submit
            uint64_t submitWait = 0; // last submit wait value

            void Init(VulkanGraphics* device);
            void Shutdown();

            VulkanUploadContext Allocate(uint64_t size);
            void Submit(VulkanUploadContext context);
            uint64_t Flush();
        };
        mutable CopyAllocator copyAllocator;

        mutable std::mutex initLocker;
        mutable bool pendingSubmitInits = false;

        struct FrameResources
        {
            VkFence fence[(uint8_t)QueueType::Count] = {};

            VkCommandPool initCommandPool = VK_NULL_HANDLE;
            VkCommandBuffer initCommandBuffer = VK_NULL_HANDLE;

        };
        FrameResources frames[kMaxFramesInFlight] = {};
        const FrameResources& GetFrameResources() const { return frames[GetFrameIndex()]; }
        FrameResources& GetFrameResources() { return frames[GetFrameIndex()]; }

        /* Command queues*/
        std::atomic_uint8_t cmdBuffersCount{ 0 };
        struct CommandListMetadata
        {
            QueueType queue = {};
            std::vector<uint8_t> waits;
        } commandListMeta[kMaxCommandLists];

        std::unique_ptr<VulkanCommandBuffer> commandLists[kMaxCommandLists][(uint8_t)QueueType::Count] = {};

        inline VulkanCommandBuffer* GetCommandBuffer(uint8_t cmd)
        {
            return commandLists[cmd][(uint8_t)commandListMeta[cmd].queue].get();
        }

		// Caches
		std::mutex renderPassCacheMutex;
		std::unordered_map<size_t, VkRenderPass> renderPassCache;

		std::mutex framebufferCacheMutex;
		std::unordered_map<size_t, VkFramebuffer> framebufferCache;

		std::mutex descriptorSetLayoutCacheMutex;
		std::unordered_map<size_t, std::unique_ptr<VulkanDescriptorSetLayout>> descriptorSetLayoutCache;

		std::mutex pipelineLayoutCacheMutex;
		std::unordered_map<size_t, std::unique_ptr<VulkanPipelineLayout>> pipelineLayoutCache;

		// Deletion queue objects
		std::mutex destroyMutex;
		std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> deletionImagesQueue;
		std::deque<std::pair<VkImageView, uint64_t>> deletionImageViews;
		std::deque<std::pair<VkSampler, uint64_t>> destroyedSamplers;
		std::deque<std::pair<std::pair<VkBuffer, VmaAllocation>, uint64_t>> deletionBuffersQueue;
		std::deque<std::pair<VkShaderModule, uint64_t>> deletionShaderModulesQueue;
		std::deque<std::pair<VkPipeline, uint64_t>> deletionPipelinesQueue;
		std::deque<std::pair<VkDescriptorPool, uint64_t>> deletionDescriptorPoolQueue;

        // Bindless
        struct BindlessDescriptorHeap
        {
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            std::vector<uint32_t> freeList;
            std::mutex locker;

            void Init(VkDevice device, VkDescriptorType type, uint32_t descriptorCount)
            {
                descriptorCount = Min(descriptorCount, 100000u);

                // Create descriptor pool 
                VkDescriptorPoolSize poolSize = {};
                poolSize.type = type;
                poolSize.descriptorCount = descriptorCount;

                VkDescriptorPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
                poolInfo.maxSets = 1;
                poolInfo.poolSizeCount = 1;
                poolInfo.pPoolSizes = &poolSize;
                poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
                VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));

                // Create bindless descriptor set layout
                VkDescriptorSetLayoutBinding bindlessLayout{};
                bindlessLayout.binding = 0;
                bindlessLayout.descriptorType = type;
                bindlessLayout.descriptorCount = descriptorCount;
                bindlessLayout.stageFlags = VK_SHADER_STAGE_ALL;
                bindlessLayout.pImmutableSamplers = nullptr;

                const VkDescriptorBindingFlagsEXT bindingFlags =
                    //VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                bindingFlagsInfo.bindingCount = 1;
                bindingFlagsInfo.pBindingFlags = &bindingFlags;

                VkDescriptorSetLayoutCreateInfo bindlessLayoutInfo = {};
                bindlessLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                bindlessLayoutInfo.pNext = &bindingFlagsInfo;
                bindlessLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                bindlessLayoutInfo.bindingCount = 1;
                bindlessLayoutInfo.pBindings = &bindlessLayout;

                VK_CHECK(vkCreateDescriptorSetLayout(device, &bindlessLayoutInfo, nullptr, &descriptorSetLayout));

                // Allocate bindless descriptor set
                VkDescriptorSetAllocateInfo allocateInfo = {};
                allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocateInfo.descriptorPool = descriptorPool;
                allocateInfo.descriptorSetCount = 1;
                allocateInfo.pSetLayouts = &descriptorSetLayout;
                VK_CHECK(vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet));

                for (uint32_t i = 0; i < descriptorCount; ++i)
                {
                    freeList.push_back(descriptorCount - i - 1);
                }
            }

            void Destroy(VkDevice device)
            {
                vkDestroyDescriptorPool(device, descriptorPool, nullptr);
                vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
            }

            uint32_t Allocate()
            {
                locker.lock();
                if (!freeList.empty())
                {
                    uint32_t index = freeList.back();
                    freeList.pop_back();
                    locker.unlock();
                    return index;
                }
                locker.unlock();
                return kInvalidBindlessIndex;
            }

            void Free(uint32_t index)
            {
                ALIMER_ASSERT(index != kInvalidBindlessIndex);

                locker.lock();
                freeList.push_back(index);
                locker.unlock();
            }
        };

        BindlessDescriptorHeap bindlessSampledImages;
        BindlessDescriptorHeap bindlessStorageBuffers;
        BindlessDescriptorHeap bindlessStorageImages;
        BindlessDescriptorHeap bindlessSamplers;

        std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessSampledImages;
        //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessUniformTexelBuffers;
        std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessStorageBuffers;
        std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessStorageImages;
        //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessStorageTexelBuffers;
        std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessSamplers;
        //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessAccelerationStructures;
	};
}
