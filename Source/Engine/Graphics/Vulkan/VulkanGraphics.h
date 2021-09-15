// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

//#include "Core/ThreadSafeQueue.h"
#include "Graphics/Graphics.h"
#include "VulkanPipelineLayout.h"
#include <queue>

namespace Alimer
{
    struct VulkanUploadContext
    {
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        uint64_t target = 0;
        RefCountPtr<VulkanBuffer> uploadBuffer;
        uint8_t* data = nullptr;
    };

	class VulkanGraphics final : public Graphics
	{
	public:
		// Null objects for descriptor sets
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
		VkSampler		nullSampler = VK_NULL_HANDLE;

	public:
		VulkanGraphics(ValidationMode validationMode);
		~VulkanGraphics() override;

		void WaitIdle() override;

		void FinishFrame() override;

		void SetObjectName(VkObjectType type, uint64_t handle, const std::string_view& name);

		VkFence AcquireFence();
		void ReleaseFence(VkFence fence);
		void SubmitFence(VkFence fence);

		VkCommandBuffer CreateCommandBuffer();
		void FlushCommandBuffer(VkCommandBuffer commandBuffer);

        VulkanUploadContext UploadBegin(uint64_t size);
        void UploadEnd(VulkanUploadContext context);
        uint64_t FlushCopy();
        VkSemaphore GetCopySemaphore() const;

		VkRenderPass GetVkRenderPass(const VulkanRenderPassKey& key);
		VkFramebuffer GetVkFramebuffer(uint64_t hash, const VulkanFboKey& key);

		VulkanDescriptorSetLayout& RequestDescriptorSetLayout(const uint32_t setIndex, const std::vector<ShaderResource>& resources);
		VulkanPipelineLayout& RequestPipelineLayout(const std::vector<VulkanShader*>& shaders);

		void DeferDestroy(VkImage texture, VmaAllocation allocation);
		void DeferDestroy(VkBuffer buffer, VmaAllocation allocation);
		void DeferDestroy(VkImageView view);
		void DeferDestroy(VkSampler resource);
		void DeferDestroy(VkShaderModule resource);
		void DeferDestroy(VkPipeline resource);
		void DeferDestroy(VkDescriptorPool resource);

		bool DebugUtilsSupported() const noexcept { return debugUtils; }
		VkInstance GetInstance() const noexcept { return instance; }
		VkPhysicalDevice GetPhysicalDevice() const noexcept { return physicalDevice; }
		VmaAllocator GetAllocator() const { return allocator; }
		VkDevice GetHandle() const { return device; }
		VkPipelineCache GetPipelineCache() const { return pipelineCache; }

		bool BufferDeviceAddressSupported() const noexcept { return bufferDeviceAddress; }
		uint32_t GetGraphicsQueueFamily() const noexcept { return graphicsQueueFamily; }
		uint32_t GetComputeQueueFamily() const noexcept { return computeQueueFamily; }
		uint32_t GetCopyQueueFamily() const noexcept { return copyQueueFamily; }

		uint32_t AllocateSRV();
        VkDescriptorSetLayout GetBindlessSampledImageDescriptorSetLayout() const;
        VkDescriptorSet GetBindlessSampledImageDescriptorSet() const;

        uint32_t AllocateUAV();

	private:
		void ProccessCommands();
		void ProcessDeletionQueue();

        TextureRef CreateTextureCore(const TextureCreateInfo& info, const void* initialData) override;
		BufferRef CreateBuffer(const BufferCreateInfo& info, const void* initialData) override;
        ShaderRef CreateShader(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint) override;
		SamplerRef CreateSampler(const SamplerCreateInfo* info) override;
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
		VmaAllocator allocator = VK_NULL_HANDLE;
		bool bufferDeviceAddress = false;

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
        VkQueue copyQueue = VK_NULL_HANDLE;

		VkFormat defaultDepthStencilFormat = VK_FORMAT_UNDEFINED;
		VkFormat defaultDepthFormat = VK_FORMAT_UNDEFINED;

		VkPipelineCache pipelineCache = VK_NULL_HANDLE;

		VkFence frameFences[kMaxFramesInFlight] = {};

		std::mutex fenceMutex;
		std::set<VkFence> allFences;
		std::queue<VkFence> availableFences;

		std::thread processCommandsThread;
		std::mutex processCommandsThreadMutex;
		std::atomic_bool processCommands{ true };
		//ThreadSafeQueue<VkFence> submittedFences;

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
		std::deque<std::pair<VkSampler, uint64_t>> deletionSamplers;
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
                VkDescriptorPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                poolInfo.poolSizeCount = 1;
                poolInfo.pPoolSizes = &poolSize;
                poolInfo.maxSets = 1;
                poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
                VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));

                // Create bindless descriptor set layout
                VkDescriptorSetLayoutBinding bindlessLayout;
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

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
                bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlagsInfo.pNext = nullptr;
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
	};
}
