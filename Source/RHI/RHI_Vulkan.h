// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#define NOMINMAX
#include "RHI.h"
#include "volk.h"
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"

#include <mutex>
#include <deque>
#include <unordered_map>

namespace RHI
{
    class VulkanDevice;

    class VulkanBuffer final : public IBuffer
    {
    public:
        VulkanDevice* device = nullptr;
        uint64_t size = 0;
    };

    class VulkanTexture final : public ITexture
    {
    public:
        VulkanDevice* device = nullptr;
    };

    class VulkanSwapChain final : public ISwapChain
    {
    public:
        VulkanDevice* device = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR handle = VK_NULL_HANDLE;
        VkExtent2D size = {};
        TextureFormat format = TextureFormat::BGRA8UnormSrgb;
        PresentMode presentMode = PresentMode::Fifo;
        uint32_t imageCount = 0;
        

        //mutable bool needAcquire = true;
        mutable uint32_t backBufferIndex = 0;
        std::vector<RefCountPtr<VulkanTexture>> backBufferTextures;
        std::vector<VkImageView> swapChainImageViews;

        ~VulkanSwapChain() override;
        void Destroy(bool destroyHandle);
        bool Resize(uint32_t width, uint32_t height) override;
        VkImageView AcquireNextImage() const;
        void AfterPresent(VkResult result) const;

        VkSemaphore GetImageAvailableSemaphore() const noexcept { return imageAvailableSemaphores[semaphoreIndex]; }
        VkSemaphore GetRenderCompleteSemaphore() const noexcept { return renderCompleteSemaphores[semaphoreIndex]; }

    private:
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderCompleteSemaphores;
        std::vector<VkFence> imageAcquiredFences;
        mutable std::vector<bool> imageAcquiredFenceSubmitted;
        mutable uint32_t semaphoreIndex = 0;
        bool isMinimized = false;
    };

    class VulkanCommandList final : public ICommandList
    {
        friend class VulkanDevice;

    public:
        VulkanDevice* device;
        CommandQueue queue;
        uint8_t index;

        VulkanCommandList(VulkanDevice* device_, CommandQueue queue_, uint8_t index_);
        ~VulkanCommandList() override;

        VkCommandBuffer GetHandle() const;

        void Reset(uint32_t frameIndex);
        void PushDebugGroup(const char* name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const char* name) override;
        void BeginRenderPass(const ISwapChain* swapChain, const float clearColor[4]) override;
        void EndRenderPass() override;

    private:
        VkCommandPool commandPools[kMaxFramesInFlight];
        VkCommandBuffer commandBuffers[kMaxFramesInFlight];
        VkCommandBuffer commandBuffer; // Active command buffer
        bool insideRenderPass = false;
        std::vector<const VulkanSwapChain*> swapChains;

        struct DescriptorBinder
        {
            VulkanDevice* device;
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            uint32_t poolSize = 256;

            std::vector<VkWriteDescriptorSet> descriptorWrites;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkBufferView> texelBufferViews;
            std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelerationStructureViews;
            bool dirty = false;

            //GPUBuffer CBV[DESCRIPTORBINDER_CBV_COUNT];
            //uint64_t CBV_offset[DESCRIPTORBINDER_CBV_COUNT];
            //GPUResource SRV[DESCRIPTORBINDER_SRV_COUNT];
            //int SRV_index[DESCRIPTORBINDER_SRV_COUNT];
            //GPUResource UAV[DESCRIPTORBINDER_UAV_COUNT];
            //int UAV_index[DESCRIPTORBINDER_UAV_COUNT];
            //Sampler SAM[DESCRIPTORBINDER_SAMPLER_COUNT];

            void Init(VulkanDevice* device_);
            void Shutdown();
            void Reset();
            void Flush(bool graphics);
        };
        DescriptorBinder binders[kMaxFramesInFlight];
    };

    struct VulkanCopyContext
    {
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        uint64_t target = 0;
        RefCountPtr<VulkanBuffer> uploadBuffer;
    };


    template <class T>
    void hash_combine(size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    struct VulkanAttachmentDescription
    {
        TextureFormat format;
        LoadAction loadAction;
        StoreAction storeAction;
    };

    struct VulkanRenderPassKey
    {
        uint32_t colorAttachmentCount = 0;
        VulkanAttachmentDescription colorAttachments[kMaxColorAttachments] = {};
        VulkanAttachmentDescription depthStencilAttachment = {};
        uint32_t sampleCount = 1;

        size_t GetHash() const
        {
            if (hash == 0)
            {
                hash_combine(hash, colorAttachmentCount);
                hash_combine(hash, (uint32_t)sampleCount);
                hash_combine(hash, (uint32_t)depthStencilAttachment.format);
                hash_combine(hash, (uint32_t)depthStencilAttachment.loadAction);
                hash_combine(hash, (uint32_t)depthStencilAttachment.storeAction);

                for (uint32_t i = 0; i < colorAttachmentCount; ++i)
                {
                    hash_combine(hash, (uint32_t)colorAttachments[i].format);
                    hash_combine(hash, (uint32_t)colorAttachments[i].loadAction);
                    hash_combine(hash, (uint32_t)colorAttachments[i].storeAction);
                }
            }

            return hash;
        }

    private:
        mutable size_t hash = 0;
    };

    struct VulkanFboKey
    {
        VkRenderPass renderPass;
        uint32_t attachmentCount = 0;
        VkImageView attachments[kMaxColorAttachments + 1] = {};
        uint32_t width;
        uint32_t height;
        uint32_t layers;
    };

    class VulkanDevice final : public IDevice
    {
        friend class VulkanCommandList;

    public:
        VulkanDevice(ValidationMode validationMode);
        ~VulkanDevice() override;

        void WaitIdle() override;
        bool BeginFrame() override;
        void EndFrame() override;
        ICommandList* BeginCommandList(CommandQueue queue = CommandQueue::Graphics) override;
        void SubmitCommandLists();
        void ProcessDeletionQueue();

        TextureHandle CreateTextureCore(const TextureDescriptor* descriptor, const TextureData* initialData) override;
        SwapChainHandle CreateSwapChainCore(void* windowHandle, const SwapChainDescriptor* descriptor) override;

        VkInstance GetInstance() const { return instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
        VkDevice GetHandle() const { return device; }

        VkRenderPass GetVkRenderPass(const VulkanRenderPassKey& key);
        VkFramebuffer GetVkFramebuffer(uint64_t hash, const VulkanFboKey& key);

    private:
        bool debugUtils = false;
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

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

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilies;

        uint32_t graphicsFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t computeFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t copyFamily = VK_QUEUE_FAMILY_IGNORED;
        std::vector<uint32_t> queueFamilies; // Unique queue families

        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;
        VkQueue copyQueue = VK_NULL_HANDLE;

        VmaAllocator allocator = VK_NULL_HANDLE;

        /* Null resource to bind */
        VkBuffer		nullBuffer = VK_NULL_HANDLE;
        VmaAllocation	nullBufferAllocation = VK_NULL_HANDLE;
        VkBufferView	nullBufferView = VK_NULL_HANDLE;
        VkSampler		nullSampler = VK_NULL_HANDLE;
        VmaAllocation	nullImageAllocation1D = VK_NULL_HANDLE;
        VmaAllocation	nullImageAllocation2D = VK_NULL_HANDLE;
        VmaAllocation	nullImageAllocation3D = VK_NULL_HANDLE;
        VkImage			nullImage1D = VK_NULL_HANDLE;
        VkImage			nullImage2D = VK_NULL_HANDLE;
        VkImage			nullImage3D = VK_NULL_HANDLE;
        VkImageView		nullImageView1D = VK_NULL_HANDLE;
        VkImageView		nullImageView1DArray = VK_NULL_HANDLE;
        VkImageView		nullImageView2D = VK_NULL_HANDLE;
        VkImageView		nullImageView2DArray = VK_NULL_HANDLE;
        VkImageView		nullImageViewCube = VK_NULL_HANDLE;
        VkImageView		nullImageViewCubeArray = VK_NULL_HANDLE;
        VkImageView		nullImageView3D = VK_NULL_HANDLE;

        struct Queue
        {
            VkQueue queue = VK_NULL_HANDLE;
            VkSemaphore semaphore = VK_NULL_HANDLE;
            std::vector<const VulkanSwapChain*> submit_swapchains;
            std::vector<VkSwapchainKHR> submitVkSwapchains;
            std::vector<uint32_t> submit_swapChainImageIndices;
            std::vector<VkPipelineStageFlags> submit_waitStages;
            std::vector<VkSemaphore> submit_waitSemaphores;
            std::vector<uint64_t> submit_waitValues;
            std::vector<VkSemaphore> submit_signalSemaphores;
            std::vector<uint64_t> submit_signalValues;
            std::vector<VkCommandBuffer> submit_cmds;

            void Submit(VkFence fence)
            {
                VkTimelineSemaphoreSubmitInfo timelineInfo = {};
                timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
                timelineInfo.pNext = nullptr;
                timelineInfo.waitSemaphoreValueCount = (uint32_t)submit_waitValues.size();
                timelineInfo.pWaitSemaphoreValues = submit_waitValues.data();
                timelineInfo.signalSemaphoreValueCount = (uint32_t)submit_signalValues.size();
                timelineInfo.pSignalSemaphoreValues = submit_signalValues.data();

                VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
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
                    presentInfo.swapchainCount = (uint32_t)submitVkSwapchains.size();
                    presentInfo.pSwapchains = submitVkSwapchains.data();
                    presentInfo.pImageIndices = submit_swapChainImageIndices.data();
                    res = vkQueuePresentKHR(queue, &presentInfo);
                    assert(res == VK_SUCCESS);

                    for (auto& swapchain : submit_swapchains)
                    {
                        swapchain->AfterPresent(res);
                    }
                }

                submit_swapchains.clear();
                submit_swapChainImageIndices.clear();
                submit_waitStages.clear();
                submit_waitSemaphores.clear();
                submit_waitValues.clear();
                submit_signalSemaphores.clear();
                submit_signalValues.clear();
                submit_cmds.clear();
            }

        } queues[(uint8_t)CommandQueue::Count];

        struct CopyAllocator
        {
            VulkanDevice* device = nullptr;
            VkSemaphore semaphore = VK_NULL_HANDLE;
            uint64_t fenceValue = 0;
            std::mutex locker;

            std::vector<VulkanCopyContext> freeList; // available
            std::vector<VulkanCopyContext> workList; // in progress
            std::vector<VkCommandBuffer> submit_cmds; // for next submit
            uint64_t submit_wait = 0; // last submit wait value

            void Init(VulkanDevice* device);
            void Shutdown();
            VulkanCopyContext Allocate(uint64_t size);
            void Submit(VulkanCopyContext context);
            uint64_t Flush();
        };
        mutable CopyAllocator copyAllocator;

        mutable std::mutex initLocker;
        mutable bool submit_inits = false;

        struct FrameResources
        {
            VkFence fence[(uint8_t)CommandQueue::Count] = {};

            VkCommandPool initCommandPool = VK_NULL_HANDLE;
            VkCommandBuffer initCommandBuffer = VK_NULL_HANDLE;

        };
        FrameResources frames[kMaxFramesInFlight] = {};
        const FrameResources& GetFrameResources() const { return frames[GetFrameIndex()]; }
        FrameResources& GetFrameResources() { return frames[GetFrameIndex()]; }

        std::vector<VkDynamicState> psoDynamicStates;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};

        /* Command queues*/
        std::atomic_uint8_t commandListCount{ 0 };
        struct CommandListMetadata
        {
            CommandQueue queue = {};
            std::vector<uint8_t> waits;
        } commandListMeta[kMaxCommandLists];

        std::unique_ptr<VulkanCommandList> commandLists[kMaxCommandLists][(uint8_t)CommandQueue::Count] = {};

        VulkanCommandList* GetCommandList(uint8_t cmd)
        {
            return commandLists[cmd][(uint8_t)commandListMeta[cmd].queue].get();
        }

        /* Caches */
        std::mutex renderPassCacheMutex;
        std::unordered_map<size_t, VkRenderPass> renderPassCache;

        std::mutex framebufferCacheMutex;
        std::unordered_map<size_t, VkFramebuffer> framebufferCache;

        /* Deletion queue */
        std::mutex destroyMutex;
        std::deque<std::pair<std::pair<VkImage, VmaAllocation>, uint64_t>> destroyedImages;
    };
}
