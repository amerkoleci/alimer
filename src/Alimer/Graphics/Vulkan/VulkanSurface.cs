// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Vulkan.VulkanUtils;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanSurface : Surface
{
    private readonly VkSurfaceKHR _surface;
    private VkSwapchainKHR _handle;
    private uint _acquireSemaphoreIndex;
    private uint _acquireImageIndex;
    private VkSemaphore[]? _acquireSemaphores;
    private VkSemaphore[]? _releaseSemaphores;
    private VulkanTexture[]? _backbufferTextures;
    private readonly Lock _lock = new();
    private bool _configured;

    public VkSwapchainKHR Handle => _handle;
    public uint AcquireSemaphoreIndex => _acquireSemaphoreIndex;
    public VkSemaphore AcquireSemaphore => _acquireSemaphores![_acquireSemaphoreIndex];
    public VkSemaphore ReleaseSemaphore => _releaseSemaphores![_acquireImageIndex];
    public uint ImageIndex => _acquireImageIndex;
    public VulkanTexture CurrentTexture => _backbufferTextures![_acquireImageIndex];
    public bool NeedAcquire { get; set; }

    public VulkanSurface(VulkanGraphicsManager manager, in SurfaceDescriptor descriptor)
        : base(descriptor)
    {
        // Create VkSurface first.
        VkInstanceApi instanceApi = manager.InstanceApi;

        switch (Source)
        {
            case Win32SwapChainSurface win32Surface:
            {
                VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = new()
                {
                    hinstance = Win32Native.GetModuleHandleW(null),
                    hwnd = win32Surface.Hwnd
                };
                instanceApi.vkCreateWin32SurfaceKHR(in surfaceCreateInfo, out _surface).CheckResult();
                break;
            }

            case AndroidSwapChainSurface androidSurface:
            {
                VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = new()
                {
                    window = androidSurface.Window,
                };
                instanceApi.vkCreateAndroidSurfaceKHR(in surfaceCreateInfo, out _surface).CheckResult();
                break;
            }

            case WaylandSwapChainSurface waylandSurface:
            {
                VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = new()
                {
                    display = waylandSurface.Display,
                    surface = waylandSurface.Surface,
                };
                instanceApi.vkCreateWaylandSurfaceKHR(in surfaceCreateInfo, out _surface).CheckResult();
                break;
            }

            case XlibSwapChainSurface xlibSurface:
            {
                VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = new()
                {
                    dpy = xlibSurface.Display,
                    window = xlibSurface.Window,
                };
                instanceApi.vkCreateXlibSurfaceKHR(in surfaceCreateInfo, out _surface).CheckResult();
                break;
            }

            case MetalLayerChainSurface metalLayerSurface:
            {
                VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = new()
                {
                    pLayer = metalLayerSurface.Layer,
                };
                instanceApi.vkCreateMetalSurfaceEXT(in surfaceCreateInfo, out _surface).CheckResult();
                break;
            }

            default:
                throw new GraphicsException($"Vulkan: Invalid kind for surface: {Source.Kind}");
        }
    }

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        //DestroySwapchain(true);
        _configured = false;

        if (disposing)
        {
            for (int i = 0; i < _backbufferTextures!.Length; ++i)
            {
                _backbufferTextures[i].Dispose();
            }
        }

        base.Dispose(disposing);
    }

    protected override void ConfigureCore()
    {
        if (_configured)
            return;

        VulkanGraphicsDevice backendDevice = (VulkanGraphicsDevice)Device;
        VkInstanceApi instanceApi = backendDevice.VkAdapter.VkGraphicsManager.InstanceApi;
        //const QueueFamilyIndices&queueFamilyIndices = backendDevice.VkAdapter.qu->queueFamilyIndices;

        VkBool32 presentSupport = false;
        VkResult result = instanceApi.vkGetPhysicalDeviceSurfaceSupportKHR(
            backendDevice.PhysicalDevice,
            backendDevice.GraphicsFamily,
            _surface,
            &presentSupport);

        // Present family not found, we cannot create SwapChain
        if (result != VkResult.Success || !presentSupport)
        {
            throw new GraphicsException($"Vulkan: QueueFamilyIndex {backendDevice.GraphicsFamily} doesn't support presenting surface");
        }

        ResizeCore(Width, Height);
        _configured = true;
    }

    protected override void UnconfigureCore()
    {
        if (!_configured)
            return;

        //DestroySwapchain(false);
        _configured = false;
    }

    //private void DestroySwapchain(bool destroySurface)
    //{
    //
    //}

    /// <inheritdoc/>
    protected internal override void Destroy()
    {
        VulkanGraphicsDevice backendDevice = (VulkanGraphicsDevice)Device;
        for (uint i = 0; i < _acquireSemaphores!.Length; ++i)
        {
            backendDevice.DeviceApi.vkDestroySemaphore(_acquireSemaphores[i]);
        }

        for (uint i = 0; i < _releaseSemaphores!.Length; ++i)
        {
            backendDevice.DeviceApi.vkDestroySemaphore(_releaseSemaphores[i]);
        }

        backendDevice.DeviceApi.vkDestroySwapchainKHR(_handle, null);
        backendDevice.VkAdapter.VkGraphicsManager.InstanceApi.vkDestroySurfaceKHR(
            _surface,
            null);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        VulkanGraphicsDevice backendDevice = (VulkanGraphicsDevice)Device;
        backendDevice.SetObjectName(VK_OBJECT_TYPE_SURFACE_KHR, _handle, newLabel);
        backendDevice.SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, _handle, newLabel);
    }

    /// <inheritdoc />
    protected override void ResizeCore(int newWidth, int newHeight)
    {
        VulkanGraphicsDevice backendDevice = (VulkanGraphicsDevice)Device;
        VkPhysicalDevice physicalDevice = backendDevice.VkAdapter.Handle;
        VkInstanceApi instanceApi = backendDevice.VkAdapter.VkGraphicsManager.InstanceApi;

        instanceApi.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, out VkSurfaceCapabilitiesKHR caps).CheckResult();

        instanceApi.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, out uint surfaceFormatCount).CheckResult();
        Span<VkSurfaceFormatKHR> swapchainFormats = stackalloc VkSurfaceFormatKHR[(int)surfaceFormatCount];
        instanceApi.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, swapchainFormats).CheckResult();

        instanceApi.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, out uint presentModeCount).CheckResult();
        Span<VkPresentModeKHR> swapchainPresentModes = stackalloc VkPresentModeKHR[(int)presentModeCount];
        instanceApi.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, swapchainPresentModes).CheckResult();

        VkSurfaceFormatKHR surfaceFormat = new()
        {
            format = backendDevice.ToVkFormat(Format),
            colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
        bool valid = false;

        bool allowHDR = true;
        foreach (VkSurfaceFormatKHR format in swapchainFormats)
        {
            if (!allowHDR && format.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                continue;

            if (format.format == surfaceFormat.format)
            {
                surfaceFormat = format;
                valid = true;
                break;
            }
        }

        if (!valid)
        {
            surfaceFormat.format = VkFormat.B8G8R8A8Unorm;
            surfaceFormat.colorSpace = VkColorSpaceKHR.SrgbNonLinear;
        }

        // For now, we only include the color spaces that were tested successfully:
        ColorSpace prevColorSpace = ColorSpace;
        switch (surfaceFormat.colorSpace)
        {
            default:
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
                ColorSpace = ColorSpace.SRGB;
                break;

            case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
                ColorSpace = ColorSpace.HDR_LINEAR;
                break;

            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
                ColorSpace = ColorSpace.HDR10_ST2084;
                break;
        }

        if (prevColorSpace != ColorSpace)
        {
            if (_handle.IsNotNull)
            {
                // For some reason, if the swapchain gets recreated (via oldSwapChain) with different color space but same image format, the color space change will not be applied
                backendDevice.DeviceApi.vkDeviceWaitIdle().CheckResult();
                backendDevice.DeviceApi.vkDestroySwapchainKHR(_handle);
                _handle = VkSwapchainKHR.Null;
            }
        }

        VkExtent2D swapChainExtent = default;
        if (caps.currentExtent.width != 0xFFFFFFFF && caps.currentExtent.height != 0xFFFFFFFF)
        {
            swapChainExtent = caps.currentExtent;
        }
        else
        {
            swapChainExtent = new(newWidth, newHeight);
            swapChainExtent.width = Math.Max(caps.minImageExtent.width, Math.Min(caps.maxImageExtent.width, swapChainExtent.width));
            swapChainExtent.height = Math.Max(caps.minImageExtent.height, Math.Min(caps.maxImageExtent.height, swapChainExtent.height));
        }

        VkPresentModeKHR presentMode = PresentMode.ToVk();

        {
            ReadOnlySpan<VkPresentModeKHR> kPresentModeFallbacks =
            [
                VK_PRESENT_MODE_IMMEDIATE_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_FIFO_KHR
            ];

            // Go to the target mode.
            int modeIndex = 0;
            while (kPresentModeFallbacks[modeIndex] != presentMode)
            {
                modeIndex++;
            }

            // Find the first available fallback.
            while (!HasPresentMode(swapchainPresentModes, kPresentModeFallbacks[modeIndex]))
            {
                modeIndex++;
            }

            Debug.Assert(modeIndex < kPresentModeFallbacks.Length);
            presentMode = kPresentModeFallbacks[modeIndex];

            static bool HasPresentMode(ReadOnlySpan<VkPresentModeKHR> presentModes, VkPresentModeKHR target)
            {
                foreach (VkPresentModeKHR presentMode in presentModes)
                {
                    if (presentMode == target)
                        return true;
                }

                return false;
            }
        }

        // Determine the number of images
        uint imageCount = presentMode.MinImageCountForPresentMode();
        imageCount = Math.Max(imageCount, caps.minImageCount);
        if (caps.maxImageCount != 0)
        {
            imageCount = Math.Min(imageCount, caps.maxImageCount);
        }

        VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlags.InputAttachment | VkImageUsageFlags.TransferDst;

        VkFormatProperties formatProps;
        instanceApi.vkGetPhysicalDeviceFormatProperties(physicalDevice, surfaceFormat.format, &formatProps);
        if (((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) != 0) ||
            ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) != 0))
        {
            imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        VkSwapchainCreateInfoKHR createInfo = new()
        {
            surface = _surface,
            minImageCount = imageCount,
            imageFormat = surfaceFormat.format,
            imageColorSpace = surfaceFormat.colorSpace,
            imageExtent = swapChainExtent,
            imageArrayLayers = 1,
            imageUsage = imageUsage,
            imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            preTransform = caps.currentTransform,
            compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            presentMode = presentMode,
            clipped = true,
            oldSwapchain = _handle
        };

        VkResult result = backendDevice.DeviceApi.vkCreateSwapchainKHR(&createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create SwapChain.");
            return;
        }

        if (createInfo.oldSwapchain.IsNotNull)
        {
            backendDevice.QueueDestroySwapchain(createInfo.oldSwapchain);
        }

        // We need to create a new semaphore or jump through a few hoops to wait for the current one to be unsignalled
        // before we can use it again creating a new one is easiest. See also:
        // https://github.com/KhronosGroup/Vulkan-Docs/issues/152
        // https://www.khronos.org/blog/resolving-longstanding-issues-with-wsi
        if (_acquireSemaphores is not null)
        {
            for (int i = 0; i < _acquireSemaphores!.Length; ++i)
            {
                backendDevice.QueueDestroySemaphore(_acquireSemaphores[i]);
            }
            _acquireSemaphores = default;
        }
        if (_releaseSemaphores is not null)
        {
            for (int i = 0; i < _releaseSemaphores!.Length; ++i)
            {
                backendDevice.QueueDestroySemaphore(_releaseSemaphores[i]);
            }
            _releaseSemaphores = default;
        }

        if (_backbufferTextures is not null)
        {
            for (int i = 0; i < _backbufferTextures!.Length; ++i)
            {
                _backbufferTextures[i].Dispose();
            }

            _backbufferTextures = default;
        }

        NeedAcquire = true;
        backendDevice.DeviceApi.vkGetSwapchainImagesKHR(_handle, out uint actualImageCount).CheckResult();
        Span<VkImage> swapChainImages = stackalloc VkImage[(int)actualImageCount];
        backendDevice.DeviceApi.vkGetSwapchainImagesKHR(_handle, swapChainImages).CheckResult();
        _acquireSemaphores = new VkSemaphore[actualImageCount];
        _releaseSemaphores = new VkSemaphore[actualImageCount];
        _backbufferTextures = new VulkanTexture[actualImageCount];
        _acquireImageIndex = 0;
        _acquireSemaphoreIndex = 0;

        for (int i = 0; i < actualImageCount; i++)
        {
            TextureDescriptor description = TextureDescriptor.Texture2D(
                createInfo.imageFormat.FromVkFormat(),
                createInfo.imageExtent.width,
                createInfo.imageExtent.height,
                usage: TextureUsage.RenderTarget
            );
            description.Label = $"BackBuffer texture {i}";

            _backbufferTextures[i] = new VulkanTexture(backendDevice, swapChainImages[i], in description, TextureLayout.Undefined);
            backendDevice.DeviceApi.vkCreateSemaphore(out _acquireSemaphores[i]).CheckResult();
            backendDevice.DeviceApi.vkCreateSemaphore(out _releaseSemaphores[i]).CheckResult();
        }
    }

    /// <inheritdoc />
    public override Texture? AcquireNextTexture()
    {
        if (!NeedAcquire)
        {
            Log.Error("Cannot acquire texture before being submitted to command queue.");
            return default;
        }

        VulkanGraphicsDevice backendDevice = (VulkanGraphicsDevice)Device;
        VkResult result = VK_SUCCESS;
        lock (_lock)
        {
            result = backendDevice.DeviceApi.vkAcquireNextImageKHR(
                Handle,
                TimeoutValue,
                _acquireSemaphores![_acquireSemaphoreIndex],
                VkFence.Null,
                out _acquireImageIndex
                );
        }

        if (result != VK_SUCCESS)
        {
            // Handle outdated error in acquire
            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                // we need to create a new semaphore or jump through a few hoops to
                // wait for the current one to be unsignalled before we can use it again
                // creating a new one is easiest. See also:
                // https://github.com/KhronosGroup/Vulkan-Docs/issues/152
                // https://www.khronos.org/blog/resolving-longstanding-issues-with-wsi
                if (_acquireSemaphores is not null)
                {
                    for (uint i = 0; i < _acquireSemaphores!.Length; ++i)
                    {
                        backendDevice.QueueDestroySemaphore(_acquireSemaphores[i]);
                    }
                    _acquireSemaphores = default;
                }

                ResizeCore(Width, Height);
                return AcquireNextTexture();
            }
        }

        NeedAcquire = false;
        return _backbufferTextures![_acquireImageIndex];
    }

    public void AdvanceFrame()
    {
        _acquireSemaphoreIndex = (_acquireSemaphoreIndex + 1) % (uint)_acquireSemaphores!.Length;
    }
}

internal static partial class Win32Native
{
    [LibraryImport("kernel32")]
    //[SetsLastSystemError]
    public static unsafe partial nint GetModuleHandleW(ushort* lpModuleName);
}
