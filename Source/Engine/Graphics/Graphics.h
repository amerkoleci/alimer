// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Module.h"
#include "Graphics/CommandBuffer.h"

namespace Alimer
{
    struct BufferDesc;
    struct TextureDesc;
    struct TextureData;
    struct SamplerDesc;
    struct RenderPipelineDesc;
    enum class ShaderStages : uint32_t;
    class Window;

    /// Defines a Graphics module class.
    class ALIMER_API Graphics : public Module<Graphics>
    {
        friend class Buffer;
        friend class Texture;
        friend class Sampler;
        friend class Shader;
        friend class Pipeline;

    public:
        Graphics();
        virtual ~Graphics() = default;

        static bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters);

        virtual void WaitIdle() = 0;
        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Resize(uint32_t newWidth, uint32_t newHeight) = 0;

        //! Returns the set of features supported by this device.
        const DeviceFeatures& GetFeatures() const { return features; }

        //! Returns the set of hardware limits for this device.
        const DeviceLimits& GetLimits() const { return limits; }

        [[nodiscard]] virtual Texture* GetCurrentBackBuffer() const = 0;
        [[nodiscard]] virtual Texture* GetBackBuffer(uint32_t index) const = 0;
        [[nodiscard]] virtual uint32_t GetCurrentBackBufferIndex() const = 0;
        [[nodiscard]] virtual uint32_t GetBackBufferCount() const = 0;
        [[nodiscard]] virtual Texture* GetBackBufferDepthStencilTexture() const = 0;

        [[nodiscard]] virtual u64 GetFrameCount() const = 0;
        [[nodiscard]] virtual u32 GetFrameIndex() const = 0;

        

        /// Return backbuffer width.
        [[nodiscard]] uint32_t GetBackBufferWidth() const { return backBufferWidth; }
        /// Return backbuffer height.
        [[nodiscard]] uint32_t GetBackBufferHeight() const { return backBufferHeight; }

    private:
        virtual TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) = 0;
        virtual BufferRef CreateBuffer(const BufferDesc& desc, const void* initialData) = 0;
        virtual SamplerRef CreateSampler(const SamplerDesc& desc) = 0;
        virtual ShaderRef CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength) = 0;
        virtual PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc) = 0;

    protected:
        DeviceFeatures features{};
        DeviceLimits limits{};

        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        bool vsyncEnabled = false;
    };

    /** Provides easier access to graphics module. */
    ALIMER_API Graphics& gGraphics();
}

