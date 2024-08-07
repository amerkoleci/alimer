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
    private VkSemaphore _acquireSemaphore = VkSemaphore.Null;
    private VkSemaphore _releaseSemaphore = VkSemaphore.Null;
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
                        vkCreateWin32SurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                case SwapChainSurfaceType.Android:
                    {
                        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = new()
                        {
                            window = surfaceSource.Handle,
                        };
                        vkCreateAndroidSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                case SwapChainSurfaceType.Wayland:
                    {
                        VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = new()
                        {
                            display = surfaceSource.ContextHandle,
                            surface = surfaceSource.Handle,
                        };
                        vkCreateWaylandSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                case SwapChainSurfaceType.Xcb:
                    {
                        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = new()
                        {
                            connection = surfaceSource.ContextHandle,
                            window = (uint)(nuint)surfaceSource.Handle,
                        };
                        vkCreateXcbSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                case SwapChainSurfaceType.Xlib:
                    {
                        VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = new()
                        {
                            dpy = surfaceSource.ContextHandle,
                            window = (nuint)surfaceSource.Handle,
                        };
                        vkCreateXlibSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                case SwapChainSurfaceType.MetalLayer:
                    {
                        VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = new()
                        {
                            pLayer = surfaceSource.Handle,
                        };
                        vkCreateMetalSurfaceEXT(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                default:
                    throw new GraphicsException($"Vulkan: Invalid kind for surface: {surfaceSource.Kind}");
            }

            VkBool32 presentSupport = false;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
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
    public VkSemaphore AcquireSemaphore => _acquireSemaphore;
    public VkSemaphore ReleaseSemaphore => _releaseSemaphore;
    public uint AcquiredImageIndex { get; set; }
    public bool NeedAcquire { get; set; }   
    public VulkanTexture CurrentTexture => _backbufferTextures![AcquiredImageIndex];

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanSwapChain" /> class.
    /// </summary>
    ~VulkanSwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device.PhysicalDevice, _surface, &caps).CheckResult();

        ReadOnlySpan<VkSurfaceFormatKHR> swapchainFormats = vkGetPhysicalDeviceSurfaceFormatsKHR(_device.PhysicalDevice, _surface);
        ReadOnlySpan<VkPresentModeKHR> swapchainPresentModes = vkGetPhysicalDeviceSurfacePresentModesKHR(_device.PhysicalDevice, _surface);

        VkSurfaceFormatKHR surfaceFormat = new();
        surfaceFormat.format = ((VulkanGraphicsDevice)Device).ToVkFormat(ColorFormat);
        surfaceFormat.colorSpace = VkColorSpaceKHR.SrgbNonLinear;
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
            };

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
        vkGetPhysicalDeviceFormatProperties(_device.PhysicalDevice, surfaceFormat.format, &formatProps);
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

        VkResult result = vkCreateSwapchainKHR(_device.Handle, &createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create SwapChain.");
            return;
        }

        if (createInfo.oldSwapchain.IsNotNull)
        {
            vkDestroySwapchainKHR(_device.Handle, createInfo.oldSwapchain);
        }

        NeedAcquire = true;
        ReadOnlySpan<VkImage> swapChainImages = vkGetSwapchainImagesKHR(_device.Handle, _handle);
        _backbufferTextures = new VulkanTexture[swapChainImages.Length];

        for (int i = 0; i < swapChainImages.Length; i++)
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
        }

        if (_acquireSemaphore.IsNull)
        {
            vkCreateSemaphore(_device.Handle, out _acquireSemaphore).CheckResult();
        }

        if (_releaseSemaphore.IsNull)
        {
            vkCreateSemaphore(_device.Handle, out _releaseSemaphore).CheckResult();
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
        if (_acquireSemaphore.IsNotNull)
        {
            vkDestroySemaphore(_device.Handle, _acquireSemaphore);
        }

        if (_releaseSemaphore.IsNotNull)
        {
            vkDestroySemaphore(_device.Handle, _releaseSemaphore);
        }

        vkDestroySwapchainKHR(_device.Handle, _handle, null);
        vkDestroySurfaceKHR(_device.Instance, _surface, null);
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
            return _backbufferTextures![AcquiredImageIndex];

        VkResult result = VkResult.Success;
        uint imageIndex = 0;
        lock (LockObject)
        {
            result = vkAcquireNextImageKHR(
                _device.Handle,
                Handle,
                ulong.MaxValue,
                _acquireSemaphore,
                VkFence.Null,
                out imageIndex);
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

        AcquiredImageIndex = imageIndex;
        NeedAcquire = false;
        return CurrentTexture;
    }
}
