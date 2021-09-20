// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Module.h"
#include "Graphics/CommandBuffer.h"
//#include "Platform/WindowHandle.h"
#include <mutex>

namespace Alimer
{
    struct BufferCreateInfo;
    struct TextureCreateInfo;
    struct SamplerDesc;
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

        void AddGPUObject(GPUResource* resource);
        void RemoveGPUObject(GPUResource* resource);

        /// Wait for device to finish all pending GPU operations
        virtual void WaitIdle() = 0;

        /// Begin the rendering frame and return false if not possible.
        [[nodiscard]] virtual bool BeginFrame() = 0;

        /// Finish the rendering frame and releases all stale resources.
        virtual void EndFrame() = 0;

        [[nodiscard]] virtual CommandBuffer* BeginCommandBuffer(QueueType queueType = QueueType::Graphics) = 0;

        /// Create new texture.
        [[nodiscard]] TextureRef CreateTexture(const TextureCreateInfo& info, const TextureData* initialData = nullptr);

        /// Create new sampler.
        [[nodiscard]] virtual SamplerRef CreateSampler(const SamplerDesc& desc) = 0;

        /// Return the graphics capabilities.
        [[nodiscard]] const GraphicsDeviceCaps& GetCaps() const noexcept { return caps; }

        /// Return the shader format.
        [[nodiscard]] ShaderFormat GetShaderFormat() const noexcept { return shaderFormat; }

        [[nodiscard]] constexpr u64 GetTimestampFrequency() const { return timestampFrequency; }

        [[nodiscard]] constexpr u32 GetFrameIndex() const { return frameIndex; }
        [[nodiscard]] constexpr u64 GetFrameCount() const { return frameCount; }

        bool IsDeviceLost() const noexcept { return deviceLost; }

    private:
        virtual TextureRef CreateTextureCore(const TextureCreateInfo& info, const TextureData* initialData) = 0;
        virtual BufferRef CreateBuffer(const BufferCreateInfo* info, const void* initialData) = 0;
        virtual ShaderRef CreateShader(ShaderStages stage, const void* byteCode, size_t byteCodeLength, const std::string& entryPoint) = 0;
        virtual PipelineRef CreateRenderPipeline(const RenderPipelineStateCreateInfo* info) = 0;
        virtual PipelineRef CreateComputePipeline(const ComputePipelineCreateInfo* info) = 0;
        virtual SwapChainRef CreateSwapChain(void* window, const SwapChainCreateInfo& info) = 0;

    protected:
        void OnCreated();
        void Destroy();

        GraphicsDeviceCaps caps{};
        ShaderFormat shaderFormat{};

        u64 timestampFrequency = 0;
        u32 frameIndex = 0;
        u64 frameCount = 0;
        bool deviceLost = false;

        /// Mutex for accessing the GPU resource vector from several threads.
        std::mutex objectsMutex;
        /// GPU objects.
        std::vector<GPUResource*> objects;
    };

    /** Provides easier access to graphics module. */
    ALIMER_API Graphics& gGraphics();
}
