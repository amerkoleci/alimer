// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanSwapChain : SwapChain
{
    private readonly VkSurfaceKHR _surface;
    private VkSwapchainKHR _handle;
    private VkSemaphore _acquireSemaphore = VkSemaphore.Null;
    private VkSemaphore _releaseSemaphore = VkSemaphore.Null;
    private VulkanTexture[]? _backbufferTextures;
    public readonly object LockObject = new();

    public VulkanSwapChain(VulkanGraphicsDevice device, SwapChainSurface surfaceSource, in SwapChainDescriptor descriptor)
        : base(device, surfaceSource, descriptor)
    {
        // Create VkSurface first.
        _surface = CreateVkSurface();

        AfterReset();

        VkSurfaceKHR CreateVkSurface()
        {
            VkSurfaceKHR surface = VkSurfaceKHR.Null;
            switch (surfaceSource)
            {
                case Win32SwapChainSurface win32Source:
                    {
                        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = new()
                        {
                            hinstance = GetModuleHandleW(null),
                            hwnd = win32Source.Hwnd
                        };
                        vkCreateWin32SurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
                        break;
                    }

                case AndroidSwapChainSurface androidSurface:
                    {
                        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = new()
                        {
                            window = androidSurface.Window,
                        };
                        vkCreateAndroidSurfaceKHR(device.Instance, &surfaceCreateInfo, null, &surface).CheckResult();
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

    public VkInstance VkInstance => ((VulkanGraphicsDevice)Device).Instance;
    public VkDevice VkDevice => ((VulkanGraphicsDevice)Device).Handle;
    public VkPhysicalDevice PhysicalDevice => ((VulkanGraphicsDevice)Device).PhysicalDevice;
    public VkFormat VkFormat { get; }
    public VkSwapchainKHR Handle => _handle;
    public VkSemaphore AcquireSemaphore => _acquireSemaphore;
    public VkSemaphore ReleaseSemaphore => _releaseSemaphore;
    public uint AcquiredImageIndex { get; set; }
    public VulkanTexture CurrentTexture => _backbufferTextures[AcquiredImageIndex];

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanSwapChain" /> class.
    /// </summary>
    ~VulkanSwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, _surface, &caps).CheckResult();

        ReadOnlySpan<VkSurfaceFormatKHR> swapchainFormats = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, _surface);
        ReadOnlySpan<VkPresentModeKHR> swapchainPresentModes = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, _surface);

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
            swapChainExtent = new(DrawableSize.Width, DrawableSize.Height);
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
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice, surfaceFormat.format, &formatProps);
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

        VkResult result = vkCreateSwapchainKHR(VkDevice, &createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create SwapChain.");
            return;
        }

        if (createInfo.oldSwapchain.IsNotNull)
        {
            vkDestroySwapchainKHR(VkDevice, createInfo.oldSwapchain);
        }

        DrawableSize = new((int)createInfo.imageExtent.width, (int)createInfo.imageExtent.height);
        ReadOnlySpan<VkImage> swapChainImages = vkGetSwapchainImagesKHR(VkDevice, _handle);

        _backbufferTextures = new VulkanTexture[swapChainImages.Length];

        for (int i = 0; i < swapChainImages.Length; i++)
        {
            TextureDescriptor descriptor = TextureDescriptor.Texture2D(
                PixelFormat.Bgra8UnormSrgb, // createInfo.imageFormat.FromVkFormat(),
                createInfo.imageExtent.width,
                createInfo.imageExtent.height,
                usage: TextureUsage.RenderTarget,
                label: $"BackBuffer texture {i}"
            );

            _backbufferTextures[i] = new VulkanTexture(Device, swapChainImages[i], descriptor);
        }

        if (_acquireSemaphore.IsNull)
        {
            vkCreateSemaphore(VkDevice, out _acquireSemaphore).CheckResult();
        }

        if (_releaseSemaphore.IsNull)
        {
            vkCreateSemaphore(VkDevice, out _releaseSemaphore).CheckResult();
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
            vkDestroySemaphore(VkDevice, _acquireSemaphore);
        }

        if (_releaseSemaphore.IsNotNull)
        {
            vkDestroySemaphore(VkDevice, _releaseSemaphore);
        }

        vkDestroySwapchainKHR(VkDevice, _handle, null);
        vkDestroySurfaceKHR(VkInstance, _surface, null);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        //_handle.Get()->SetDebugName(newLabel);
    }

    protected override void ResizeBackBuffer(int width, int height)
    {

    }

    [DllImport("kernel32", ExactSpelling = true)]
    //[SetsLastSystemError]
    private static extern nint GetModuleHandleW(ushort* lpModuleName);
}
