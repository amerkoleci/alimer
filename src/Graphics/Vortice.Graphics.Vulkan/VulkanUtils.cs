// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using Vortice.Vulkan;

namespace Vortice.Graphics;

internal static unsafe class VulkanUtils
{

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFormat ToVulkanFormat(this TextureFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case TextureFormat.R8UNorm:
                return VkFormat.R8UNorm;
            case TextureFormat.R8SNorm:
                return VkFormat.R8SNorm;
            case TextureFormat.R8UInt:
                return VkFormat.R8UInt;
            case TextureFormat.R8SInt:
                return VkFormat.R8SInt;
            // 16-bit formats
            case TextureFormat.R16UNorm:
                return VkFormat.R16UNorm;
            case TextureFormat.R16SNorm:
                return VkFormat.R16SNorm;
            case TextureFormat.R16UInt:
                return VkFormat.R16UInt;
            case TextureFormat.R16SInt:
                return VkFormat.R16SInt;
            case TextureFormat.R16Float:
                return VkFormat.R16SFloat;
            case TextureFormat.RG8UNorm:
                return VkFormat.R8G8UNorm;
            case TextureFormat.RG8SNorm:
                return VkFormat.R8G8SNorm;
            case TextureFormat.RG8UInt:
                return VkFormat.R8G8UInt;
            case TextureFormat.RG8SInt:
                return VkFormat.R8G8SInt;
            // 32-bit formats
            case TextureFormat.R32UInt:
                return VkFormat.R32UInt;
            case TextureFormat.R32SInt:
                return VkFormat.R32SInt;
            case TextureFormat.R32Float:
                return VkFormat.R32SFloat;
            case TextureFormat.RG16UNorm:
                return VkFormat.R16G16UNorm;
            case TextureFormat.RG16SNorm:
                return VkFormat.R16G16SNorm;
            case TextureFormat.RG16UInt:
                return VkFormat.R16G16UInt;
            case TextureFormat.RG16SInt:
                return VkFormat.R16G16SInt;
            case TextureFormat.RG16Float:
                return VkFormat.R16G16SFloat;
            case TextureFormat.RGBA8UNorm:
                return VkFormat.R8G8B8A8UNorm;
            case TextureFormat.RGBA8UNormSrgb:
                return VkFormat.R8G8B8A8SRgb;
            case TextureFormat.RGBA8SNorm:
                return VkFormat.R8G8B8A8SNorm;
            case TextureFormat.RGBA8UInt:
                return VkFormat.R8G8B8A8UInt;
            case TextureFormat.RGBA8SInt:
                return VkFormat.R8G8B8A8SInt;
            case TextureFormat.BGRA8UNorm:
                return VkFormat.B8G8R8A8UNorm;
            case TextureFormat.BGRA8UNormSrgb:
                return VkFormat.B8G8R8A8SRgb;
            // Packed 32-Bit formats
            case TextureFormat.RGB10A2UNorm:
                return VkFormat.A2B10G10R10UNormPack32;
            case TextureFormat.RG11B10Float:
                return VkFormat.B10G11R11UFloatPack32;
            case TextureFormat.RGB9E5Float:
                return VkFormat.E5B9G9R9UFloatPack32;
            // 64-Bit formats
            case TextureFormat.RG32UInt:
                return VkFormat.R32G32UInt;
            case TextureFormat.RG32SInt:
                return VkFormat.R32G32SInt;
            case TextureFormat.RG32Float:
                return VkFormat.R32G32SFloat;
            case TextureFormat.RGBA16UNorm:
                return VkFormat.R16G16B16A16UNorm;
            case TextureFormat.RGBA16SNorm:
                return VkFormat.R16G16B16A16SNorm;
            case TextureFormat.RGBA16UInt:
                return VkFormat.R16G16B16A16UInt;
            case TextureFormat.RGBA16SInt:
                return VkFormat.R16G16B16A16SInt;
            case TextureFormat.RGBA16Float:
                return VkFormat.R16G16B16A16SFloat;
            // 128-Bit formats
            case TextureFormat.RGBA32UInt:
                return VkFormat.R32G32B32A32UInt;
            case TextureFormat.RGBA32SInt:
                return VkFormat.R32G32B32A32SInt;
            case TextureFormat.RGBA32Float:
                return VkFormat.R32G32B32A32SFloat;
            // Depth-stencil formats
            case TextureFormat.Depth16UNorm:
                return VkFormat.D16UNorm;
            case TextureFormat.Depth32Float:
                return VkFormat.D32SFloat;
            case TextureFormat.Depth24UNormStencil8:
                return VkFormat.D24UNormS8UInt;
            case TextureFormat.Depth32FloatStencil8:
                return VkFormat.D32SFloatS8UInt;
            // Compressed BC formats
            case TextureFormat.BC1RGBAUNorm:
                return VkFormat.BC1RGBAUNormBlock;
            case TextureFormat.BC1RGBAUNormSrgb:
                return VkFormat.BC1RGBASRgbBlock;
            case TextureFormat.BC2RGBAUNorm:
                return VkFormat.BC2UNormBlock;
            case TextureFormat.BC2RGBAUNormSrgb:
                return VkFormat.BC2SRgbBlock;
            case TextureFormat.BC3RGBAUNorm:
                return VkFormat.BC3UNormBlock;
            case TextureFormat.BC3RGBAUNormSrgb:
                return VkFormat.BC3SRgbBlock;
            case TextureFormat.BC4RSNorm:
                return VkFormat.BC4SNormBlock;
            case TextureFormat.BC4RUNorm:
                return VkFormat.BC4UNormBlock;
            case TextureFormat.BC5RGSNorm:
                return VkFormat.BC5SNormBlock;
            case TextureFormat.BC5RGUNorm:
                return VkFormat.BC5UNormBlock;
            case TextureFormat.BC6HRGBUFloat:
                return VkFormat.BC6HUFloatBlock;
            case TextureFormat.BC6HRGBFloat:
                return VkFormat.BC6HSFloatBlock;
            case TextureFormat.BC7RGBAUNorm:
                return VkFormat.BC7UNormBlock;
            case TextureFormat.BC7RGBAUNormSrgb:
                return VkFormat.BC7SRgbBlock;

            default:
                return ThrowHelper.ThrowArgumentException<VkFormat>("Invalid texture format");
        }
    }

    public static VkMemoryHeap GetMemoryHeap(this VkPhysicalDeviceMemoryProperties memoryProperties, uint index)
    {
        return (&memoryProperties.memoryHeaps_0)[index];
    }
}
