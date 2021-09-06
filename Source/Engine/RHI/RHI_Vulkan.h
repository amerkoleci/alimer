// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "RHI.h"
#include "PlatformInclude.h"
#include "volk.h"
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"

#include <unordered_map>

namespace Alimer::rhi
{
    [[nodiscard]] constexpr const char* ToString(VkResult result)
    {
        switch (result)
        {
#define STR(r)   \
	case VK_##r: \
		return #r
            STR(SUCCESS);
            STR(NOT_READY);
            STR(TIMEOUT);
            STR(EVENT_SET);
            STR(EVENT_RESET);
            STR(INCOMPLETE);
            STR(ERROR_OUT_OF_HOST_MEMORY);
            STR(ERROR_OUT_OF_DEVICE_MEMORY);
            STR(ERROR_INITIALIZATION_FAILED);
            STR(ERROR_DEVICE_LOST);
            STR(ERROR_MEMORY_MAP_FAILED);
            STR(ERROR_LAYER_NOT_PRESENT);
            STR(ERROR_EXTENSION_NOT_PRESENT);
            STR(ERROR_FEATURE_NOT_PRESENT);
            STR(ERROR_INCOMPATIBLE_DRIVER);
            STR(ERROR_TOO_MANY_OBJECTS);
            STR(ERROR_FORMAT_NOT_SUPPORTED);
            STR(ERROR_SURFACE_LOST_KHR);
            STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            STR(SUBOPTIMAL_KHR);
            STR(ERROR_OUT_OF_DATE_KHR);
            STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
            STR(ERROR_VALIDATION_FAILED_EXT);
            STR(ERROR_INVALID_SHADER_NV);
#undef STR
            default:
                return "UNKNOWN_ERROR";
        }
    }

    class Vulkan_Device;

    class Vulkan_Texture final : public RefCounter<ITexture>
    {
    public:
        Vulkan_Device* device;
        TextureDesc desc;
        VkImage handle = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        ~Vulkan_Texture() override;

    private:
        IDevice* GetDevice() const override;
        const TextureDesc& GetDesc() const override { return desc; }
        void ApiSetName() override;
    };

    class Vulkan_Device final : public RefCounter<IDevice>
    {
    private:
        TextureHandle backBuffer;
        Format depthStencilFormat = Format::Undefined;
        TextureHandle depthStencilTexture;

        uint64_t frameCount = 0;
        uint32_t frameIndex = 0;

    public:
        [[nodiscard]] static bool IsAvailable();

        Vulkan_Device(bool enableDebugLayers);
        ~Vulkan_Device() override;

        bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters);
        void WaitIdle() override;
        ICommandList* BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        GraphicsAPI GetGraphicsAPI() const override { return GraphicsAPI::Vulkan; }
        uint64_t GetFrameCount() const override { return frameCount; }
        uint32_t GetFrameIndex() const { return frameIndex; }

        ITexture* GetCurrentBackBuffer() const override { return backBuffer; }

        ITexture* GetBackBuffer(uint32_t index) const override
        {
            if (index == 0)
                return backBuffer;

            return nullptr;
        }

        uint32_t GetCurrentBackBufferIndex() const override { return 0; }
        uint32_t GetBackBufferCount() const override { return 1; }
        ITexture* GetBackBufferDepthStencilTexture() const override { return depthStencilTexture; }

        TextureHandle CreateTextureCore(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) override;
        BufferHandle CreateBufferCore(const BufferDesc& desc, void* nativeHandle, const void* initialData) override;
        SamplerHandle CreateSampler(const SamplerDesc& desc) override;
        ShaderHandle CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main") override;
        PipelineHandle CreateRenderPipelineCore(const RenderPipelineDesc& desc) override;

    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    };
}

/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err) \
		{ \
			LOGE("Detected Vulkan error: {}", Alimer::rhi::ToString(err)); \
		} \
	} while (0)

#define VK_LOG_ERROR(result, message) LOGE("Vulkan: {}, error: {}", message, Alimer::rhi::ToString(result));
