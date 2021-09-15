// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanGraphics.h"

#if defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_XCB_KHR)
#include "Platform/SDL2/Window.SDL2.h"
#include <SDL_vulkan.h>
#endif

namespace Alimer
{
    namespace
    {
        struct SwapChainSupportDetails
        {
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR>   presentModes;
        };

        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            SwapChainSupportDetails details{};

            uint32_t count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
            details.formats.resize(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, details.formats.data());

            // Present modes
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
            details.presentModes.resize(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, details.presentModes.data());

            return details;
        }

        inline VkSurfaceFormatKHR QuerySurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& available_formats,
            VkFormat format)
        {
            if (available_formats.size() == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED)
                return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

            for (const auto& available_format : available_formats)
            {
                if (available_format.format == format && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return available_format;
            }

            return available_formats[0];
        }

        constexpr VkPresentModeKHR QueryPresentMode(const std::vector<VkPresentModeKHR>& availableModes, bool verticalSync)
        {
            if (verticalSync)
            {
                return VK_PRESENT_MODE_FIFO_KHR;
            }

            VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

            for (const auto& availableMode : availableModes)
            {
                if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
                if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                {
                    presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                }
            }

            return presentMode;
        }
    }

    VulkanSwapChain::VulkanSwapChain(VulkanGraphics& device_, void* windowHandle, const SwapChainCreateInfo& info)
        : SwapChain(info)
        , device(device_)
    {
        VkInstance instance = device.GetInstance();

        // Create the os-specific surface
        VkResult result = VK_SUCCESS;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        VkAndroidSurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.window = (ANativeWindow*)windowHandle;
        result = vkCreateAndroidSurfaceKHR(instance, &createInfo, NULL, &surface);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        VkWin32SurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        createInfo.hinstance = GetModuleHandleW(nullptr);
        createInfo.hwnd = (HWND)windowHandle;
        result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        VkIOSSurfaceCreateInfoMVK createInfo{ VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK };
        createInfo.pView = windowHandle;
        result = vkCreateIOSSurfaceMVK(instance, &createInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        VkMacOSSurfaceCreateInfoMVK createInfo { VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK };
        createInfo.pView = windowHandle;
        result = vkCreateMacOSSurfaceMVK(instance, &createInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VkWaylandSurfaceCreateInfoKHR createInfo { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
        createInfo.display = display;
        createInfo.surface = windowHandle;
        result = vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VkXcbSurfaceCreateInfoKHR createInfo { VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
        createInfo.connection = connection;
        createInfo.window = windowHandle;
        result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
        VkHeadlessSurfaceCreateInfoEXT createInfo { VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT };
        result = vkCreateHeadlessSurfaceEXT(instance, &createInfo, nullptr, &surface);
#endif

        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Could not create surface");
            return;
        }

        VkBool32 supported = VK_FALSE;
        VK_CHECK(
            vkGetPhysicalDeviceSurfaceSupportKHR(device.GetPhysicalDevice(), device.GetGraphicsQueueFamily(), surface, &supported)
        );
        ALIMER_ASSERT(supported);

        ResizeBackBuffer(width, height);

        if (info.label != nullptr)
        {
            device.SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)handle, info.label);
        }
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        Destroy();

        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(device.GetInstance(), surface, nullptr);
            surface = VK_NULL_HANDLE;
        }
    }

    void VulkanSwapChain::Destroy()
    {
        Destroy(true);
    }

    void VulkanSwapChain::Destroy(bool destroyHandle)
    {
        VK_CHECK(vkDeviceWaitIdle(device.GetHandle()));

        for (size_t i = 0; i < imageAcquiredFences.size(); ++i)
        {
            if (imageAcquiredFenceSubmitted[i])
            {
                VkFence fence = imageAcquiredFences[i];
                if (vkGetFenceStatus(device.GetHandle(), fence) == VK_NOT_READY)
                {
                    vkWaitForFences(device.GetHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
                }
            }
        }

        backBufferTextures.clear();
        imageAcquiredFenceSubmitted.clear();

        for (uint32_t i = 0; i < bufferCount; i++)
        {
            vkDestroySemaphore(device.GetHandle(), imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device.GetHandle(), renderCompleteSemaphores[i], nullptr);
            vkDestroyFence(device.GetHandle(), imageAcquiredFences[i], nullptr);
        }

        imageAvailableSemaphores.clear();
        renderCompleteSemaphores.clear();
        imageAcquiredFences.clear();
        semaphoreIndex = 0;
        backBufferIndex = 0;
        needAcquire = true;

        if (destroyHandle &&
            handle != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device.GetHandle(), handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
    }

    void VulkanSwapChain::ResizeBackBuffer(uint32_t width, uint32_t height)
    {
        VkSurfaceCapabilitiesKHR caps{};
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), surface, &caps));

        SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(device.GetPhysicalDevice(), surface);

        VkFormat vulkanSurfaceFormat = ToVulkanFormat(colorFormat);
        VkSurfaceFormatKHR surfaceFormat = QuerySurfaceFormat(swapChainDetails.formats, vulkanSurfaceFormat);
        VkPresentModeKHR presentMode = QueryPresentMode(swapChainDetails.presentModes, verticalSync);

        // Determine the number of images
        bufferCount = caps.minImageCount + 1;
        if ((caps.maxImageCount > 0) && (bufferCount > caps.maxImageCount))
        {
            bufferCount = caps.maxImageCount;
        }

        VkSwapchainKHR oldSwapchain = handle;

        VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        info.surface = surface;
        info.minImageCount = bufferCount;
        info.imageFormat = surfaceFormat.format;
        info.imageColorSpace = surfaceFormat.colorSpace;
        info.imageExtent = {};
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;
        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = oldSwapchain;

        if (caps.currentExtent.width == 0xffffffff)
        {
            info.imageExtent.width = width;
            info.imageExtent.height = height;
        }
        else
        {
            info.imageExtent.width = caps.currentExtent.width;
            info.imageExtent.height = caps.currentExtent.height;
        }

        info.preTransform = caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : caps.currentTransform;

        // Simply select the first composite alpha format available
        std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (auto& compositeAlphaFlag : compositeAlphaFlags) {
            if (caps.supportedCompositeAlpha & compositeAlphaFlag) {
                info.compositeAlpha = compositeAlphaFlag;
                break;
            };
        }

        if (vkCreateSwapchainKHR(device.GetHandle(), &info, nullptr, &handle) != VK_SUCCESS)
        {
            return;
        }

        if (oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device.GetHandle(), oldSwapchain, nullptr);
            oldSwapchain = VK_NULL_HANDLE;
        }

        width = info.imageExtent.width;
        height = info.imageExtent.height;
        colorFormat = FromVulkanFormat(info.imageFormat);

        VK_CHECK(vkGetSwapchainImagesKHR(device.GetHandle(), handle, &bufferCount, nullptr));
        std::vector<VkImage> swapchainImages(bufferCount);
        VK_CHECK(vkGetSwapchainImagesKHR(device.GetHandle(), handle, &bufferCount, swapchainImages.data()));
        backBufferTextures.resize(bufferCount);
        imageAvailableSemaphores.resize(bufferCount);
        renderCompleteSemaphores.resize(bufferCount);
        imageAcquiredFences.resize(bufferCount);
        imageAcquiredFenceSubmitted.resize(bufferCount, false);
        semaphoreIndex = 0;
        backBufferIndex = 0;

        TextureCreateInfo textureInfo{};
        textureInfo.width = info.imageExtent.width;
        textureInfo.height = info.imageExtent.height;
        textureInfo.format = FromVulkanFormat(info.imageFormat);
        textureInfo.usage = TextureUsage::RenderTarget;

        const VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        const VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        for (uint32_t i = 0; i < bufferCount; i++)
        {
            VK_CHECK(vkCreateSemaphore(device.GetHandle(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            VK_CHECK(vkCreateSemaphore(device.GetHandle(), &semaphoreInfo, nullptr, &renderCompleteSemaphores[i]));
            VK_CHECK(vkCreateFence(device.GetHandle(), &fenceInfo, nullptr, &imageAcquiredFences[i]));

            // Set debug names for easy track
            if (device.DebugUtilsSupported())
            {
                device.SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)imageAvailableSemaphores[i], fmt::format("ImageAvailableSemaphore {}", i));
                device.SetObjectName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)renderCompleteSemaphores[i], fmt::format("RenderCompleteSemaphore {}", i));
                device.SetObjectName(VK_OBJECT_TYPE_FENCE, (uint64_t)imageAcquiredFences[i], fmt::format("ImageAcquiredFence {}", i));
            }

            VulkanTexture* texture = new VulkanTexture(device, textureInfo, swapchainImages[i]);
            texture->SetSwapChain(this);
            texture->SetWaitSemaphore(imageAvailableSemaphores[i]);
            texture->SetSignalSemaphore(renderCompleteSemaphores[i]);

            backBufferTextures[i] = RefCountPtr<VulkanTexture>::Create(texture);
        }

        isMinimized = false;
    }

    void VulkanSwapChain::AfterPresent(VkResult result)
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR)
        {
            Recreate();
            semaphoreIndex = bufferCount - 1;
        }
        else if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to present swap chain image!");
            return;
        }

        // We need to acquire next image.
        needAcquire = true;
        semaphoreIndex = (semaphoreIndex + 1) % bufferCount;
    }

    void VulkanSwapChain::Recreate()
    {
        VkSurfaceCapabilitiesKHR caps;
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), surface, &caps));

        // Don't create swapchain when window is minimized
        if (caps.currentExtent.width == 0 ||
            caps.currentExtent.height == 0)
        {
            isMinimized = true;
            return;
        }

        Destroy(false);
        ResizeBackBuffer(caps.currentExtent.width, caps.currentExtent.height);
    }

    VkResult VulkanSwapChain::AcquireNextImage() const
    {
        if (!needAcquire) {
            return VK_SUCCESS;
        }

        uint32_t submittedImageFenceIndex = (semaphoreIndex + 1u) % static_cast<uint32_t>(imageAcquiredFenceSubmitted.size());
        if (imageAcquiredFenceSubmitted[submittedImageFenceIndex])
        {
            VkFence submittedFence = imageAcquiredFences[submittedImageFenceIndex];
            if (vkGetFenceStatus(device.GetHandle(), submittedFence) == VK_NOT_READY)
            {
                VK_CHECK(vkWaitForFences(device.GetHandle(), 1, &submittedFence, VK_TRUE, ULLONG_MAX));
            }

            VK_CHECK(vkResetFences(device.GetHandle(), 1, &submittedFence));
            imageAcquiredFenceSubmitted[submittedImageFenceIndex] = false;
        }

        VkFence imageAcquiredFence = imageAcquiredFences[semaphoreIndex];
        VkSemaphore imageAcquiredSemaphore = imageAvailableSemaphores[semaphoreIndex];

        VkResult result = vkAcquireNextImageKHR(device.GetHandle(), handle, UINT64_MAX,
            imageAcquiredSemaphore,
            imageAcquiredFence,
            &backBufferIndex);

        imageAcquiredFenceSubmitted[semaphoreIndex] = (result == VK_SUCCESS);
        if (result == VK_SUCCESS)
        {
            needAcquire = false;
        }

        return result;
    }

    TextureView* VulkanSwapChain::GetCurrentTextureView() const
    {
        if (isMinimized)
        {
            // Cannot render to minimized SwapChain
            return nullptr;
        }

        VkResult result = AcquireNextImage();
        if (result == VK_NOT_READY)
        {
            LOGE("Failed to acquire texture, did you call Present?");
            return nullptr;
        }

        return backBufferTextures[backBufferIndex]->GetDefault();
    }
}
