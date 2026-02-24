// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Vulkan.VulkanUtils;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanSwapChain : SwapChain
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkSurfaceKHR _surface;
    private VkSwapchainKHR _handle;
    private uint _acquireSemaphoreIndex;
    private uint _acquireImageIndex;
    private VkSemaphore[]? _acquireSemaphores;
    private VkSemaphore[]? _releaseSemaphores;
    private VulkanTexture[]? _backbufferTextures;
    private readonly Lock _lock = new();

    public VulkanSwapChain(VulkanGraphicsDevice device, in SwapChainDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;

        // Create VkSurface first.
        VkInstance instance = device.VkAdapter.VkGraphicsManager.Instance;
        VkInstanceApi instanceApi = device.VkAdapter.VkGraphicsManager.InstanceApi;
        _surface = CreateVkSurface();

        OnResize(descriptor.Width, descriptor.Height);

        VkSurfaceKHR CreateVkSurface()
        {
            VkSurfaceKHR surface;
            switch (Surface)
            {
                case Win32SwapChainSurface win32Surface:
                {
                    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        hinstance = Win32Native.GetModuleHandleW(null),
                        hwnd = win32Surface.Hwnd
                    };
                    instanceApi.vkCreateWin32SurfaceKHR(in surfaceCreateInfo, out surface).CheckResult();
                    break;
                }

                case AndroidSwapChainSurface androidSurface:
                {
                    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        window = androidSurface.Window,
                    };
                    instanceApi.vkCreateAndroidSurfaceKHR(in surfaceCreateInfo, out surface).CheckResult();
                    break;
                }

                case WaylandSwapChainSurface waylandSurface:
                {
                    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        display = waylandSurface.Display,
                        surface = waylandSurface.Surface,
                    };
                    instanceApi.vkCreateWaylandSurfaceKHR(in surfaceCreateInfo, out surface).CheckResult();
                    break;
                }

                case XlibSwapChainSurface xlibSurface:
                {
                    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        dpy = xlibSurface.Display,
                        window = xlibSurface.Window,
                    };
                    instanceApi.vkCreateXlibSurfaceKHR(in surfaceCreateInfo, out surface).CheckResult();
                    break;
                }

                case MetalLayerChainSurface metalLayerSurface:
                {
                    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = new()
                    {
                        pLayer = metalLayerSurface.Layer,
                    };
                    instanceApi.vkCreateMetalSurfaceEXT(in surfaceCreateInfo, out surface).CheckResult();
                    break;
                }

                default:
                    throw new GraphicsException($"Vulkan: Invalid kind for surface: {Surface.Kind}");
            }

            VkBool32 presentSupport = false;
            VkResult result = instanceApi.vkGetPhysicalDeviceSurfaceSupportKHR(
                device.PhysicalDevice,
                device.GraphicsFamily,
                surface,
                &presentSupport);

            // Present family not found, we cannot create SwapChain
            if (result != VkResult.Success || !presentSupport)
            {
                throw new GraphicsException($"Vulkan: QueueFamilyIndex {device.GraphicsFamily} doesn't support presenting surface");
            }

            return surface;
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public VkSwapchainKHR Handle => _handle;
    public uint AcquireSemaphoreIndex => _acquireSemaphoreIndex;
    public VkSemaphore AcquireSemaphore => _acquireSemaphores![_acquireSemaphoreIndex];
    public VkSemaphore ReleaseSemaphore => _releaseSemaphores![_acquireImageIndex];
    public uint ImageIndex => _acquireImageIndex;
    public VulkanTexture CurrentTexture => _backbufferTextures![_acquireImageIndex];
    public bool NeedAcquire { get; set; }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        for (int i = 0; i < _backbufferTextures!.Length; ++i)
        {
            _backbufferTextures[i].Dispose();
        }

        base.Destroy();
    }

    /// <inheritdoc/>
    protected internal override void BackendDestroy()
    {
        for (uint i = 0; i < _backbufferTextures!.Length; ++i)
        {
            _device.DeviceApi.vkDestroySemaphore(_acquireSemaphores[i]);
            _device.DeviceApi.vkDestroySemaphore(_releaseSemaphores[i]);
        }

        _device.DeviceApi.vkDestroySwapchainKHR(_handle, null);
        _device.VkAdapter.VkGraphicsManager.InstanceApi.vkDestroySurfaceKHR(
            _surface,
            null);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VK_OBJECT_TYPE_SURFACE_KHR, _handle, newLabel);
        _device.SetObjectName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, _handle, newLabel);
    }

    /// <inheritdoc />
    protected override void OnResize(int newWidth, int newHeight)
    {
        VkInstanceApi instanceApi = _device.VkAdapter.VkGraphicsManager.InstanceApi;
        instanceApi.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device.PhysicalDevice, _surface, out VkSurfaceCapabilitiesKHR caps).CheckResult();

        instanceApi.vkGetPhysicalDeviceSurfaceFormatsKHR(_device.PhysicalDevice, _surface, out uint surfaceFormatCount).CheckResult();
        Span<VkSurfaceFormatKHR> swapchainFormats = stackalloc VkSurfaceFormatKHR[(int)surfaceFormatCount];
        instanceApi.vkGetPhysicalDeviceSurfaceFormatsKHR(_device.PhysicalDevice, _surface, swapchainFormats).CheckResult();

        instanceApi.vkGetPhysicalDeviceSurfacePresentModesKHR(_device.PhysicalDevice, _surface, out uint presentModeCount).CheckResult();
        Span<VkPresentModeKHR> swapchainPresentModes = stackalloc VkPresentModeKHR[(int)presentModeCount];
        instanceApi.vkGetPhysicalDeviceSurfacePresentModesKHR(_device.PhysicalDevice, _surface, swapchainPresentModes).CheckResult();

        VkSurfaceFormatKHR surfaceFormat = new()
        {
            format = _device.VkAdapter.ToVkFormat(ColorFormat),
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

        // TODO: Color space
        switch (surfaceFormat.colorSpace)
        {
            case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
                //internal_state->colorSpace = ColorSpace::HDR_LINEAR;
                break;
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
                //internal_state->colorSpace = ColorSpace::HDR10_ST2084;
                break;
            default:
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
                //internal_state->colorSpace = ColorSpace::SRGB;
                break;
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
        instanceApi.vkGetPhysicalDeviceFormatProperties(_device.PhysicalDevice, surfaceFormat.format, &formatProps);
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

        VkResult result = _device.DeviceApi.vkCreateSwapchainKHR(&createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create SwapChain.");
            return;
        }

        if (createInfo.oldSwapchain.IsNotNull)
        {
            _device.DeviceApi.vkDestroySwapchainKHR(createInfo.oldSwapchain);
        }

        // We need to create a new semaphore or jump through a few hoops to wait for the current one to be unsignalled
        // before we can use it again creating a new one is easiest. See also:
        // https://github.com/KhronosGroup/Vulkan-Docs/issues/152
        // https://www.khronos.org/blog/resolving-longstanding-issues-with-wsi
        if (_acquireSemaphores is not null)
        {
            for (int i = 0; i < _acquireSemaphores!.Length; ++i)
            {
                _device.QueueDestroySemaphore(_acquireSemaphores[i]);
            }
            _acquireSemaphores = default;
        }
        if (_releaseSemaphores is not null)
        {
            for (int i = 0; i < _releaseSemaphores!.Length; ++i)
            {
                _device.QueueDestroySemaphore(_releaseSemaphores[i]);
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
        _device.DeviceApi.vkGetSwapchainImagesKHR(_handle, out uint actualImageCount).CheckResult();
        Span<VkImage> swapChainImages = stackalloc VkImage[(int)actualImageCount];
        _device.DeviceApi.vkGetSwapchainImagesKHR(_handle, swapChainImages).CheckResult();
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

            _backbufferTextures[i] = new VulkanTexture(_device, swapChainImages[i], in description, TextureLayout.Undefined);
            _device.DeviceApi.vkCreateSemaphore(out _acquireSemaphores[i]).CheckResult();
            _device.DeviceApi.vkCreateSemaphore(out _releaseSemaphores[i]).CheckResult();
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

        VkResult result = VK_SUCCESS;
        lock (_lock)
        {
            result = _device.DeviceApi.vkAcquireNextImageKHR(
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
                OnResize(Width, Height);
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
