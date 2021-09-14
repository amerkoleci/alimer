// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Module.h"
#include "Graphics/CommandBuffer.h"
#include <set>

namespace Alimer
{
    struct TextureDesc;
    struct TextureData;
    struct SamplerDesc;
    struct RenderPipelineDesc;
    enum class ShaderStages : uint32_t;
    class Window;

    struct GraphicsCreateInfo
    {
        ValidationMode validationMode = ValidationMode::Disabled;

        PixelFormat depthStencilFormat = PixelFormat::Depth32Float;
        bool vsyncEnabled = false;
        bool isFullScreen = false;
    };

    /// Defines a Graphics module class.
    class ALIMER_API Graphics : public Module<Graphics>
    {
        friend class Texture;
        friend class Sampler;
        friend class Shader;
        friend class Pipeline;

    public:
        virtual ~Graphics() = default;

        [[nodiscard]] static std::set<GraphicsAPI> GetAvailableBackends();
        [[nodiscard]] static GraphicsAPI GetBestPlatformAPI();

        [[nodiscard]] static bool Initialize(GraphicsAPI api, Window& window, const GraphicsCreateInfo& createInfo);

        virtual void WaitIdle() = 0;
        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Resize(uint32_t newWidth, uint32_t newHeight) = 0;

        /// Returns the set of features supported by this device.
        const GraphicsAdapterInfo& GetAdapterInfo() const { return adapterInfo; }

        /// Returns the set of features supported by this device.
        const DeviceFeatures& GetFeatures() const { return features; }

        /// Returns the set of hardware limits for this device.
        const DeviceLimits& GetLimits() const { return limits; }

        [[nodiscard]] virtual Texture* GetCurrentBackBuffer() const = 0;
        [[nodiscard]] virtual Texture* GetBackBuffer(uint32_t index) const = 0;
        [[nodiscard]] virtual uint32_t GetCurrentBackBufferIndex() const = 0;
        [[nodiscard]] virtual uint32_t GetBackBufferCount() const = 0;
        [[nodiscard]] virtual Texture* GetBackBufferDepthStencilTexture() const = 0;

        [[nodiscard]] u64 GetFrameCount() const { return frameCount; }
        [[nodiscard]] u32 GetFrameIndex() const { return frameIndex; }

        /// Return backbuffer width.
        [[nodiscard]] u32 GetBackBufferWidth() const { return backBufferWidth; }
        /// Return backbuffer height.
        [[nodiscard]] u32 GetBackBufferHeight() const { return backBufferHeight; }

    private:
        virtual TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) = 0;
        virtual SamplerRef CreateSampler(const SamplerDesc& desc) = 0;
        virtual ShaderRef CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength) = 0;
        virtual PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc) = 0;

    protected:
        Graphics(Window& window);

        Window& window;
        GraphicsAdapterInfo adapterInfo{};
        DeviceFeatures features{};
        DeviceLimits limits{};

        u32 backBufferWidth = 0;
        u32 backBufferHeight = 0;
        bool vsyncEnabled = false;
        u64 frameCount = 0;
        u32 frameIndex = 0;
    };

    /** Provides easier access to graphics module. */
    ALIMER_API Graphics& gGraphics();
}

