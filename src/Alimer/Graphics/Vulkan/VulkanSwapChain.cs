// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanSwapChain : SwapChain
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkSurfaceKHR _surface;
    private VkSwapchainKHR _handle;
    private uint _acquireSemaphoreIndex;
    private uint _imageIndex;
    private VkSemaphore[]? _acquireSemaphores;
    private VkSemaphore[]? _releaseSemaphores;
    private VulkanTexture[]? _backbufferTextures;
    public readonly object LockObject = new();

    public VulkanSwapChain(VulkanGraphicsDevice device, ISwapChainSurface surfaceSource, in SwapChainDescription descriptor)
        : base(surfaceSource, descriptor)
    {
        _device = device;
        // Create VkSurface first.
        _surface = CreateVkSurface();

        AfterReset();

        VkSurfaceKHR CreateVkSurface()
        {
            VkSurfaceKHR surface = VkSurfaceKHR.Null;
            switch (surfaceSource.Kind)
            {
                case SwapChainSurfaceType.Win32:
                {
                    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        hinstance = surfaceSource.ContextHandle,
                        hwnd = surfaceSource.Handle
                    };
                    device.InstanceApi.vkCreateWin32SurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                    break;
                }

                case SwapChainSurfaceType.Android:
                {
                    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        window = surfaceSource.Handle,
                    };
                    device.InstanceApi.vkCreateAndroidSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                    break;
                }

                case SwapChainSurfaceType.Wayland:
                {
                    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        display = surfaceSource.ContextHandle,
                        surface = surfaceSource.Handle,
                    };
                    device.InstanceApi.vkCreateWaylandSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                    break;
                }

                case SwapChainSurfaceType.Xcb:
                {
                    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        connection = surfaceSource.ContextHandle,
                        window = (uint)(nuint)surfaceSource.Handle,
                    };
                    device.InstanceApi.vkCreateXcbSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                    break;
                }

                case SwapChainSurfaceType.Xlib:
                {
                    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = new()
                    {
                        dpy = surfaceSource.ContextHandle,
                        window = (nuint)surfaceSource.Handle,
                    };
                    device.InstanceApi.vkCreateXlibSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                    break;
                }

                case SwapChainSurfaceType.MetalLayer:
                {
                    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = new()
                    {
                        pLayer = surfaceSource.Handle,
                    };
                    device.InstanceApi.vkCreateMetalSurfaceEXT(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                    break;
                }

                default:
                    throw new GraphicsException($"Vulkan: Invalid kind for surface: {surfaceSource.Kind}");
            }

            VkBool32 presentSupport = false;
            VkResult result = device.InstanceApi.vkGetPhysicalDeviceSurfaceSupportKHR(
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
    public VkFormat VkFormat { get; }
    public VkSwapchainKHR Handle => _handle;
    public uint AcquireSemaphoreIndex => _acquireSemaphoreIndex;
    public VkSemaphore AcquireSemaphore => _acquireSemaphores[_acquireSemaphoreIndex]!;
    public VkSemaphore ReleaseSemaphore => _releaseSemaphores[_imageIndex];
    public uint ImageIndex => _imageIndex;
    public bool NeedAcquire { get; set; }
    public VulkanTexture CurrentTexture => _backbufferTextures![_imageIndex];

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanSwapChain" /> class.
    /// </summary>
    ~VulkanSwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        _device.InstanceApi.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device.PhysicalDevice, _surface, out VkSurfaceCapabilitiesKHR caps).CheckResult();

        _device.InstanceApi.vkGetPhysicalDeviceSurfaceFormatsKHR(_device.PhysicalDevice, _surface, out uint surfaceFormatCount).CheckResult();
        Span<VkSurfaceFormatKHR> swapchainFormats = stackalloc VkSurfaceFormatKHR[(int)surfaceFormatCount];
        _device.InstanceApi.vkGetPhysicalDeviceSurfaceFormatsKHR(_device.PhysicalDevice, _surface, swapchainFormats).CheckResult();

        _device.InstanceApi.vkGetPhysicalDeviceSurfacePresentModesKHR(_device.PhysicalDevice, _surface, out uint presentModeCount).CheckResult();
        Span<VkPresentModeKHR> swapchainPresentModes = stackalloc VkPresentModeKHR[(int)presentModeCount];
        _device.InstanceApi.vkGetPhysicalDeviceSurfacePresentModesKHR(_device.PhysicalDevice, _surface, swapchainPresentModes).CheckResult();

        VkSurfaceFormatKHR surfaceFormat = new()
        {
            format = ((VulkanGraphicsDevice)Device).ToVkFormat(ColorFormat),
            colorSpace = VkColorSpaceKHR.SrgbNonLinear
        };
        bool valid = false;

        bool allowHDR = true;
        foreach (VkSurfaceFormatKHR format in swapchainFormats)
        {
            if (!allowHDR && format.colorSpace != VkColorSpaceKHR.SrgbNonLinear)
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

        VkExtent2D swapChainExtent = default;
        if (caps.currentExtent.width != 0xFFFFFFFF &&
            caps.currentExtent.width != 0xFFFFFFFF)
        {
            swapChainExtent = caps.currentExtent;
        }
        else
        {
            swapChainExtent = new((uint)DrawableSize.Width, (uint)DrawableSize.Height);
            swapChainExtent.width = Math.Max(caps.minImageExtent.width, Math.Min(caps.maxImageExtent.width, swapChainExtent.width));
            swapChainExtent.height = Math.Max(caps.minImageExtent.height, Math.Min(caps.maxImageExtent.height, swapChainExtent.height));
        }

        VkPresentModeKHR presentMode = PresentMode.ToVk();

        {
            ReadOnlySpan<VkPresentModeKHR> kPresentModeFallbacks = stackalloc VkPresentModeKHR[3]
            {
                VkPresentModeKHR.Immediate,
                VkPresentModeKHR.Mailbox,
                VkPresentModeKHR.Fifo
            };

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

            bool HasPresentMode(ReadOnlySpan<VkPresentModeKHR> presentModes, VkPresentModeKHR target)
            {
                foreach (VkPresentModeKHR presentMode in presentModes)
                {
                    if (presentMode == target)
                        return true;
                }

                return false;
            }
            ;

        }

        // Determine the number of images
        uint imageCount = presentMode.MinImageCountForPresentMode();
        imageCount = Math.Max(imageCount, caps.minImageCount);
        if (caps.maxImageCount != 0)
        {
            imageCount = Math.Min(imageCount, caps.maxImageCount);
        }

        VkImageUsageFlags imageUsage = VkImageUsageFlags.ColorAttachment | VkImageUsageFlags.InputAttachment | VkImageUsageFlags.TransferDst;

        VkFormatProperties formatProps;
        _device.InstanceApi.vkGetPhysicalDeviceFormatProperties(_device.PhysicalDevice, surfaceFormat.format, &formatProps);
        if (((formatProps.optimalTilingFeatures & VkFormatFeatureFlags.TransferSrc) != 0) ||
            ((formatProps.optimalTilingFeatures & VkFormatFeatureFlags.BlitSrc) != 0))
        {
            imageUsage |= VkImageUsageFlags.TransferSrc;
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
            imageSharingMode = VkSharingMode.Exclusive,
            preTransform = caps.currentTransform,
            compositeAlpha = VkCompositeAlphaFlagsKHR.Opaque,
            presentMode = presentMode,
            clipped = true,
            oldSwapchain = _handle
        };

        VkResult result = _device.DeviceApi.vkCreateSwapchainKHR(_device.Handle, &createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create SwapChain.");
            return;
        }

        if (createInfo.oldSwapchain.IsNotNull)
        {
            _device.DeviceApi.vkDestroySwapchainKHR(_device.Handle, createInfo.oldSwapchain);
        }

        NeedAcquire = true;
        _device.DeviceApi.vkGetSwapchainImagesKHR(_device.Handle, _handle, out uint actualImageCount).CheckResult();
        Span<VkImage> swapChainImages = stackalloc VkImage[(int)actualImageCount];
        _device.DeviceApi.vkGetSwapchainImagesKHR(_device.Handle, _handle, swapChainImages).CheckResult();
        _acquireSemaphores = new VkSemaphore[actualImageCount];
        _releaseSemaphores = new VkSemaphore[actualImageCount];
        _backbufferTextures = new VulkanTexture[actualImageCount];

        for (int i = 0; i < actualImageCount; i++)
        {
            TextureDescriptor descriptor = TextureDescriptor.Texture2D(
                PixelFormat.BGRA8UnormSrgb, // createInfo.imageFormat.FromVkFormat(),
                createInfo.imageExtent.width,
                createInfo.imageExtent.height,
                usage: TextureUsage.RenderTarget,
                initialLayout: ResourceStates.RenderTarget,
                label: $"BackBuffer texture {i}"
            );

            _backbufferTextures[i] = new VulkanTexture(_device, swapChainImages[i], in descriptor);
            _device.DeviceApi.vkCreateSemaphore(_device.Handle, out _acquireSemaphores[i]).CheckResult();
            _device.DeviceApi.vkCreateSemaphore(_device.Handle, out _releaseSemaphores[i]).CheckResult();
        }
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            for (int i = 0; i < _backbufferTextures!.Length; ++i)
            {
                _backbufferTextures[i].Dispose();
            }
        }

        base.Dispose(disposing);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        for (uint i = 0; i < _backbufferTextures!.Length; ++i)
        {
            _device.DeviceApi.vkDestroySemaphore(_device.Handle, _acquireSemaphores[i]);
            _device.DeviceApi.vkDestroySemaphore(_device.Handle, _releaseSemaphores[i]);
        }

        _device.DeviceApi.vkDestroySwapchainKHR(_device.Handle, _handle, null);
        _device.InstanceApi.vkDestroySurfaceKHR(_device.Instance, _surface, null);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.SurfaceKHR, _handle, newLabel);
        _device.SetObjectName(VkObjectType.SwapchainKHR, _handle, newLabel);
    }

    /// <inheritdoc />
    protected override void ResizeBackBuffer()
    {

    }

    /// <inheritdoc />
    public override Texture? GetCurrentTexture()
    {
        if (!NeedAcquire)
        {
            return _backbufferTextures![ImageIndex];
        }

        VkResult result = VkResult.Success;
        lock (LockObject)
        {
            result = _device.DeviceApi.vkAcquireNextImageKHR(
                _device.Handle,
                Handle,
                ulong.MaxValue,
                _acquireSemaphores![_acquireSemaphoreIndex],
                VkFence.Null,
                out _imageIndex);
        }

        if (result != VkResult.Success)
        {
            // Handle outdated error in acquire
            if (result == VkResult.SuboptimalKHR || result == VkResult.ErrorOutOfDateKHR)
            {
                Device.WaitIdle();
                //_queue.Device.UpdateSwapChain(vulkanSwapChain);
                return GetCurrentTexture();
            }
        }

        NeedAcquire = false;
        return CurrentTexture;
    }

    public void AdvanceFrame()
    {
        _acquireSemaphoreIndex = (_acquireSemaphoreIndex + 1) % (uint)_acquireSemaphores!.Length;
    }
}
