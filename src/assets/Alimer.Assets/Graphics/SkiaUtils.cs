// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Graphics;
using SkiaSharp;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Utilities methods for skia. <see cref="SKBitmap"/>
/// </summary>
public static class SkiaUtils
{
    public static GRContext CreateGRContext(GraphicsDevice device)
    {
        return device.Backend switch
        {
            GraphicsBackend.Vulkan => CreateVulkan(device),
            GraphicsBackend.Direct3D12 => CreateDirect3D(device),
            _ => throw new NotSupportedException($"Graphics backend {device.Backend} is not supported."),
        };
    }

    private static GRContext CreateDirect3D(GraphicsDevice device)
    {
        // Query native handles from device
        GraphicsNativeHandle dxgiAdapter = device.GetNativeHandle(GraphicsNativeHandleType.DXGIAdapter);
        GraphicsNativeHandle d3d12Device = device.GetNativeHandle(GraphicsNativeHandleType.D3D12Device);
        GraphicsNativeHandle d3d12CommandQueue = device.GraphicsQueue.GetNativeHandle(GraphicsNativeHandleType.D3D12CommandQueue);

        return GRContext.CreateDirect3D(new GRD3DBackendContext
        {
            Adapter = dxgiAdapter.Handle,
            Device = d3d12Device.Handle,
            Queue = d3d12CommandQueue.Handle,
            ProtectedContext = false
        });
    }

    private static GRContext CreateVulkan(GraphicsDevice device)
    {
        // Query native handles from device
        GraphicsNativeHandle vkInstance = device.GetNativeHandle(GraphicsNativeHandleType.VkInstance);
        GraphicsNativeHandle vkPhysicalDevice = device.GetNativeHandle(GraphicsNativeHandleType.VkPhysicalDevice);
        GraphicsNativeHandle vkDevice = device.GetNativeHandle(GraphicsNativeHandleType.VkDevice);
        GraphicsNativeHandle vkGraphicsQueue = device.GraphicsQueue.GetNativeHandle(GraphicsNativeHandleType.VkQueue);

#if TODO_VULKAN
        nint GetProcAddressWrapper(string name, IntPtr instance, IntPtr device)
        {
            if (device != IntPtr.Zero)
            {
                var addr = _instance.GetDeviceProcAddress(device, name);
                if (addr != IntPtr.Zero)
                    return addr;
            }

            if (instance != IntPtr.Zero)
            {
                var addr = _instance.GetInstanceProcAddress(instance, name);
                if (addr != IntPtr.Zero)
                    return addr;
            }

            return _instance.GetInstanceProcAddress(IntPtr.Zero, name);
        } 
#endif

        return GRContext.CreateVulkan(new GRVkBackendContext()
        {
            VkInstance = vkInstance.Handle,
            VkPhysicalDevice = vkPhysicalDevice.Handle,
            VkDevice = vkDevice.Handle,
            VkQueue = vkGraphicsQueue.Handle,
            GraphicsQueueIndex = 0, //((VulkanCommandQueue)device.GraphicsQueue).QueueFamilyIndex,
            //GetProcedureAddress = GetProcAddressWrapper,
        });
    }

    public static Span<SKColor> SKColorToColor(Span<SKColor> pixels)
    {
        // ARGB --> ABGR
        foreach (ref uint pixel in MemoryMarshal.Cast<SKColor, uint>(pixels))
        {
            pixel = ((pixel >> 16) & 0x000000FF) |
                    ((pixel << 16) & 0x00FF0000) |
                    (pixel & 0xFF00FF00);
        }

        return MemoryMarshal.Cast<SKColor, SKColor>(pixels);
    }

    public static TextureAsset ToAsset(this SKBitmap bitmap)
    {
        PixelFormat format = PixelFormat.RGBA8Unorm;
        if (bitmap.ColorType == SKColorType.Rgba8888)
        {
            return TextureAsset.Create(bitmap.Width, bitmap.Height, format, bitmap.GetPixelSpan());
        }

        Span<SKColor> pixels = SKColorToColor(bitmap.Pixels);
        return TextureAsset.Create(bitmap.Width, bitmap.Height, format, pixels);
    }

    public static SKBitmap ResizeImage(this SKBitmap original, in SKSizeI size)
    {
        return ResizeImage(original, size, new SKSamplingOptions(SKCubicResampler.Mitchell));
    }

    public static SKBitmap ResizeImage(this SKBitmap original, in SKSizeI size, SKSamplingOptions sampling)
    {
        float ratioX = (float)size.Width / original.Width;
        float ratioY = (float)size.Height / original.Height;
        float ratio = Math.Min(ratioX, ratioY);

        int newWidth = (int)(original.Width * ratio);
        int newHeight = (int)(original.Height * ratio);

        return original.Resize(new SKSizeI(newWidth, newHeight), sampling);
    }

    public static List<SKBitmap> GenerateMipmaps(this SKBitmap source)
    {
        return GenerateMipmaps(source, new SKSamplingOptions(SKCubicResampler.Mitchell));
    }

    public static List<SKBitmap> GenerateMipmaps(this SKBitmap source, SKSamplingOptions sampling)
    {
        uint mipLevelsCount = ImageDescription.GetMipLevelCount((uint)source.Width, (uint)source.Height);
        List<SKBitmap> mipmaps = [source];

        SKBitmap current = source;

        while (current.Width > 1 && current.Height > 1)
        {
            int newWidth = Math.Max(1, current.Width / 2);
            int newHeight = Math.Max(1, current.Height / 2);

            // SKFilterQuality.High = new SKSamplingOptions (SKCubicResampler.Mitchell)
            SKBitmap mipmap = current.Resize(new SKSizeI(newWidth, newHeight), sampling);
            mipmaps.Add(mipmap);
            current = mipmap;
        }

        return mipmaps;
    }
}
