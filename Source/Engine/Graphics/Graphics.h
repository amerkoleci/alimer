// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Module.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/CommandBuffer.h"
//#include "Platform/WindowHandle.h"
#include <mutex>

namespace Alimer
{
    struct BufferCreateInfo;
    struct TextureCreateInfo;
    struct SamplerCreateInfo;
    struct SwapChainCreateInfo;
    struct RenderPipelineStateCreateInfo;
    struct ComputePipelineCreateInfo;

    class ALIMER_API Graphics : public Module<Graphics>
    {
        friend class Buffer;
        friend class Shader;
        friend class Sampler;
        friend class Pipeline;
        friend class SwapChain;

    public:
        Graphics() = default;

        virtual ~Graphics() = default;

        [[nodiscard]] static bool Initialize(GraphicsAPI api, ValidationMode validationMode = ValidationMode::Disabled);

        void AddGPUObject(GPUObject* resource);
        void RemoveGPUObject(GPUObject* resource);

        /// Wait for device to finish all pending GPU operations
        virtual void WaitIdle() = 0;

        /// Finish the rendering frame and releases all stale resources.
        virtual void FinishFrame() = 0;

        /// Create new texture.
        [[nodiscard]] TextureRef CreateTexture(const TextureCreateInfo& info, const void* initialData = nullptr);

        /// Create new sampler.
        [[nodiscard]] virtual SamplerRef CreateSampler(const SamplerCreateInfo* info) = 0;

        /// Return the graphics capabilities.
        //const GraphicsDeviceCaps& GetCaps() const noexcept { return caps; }

        CommandQueue& GetQueue(CommandQueueType type = CommandQueueType::Graphics)
        {
            return (type == CommandQueueType::Compute) ? *computeQueue : *graphicsQueue;
        }

        uint32_t GetFrameIndex() const { return frameIndex; }
        uint64_t GetFrameCount() const { return frameCount; }

        /// Get the native device handle (ID3D12Device, VkDevice)
        virtual void* GetNativeHandle() const noexcept = 0;

        bool IsDeviceLost() const noexcept { return deviceLost; }

    private:
        virtual TextureRef CreateTextureCore(const TextureCreateInfo& info, const void* initialData) = 0;
        virtual BufferRef CreateBuffer(const BufferCreateInfo& info, const void* initialData) = 0;
        virtual ShaderRef CreateShader(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint) = 0;
        virtual PipelineRef CreateRenderPipeline(const RenderPipelineStateCreateInfo* info) = 0;
        virtual PipelineRef CreateComputePipeline(const ComputePipelineCreateInfo* info) = 0;
        virtual SwapChainRef CreateSwapChain(void* window, const SwapChainCreateInfo& info) = 0;

    protected:
        void Destroy();

        //GraphicsDeviceCaps caps{};

        CommandQueue* graphicsQueue = nullptr;
        CommandQueue* computeQueue = nullptr;

        uint32_t frameIndex = 0;
        uint64_t frameCount = 0;

        bool deviceLost = false;

        /// Mutex for accessing the GPU resource vector from several threads.
        std::mutex objectsMutex;
        /// GPU objects.
        std::vector<GPUObject*> objects;
    };

    /** Provides easier access to graphics module. */
    ALIMER_API Graphics& gGraphics();
}
