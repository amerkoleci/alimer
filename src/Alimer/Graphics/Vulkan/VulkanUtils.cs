// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Alimer.Utilities;
using Vortice.Vulkan;
using XenoAtom.Collections;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal static unsafe class VulkanUtils
{
    private static readonly VkImageType[] s_vkImageTypeMap = [
        VkImageType.Image1D,
        VkImageType.Image2D,
        VkImageType.Image3D,
        VkImageType.Image2D,
    ];

    #region Layers Methods
    #endregion

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFormat ToVkFormat(PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case PixelFormat.R8Unorm: return VK_FORMAT_R8_UNORM;
            case PixelFormat.R8Snorm: return VK_FORMAT_R8_SNORM;
            case PixelFormat.R8Uint: return VK_FORMAT_R8_UINT;
            case PixelFormat.R8Sint: return VK_FORMAT_R8_SINT;

            // 16-bit formats
            case PixelFormat.R16Uint: return VK_FORMAT_R16_UINT;
            case PixelFormat.R16Sint: return VK_FORMAT_R16_SINT;
            case PixelFormat.R16Unorm: return VK_FORMAT_R16_UNORM;
            case PixelFormat.R16Snorm: return VK_FORMAT_R16_SNORM;
            case PixelFormat.R16Float: return VK_FORMAT_R16_SFLOAT;
            case PixelFormat.RG8Unorm: return VK_FORMAT_R8G8_UNORM;
            case PixelFormat.RG8Snorm: return VK_FORMAT_R8G8_SNORM;
            case PixelFormat.RG8Uint: return VK_FORMAT_R8G8_UINT;
            case PixelFormat.RG8Sint: return VK_FORMAT_R8G8_SINT;
            // Packed 16-Bit Pixel Formats
            case PixelFormat.BGRA4Unorm: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
            case PixelFormat.B5G6R5Unorm: return VK_FORMAT_B5G6R5_UNORM_PACK16;
            case PixelFormat.BGR5A1Unorm: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            // 32-bit formats
            case PixelFormat.R32Uint: return VK_FORMAT_R32_UINT;
            case PixelFormat.R32Sint: return VK_FORMAT_R32_SINT;
            case PixelFormat.R32Float: return VK_FORMAT_R32_SFLOAT;
            case PixelFormat.RG16Uint: return VK_FORMAT_R16G16_UINT;
            case PixelFormat.RG16Sint: return VK_FORMAT_R16G16_SINT;
            case PixelFormat.RG16Unorm: return VK_FORMAT_R16G16_UNORM;
            case PixelFormat.RG16Snorm: return VK_FORMAT_R16G16_SNORM;
            case PixelFormat.RG16Float: return VK_FORMAT_R16G16_SFLOAT;
            case PixelFormat.RGBA8Unorm: return VK_FORMAT_R8G8B8A8_UNORM;
            case PixelFormat.RGBA8UnormSrgb: return VK_FORMAT_R8G8B8A8_SRGB;
            case PixelFormat.RGBA8Snorm: return VK_FORMAT_R8G8B8A8_SNORM;
            case PixelFormat.RGBA8Uint: return VK_FORMAT_R8G8B8A8_UINT;
            case PixelFormat.RGBA8Sint: return VK_FORMAT_R8G8B8A8_SINT;
            case PixelFormat.BGRA8Unorm: return VK_FORMAT_B8G8R8A8_UNORM;
            case PixelFormat.BGRA8UnormSrgb: return VK_FORMAT_B8G8R8A8_SRGB;
            // Packed 32-Bit formats
            case PixelFormat.RGB10A2Unorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case PixelFormat.RGB10A2Uint: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
            case PixelFormat.RG11B10Float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            case PixelFormat.RGB9E5Float: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            // 64-Bit formats
            case PixelFormat.RG32Uint: return VK_FORMAT_R32G32_UINT;
            case PixelFormat.RG32Sint: return VK_FORMAT_R32G32_SINT;
            case PixelFormat.RG32Float: return VK_FORMAT_R32G32_SFLOAT;
            case PixelFormat.RGBA16Uint: return VK_FORMAT_R16G16B16A16_UINT;
            case PixelFormat.RGBA16Sint: return VK_FORMAT_R16G16B16A16_SINT;
            case PixelFormat.RGBA16Unorm: return VK_FORMAT_R16G16B16A16_UNORM;
            case PixelFormat.RGBA16Snorm: return VK_FORMAT_R16G16B16A16_SNORM;
            case PixelFormat.RGBA16Float: return VK_FORMAT_R16G16B16A16_SFLOAT;
            // 128-Bit formats
            case PixelFormat.Rgba32Uint: return VK_FORMAT_R32G32B32A32_UINT;
            case PixelFormat.Rgba32Sint: return VK_FORMAT_R32G32B32A32_SINT;
            case PixelFormat.Rgba32Float: return VK_FORMAT_R32G32B32A32_SFLOAT;

            // Depth-stencil formats
            //case PixelFormat::Stencil8:
            //    return supportsS8 ? VK_FORMAT_S8_UINT : VK_FORMAT_D24_UNORM_S8_UINT;
            case PixelFormat.Depth16Unorm: return VK_FORMAT_D16_UNORM;
            case PixelFormat.Depth24UnormStencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
            case PixelFormat.Depth32Float: return VK_FORMAT_D32_SFLOAT;
            case PixelFormat.Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;

            // Compressed BC formats
            case PixelFormat.Bc1RgbaUnorm: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case PixelFormat.Bc1RgbaUnormSrgb: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            case PixelFormat.Bc2RgbaUnorm: return VK_FORMAT_BC2_UNORM_BLOCK;
            case PixelFormat.Bc2RgbaUnormSrgb: return VK_FORMAT_BC2_SRGB_BLOCK;
            case PixelFormat.Bc3RgbaUnorm: return VK_FORMAT_BC3_UNORM_BLOCK;
            case PixelFormat.Bc3RgbaUnormSrgb: return VK_FORMAT_BC3_SRGB_BLOCK;
            case PixelFormat.Bc4RUnorm: return VK_FORMAT_BC4_UNORM_BLOCK;
            case PixelFormat.Bc4RSnorm: return VK_FORMAT_BC4_SNORM_BLOCK;
            case PixelFormat.Bc5RgUnorm: return VK_FORMAT_BC5_UNORM_BLOCK;
            case PixelFormat.Bc5RgSnorm: return VK_FORMAT_BC5_SNORM_BLOCK;
            case PixelFormat.Bc6hRgbUfloat: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case PixelFormat.Bc6hRgbFloat: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case PixelFormat.Bc7RgbaUnorm: return VK_FORMAT_BC7_UNORM_BLOCK;
            case PixelFormat.Bc7RgbaUnormSrgb: return VK_FORMAT_BC7_SRGB_BLOCK;
            // EAC/ETC compressed formats
            case PixelFormat.Etc2Rgb8Unorm: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            case PixelFormat.Etc2Rgb8UnormSrgb: return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
            case PixelFormat.Etc2Rgb8A1Unorm: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            case PixelFormat.Etc2Rgb8A1UnormSrgb: return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
            case PixelFormat.Etc2Rgba8Unorm: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            case PixelFormat.Etc2Rgba8UnormSrgb: return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
            case PixelFormat.EacR11Unorm: return VK_FORMAT_EAC_R11_UNORM_BLOCK;
            case PixelFormat.EacR11Snorm: return VK_FORMAT_EAC_R11_SNORM_BLOCK;
            case PixelFormat.EacRg11Unorm: return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
            case PixelFormat.EacRg11Snorm: return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
            // ASTC compressed formats
            case PixelFormat.Astc4x4Unorm: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            case PixelFormat.Astc4x4UnormSrgb: return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
            case PixelFormat.Astc5x4Unorm: return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            case PixelFormat.Astc5x4UnormSrgb: return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
            case PixelFormat.Astc5x5Unorm: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            case PixelFormat.Astc5x5UnormSrgb: return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
            case PixelFormat.Astc6x5Unorm: return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            case PixelFormat.Astc6x5UnormSrgb: return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
            case PixelFormat.Astc6x6Unorm: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            case PixelFormat.Astc6x6UnormSrgb: return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
            case PixelFormat.Astc8x5Unorm: return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            case PixelFormat.Astc8x5UnormSrgb: return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
            case PixelFormat.Astc8x6Unorm: return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            case PixelFormat.Astc8x6UnormSrgb: return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
            case PixelFormat.Astc8x8Unorm: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            case PixelFormat.Astc8x8UnormSrgb: return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
            case PixelFormat.Astc10x5Unorm: return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            case PixelFormat.Astc10x5UnormSrgb: return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
            case PixelFormat.Astc10x6Unorm: return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            case PixelFormat.Astc10x6UnormSrgb: return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
            case PixelFormat.Astc10x8Unorm: return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            case PixelFormat.Astc10x8UnormSrgb: return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
            case PixelFormat.Astc10x10Unorm: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            case PixelFormat.Astc10x10UnormSrgb: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
            case PixelFormat.Astc12x10Unorm: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            case PixelFormat.Astc12x10UnormSrgb: return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
            case PixelFormat.Astc12x12Unorm: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            case PixelFormat.Astc12x12UnormSrgb: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
            // ASTC HDR compressed formats
            case PixelFormat.Astc4x4Hdr: return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
            case PixelFormat.Astc5x4Hdr: return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK;
            case PixelFormat.Astc5x5Hdr: return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK;
            case PixelFormat.Astc6x5Hdr: return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK;
            case PixelFormat.Astc6x6Hdr: return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK;
            case PixelFormat.Astc8x5Hdr: return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK;
            case PixelFormat.Astc8x6Hdr: return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK;
            case PixelFormat.Astc8x8Hdr: return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK;
            case PixelFormat.Astc10x5Hdr: return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK;
            case PixelFormat.Astc10x6Hdr: return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK;
            case PixelFormat.Astc10x8Hdr: return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;
            case PixelFormat.Astc10x10Hdr: return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK;
            case PixelFormat.Astc12x10Hdr: return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK;
            case PixelFormat.Astc12x12Hdr: return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK;

            //case PixelFormat.R8BG8Biplanar420Unorm: return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
            //case PixelFormat.R10X6BG10X6Biplanar420Unorm: return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;

            default:
                return VkFormat.Undefined;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFormat ToVk(this VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat.UByte2: return VkFormat.R8G8Uint;
            case VertexFormat.UByte4: return VkFormat.R8G8B8A8Uint;
            case VertexFormat.Byte2: return VkFormat.R8G8Sint;
            case VertexFormat.Byte4: return VkFormat.R8G8B8A8Sint;
            case VertexFormat.UByte2Normalized: return VkFormat.R8G8Unorm;
            case VertexFormat.UByte4Normalized: return VkFormat.R8G8B8A8Unorm;
            case VertexFormat.Byte2Normalized: return VkFormat.R8G8Snorm;
            case VertexFormat.Byte4Normalized: return VkFormat.R8G8B8A8Snorm;

            case VertexFormat.UShort2: return VkFormat.R16G16Uint;
            case VertexFormat.UShort4: return VkFormat.R16G16B16A16Uint;
            case VertexFormat.Short2: return VkFormat.R16G16Sint;
            case VertexFormat.Short4: return VkFormat.R16G16B16A16Sint;
            case VertexFormat.UShort2Normalized: return VkFormat.R16G16Unorm;
            case VertexFormat.UShort4Normalized: return VkFormat.R16G16B16A16Unorm;
            case VertexFormat.Short2Normalized: return VkFormat.R16G16Snorm;
            case VertexFormat.Short4Normalized: return VkFormat.R16G16B16A16Snorm;
            case VertexFormat.Half2: return VkFormat.R16G16Sfloat;
            case VertexFormat.Half4: return VkFormat.R16G16B16A16Sfloat;

            case VertexFormat.Float: return VkFormat.R32Sfloat;
            case VertexFormat.Float2: return VkFormat.R32G32Sfloat;
            case VertexFormat.Float3: return VkFormat.R32G32B32Sfloat;
            case VertexFormat.Float4: return VkFormat.R32G32B32A32Sfloat;

            case VertexFormat.UInt: return VkFormat.R32Uint;
            case VertexFormat.UInt2: return VkFormat.R32G32Uint;
            case VertexFormat.UInt3: return VkFormat.R32G32B32Uint;
            case VertexFormat.UInt4: return VkFormat.R32G32B32A32Uint;

            case VertexFormat.Int: return VkFormat.R32Sint;
            case VertexFormat.Int2: return VkFormat.R32G32Sint;
            case VertexFormat.Int3: return VkFormat.R32G32B32Sint;
            case VertexFormat.Int4: return VkFormat.R32G32B32A32Sint;

            //case VertexFormat.Int1010102Normalized: return VkFormat.A2B10G10R10SnormPack32;
            case VertexFormat.UInt1010102Normalized: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case VertexFormat.RG11B10Float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            case VertexFormat.RGB9E5Float: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

            default:
                return VkFormat.Undefined;
        }
    }

    public static VkSampleCountFlags ToVk(this TextureSampleCount sampleCount)
    {
        switch (sampleCount)
        {
            case TextureSampleCount.Count2:
                return VkSampleCountFlags.Count2;
            case TextureSampleCount.Count4:
                return VkSampleCountFlags.Count4;
            case TextureSampleCount.Count8:
                return VkSampleCountFlags.Count8;
            case TextureSampleCount.Count16:
                return VkSampleCountFlags.Count16;
            case TextureSampleCount.Count32:
                return VkSampleCountFlags.Count32;
            default:
                return VkSampleCountFlags.Count1;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkImageType ToVk(this TextureDimension value) => s_vkImageTypeMap[(uint)value];

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkImageAspectFlags GetVkImageAspectFlags(this VkFormat format)
    {
        switch (format)
        {
            case VkFormat.Undefined:
                return 0;

            case VkFormat.S8Uint:
                return VkImageAspectFlags.Stencil;

            case VkFormat.D16UnormS8Uint:
            case VkFormat.D24UnormS8Uint:
            case VkFormat.D32SfloatS8Uint:
                return VkImageAspectFlags.Stencil | VkImageAspectFlags.Depth;

            case VkFormat.D16Unorm:
            case VkFormat.D32Sfloat:
            case VkFormat.X8D24UnormPack32:
                return VkImageAspectFlags.Depth;

            default:
                return VkImageAspectFlags.Color;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkPresentModeKHR ToVk(this PresentMode value)
    {
        switch (value)
        {
            default:
            case PresentMode.Fifo:
                return VkPresentModeKHR.Fifo;

            case PresentMode.Immediate:
                return VkPresentModeKHR.Immediate;

            case PresentMode.Mailbox:
                return VkPresentModeKHR.Mailbox;
        }
    }

    public static uint MinImageCountForPresentMode(this VkPresentModeKHR mode)
    {
        switch (mode)
        {
            case VkPresentModeKHR.Mailbox:
                return 3;
            default:
            case VkPresentModeKHR.Immediate:
            case VkPresentModeKHR.Fifo:
                return 2;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkQueryType ToVk(this QueryType value)
    {
        switch (value)
        {
            default:
            case QueryType.Timestamp:
                return VkQueryType.Timestamp;

            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
                return VkQueryType.Occlusion;

            case QueryType.PipelineStatistics:
                return VkQueryType.PipelineStatistics;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkAttachmentLoadOp ToVk(this LoadAction value)
    {
        switch (value)
        {
            default:
            case LoadAction.Load:
                return VkAttachmentLoadOp.Load;

            case LoadAction.Clear:
                return VkAttachmentLoadOp.Clear;

            case LoadAction.Discard:
                return VkAttachmentLoadOp.DontCare;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkAttachmentStoreOp ToVk(this StoreAction value)
    {
        switch (value)
        {
            default:
            case StoreAction.Store:
                return VkAttachmentStoreOp.Store;

            case StoreAction.Discard:
                return VkAttachmentStoreOp.DontCare;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkShaderStageFlags ToVk(this ShaderStages stage)
    {
        if (stage == ShaderStages.All)
            return VkShaderStageFlags.All;

        if ((stage & ShaderStages.Library) != 0)
            return VkShaderStageFlags.All;

        VkShaderStageFlags result = VkShaderStageFlags.None;
        if ((stage & ShaderStages.Vertex) != 0)
            result |= VkShaderStageFlags.Vertex;

        if ((stage & ShaderStages.Hull) != 0)
            result |= VkShaderStageFlags.TessellationControl;

        if ((stage & ShaderStages.Domain) != 0)
            result |= VkShaderStageFlags.TessellationEvaluation;

        if ((stage & ShaderStages.Geometry) != 0)
            result |= VkShaderStageFlags.Geometry;

        if ((stage & ShaderStages.Fragment) != 0)
            result |= VkShaderStageFlags.Fragment;

        if ((stage & ShaderStages.Compute) != 0)
            result |= VkShaderStageFlags.Compute;

        if ((stage & ShaderStages.Amplification) != 0)
            result |= VkShaderStageFlags.TaskEXT;

        if ((stage & ShaderStages.Mesh) != 0)
            result |= VkShaderStageFlags.MeshEXT;

        return result;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkPrimitiveTopology ToVk(this PrimitiveTopology type)
    {
        switch (type)
        {
            case PrimitiveTopology.PointList: return VkPrimitiveTopology.PointList;
            case PrimitiveTopology.LineList: return VkPrimitiveTopology.LineList;
            case PrimitiveTopology.LineStrip: return VkPrimitiveTopology.LineStrip;
            case PrimitiveTopology.TriangleStrip: return VkPrimitiveTopology.TriangleStrip;
            case PrimitiveTopology.PatchList: return VkPrimitiveTopology.PatchList;
            default:
                return VkPrimitiveTopology.TriangleList;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkCompareOp ToVk(this CompareFunction value)
    {
        switch (value)
        {
            case CompareFunction.Never: return VkCompareOp.Never;
            case CompareFunction.Less: return VkCompareOp.Less;
            case CompareFunction.Equal: return VkCompareOp.Equal;
            case CompareFunction.LessEqual: return VkCompareOp.LessOrEqual;
            case CompareFunction.Greater: return VkCompareOp.Greater;
            case CompareFunction.NotEqual: return VkCompareOp.NotEqual;
            case CompareFunction.GreaterEqual: return VkCompareOp.GreaterOrEqual;
            case CompareFunction.Always: return VkCompareOp.Always;
            default:
                return VkCompareOp.Never;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkVertexInputRate ToVk(this VertexStepMode value)
    {
        return value switch
        {
            VertexStepMode.Instance => VkVertexInputRate.Instance,
            _ => VkVertexInputRate.Vertex,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkCullModeFlags ToVk(this CullMode value)
    {
        return value switch
        {
            CullMode.Front => VkCullModeFlags.Front,
            CullMode.None => VkCullModeFlags.None,
            _ => VkCullModeFlags.Back,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBlendFactor ToVk(this BlendFactor value)
    {
        switch (value)
        {
            case BlendFactor.Zero:
                return VkBlendFactor.Zero;
            case BlendFactor.One:
                return VkBlendFactor.One;
            case BlendFactor.SourceColor:
                return VkBlendFactor.SrcColor;
            case BlendFactor.OneMinusSourceColor:
                return VkBlendFactor.OneMinusSrcColor;
            case BlendFactor.SourceAlpha:
                return VkBlendFactor.SrcAlpha;
            case BlendFactor.OneMinusSourceAlpha:
                return VkBlendFactor.OneMinusSrcAlpha;
            case BlendFactor.DestinationColor:
                return VkBlendFactor.DstColor;
            case BlendFactor.OneMinusDestinationColor:
                return VkBlendFactor.OneMinusDstColor;
            case BlendFactor.DestinationAlpha:
                return VkBlendFactor.DstAlpha;
            case BlendFactor.OneMinusDestinationAlpha:
                return VkBlendFactor.OneMinusDstAlpha;
            case BlendFactor.SourceAlphaSaturate:
                return VkBlendFactor.SrcAlphaSaturate;
            case BlendFactor.BlendColor:
                return VkBlendFactor.ConstantColor;
            case BlendFactor.OneMinusBlendColor:
                return VkBlendFactor.OneMinusConstantColor;
            case BlendFactor.BlendAlpha:
                return VkBlendFactor.ConstantAlpha;
            case BlendFactor.OneMinusBlendAlpha:
                return VkBlendFactor.OneMinusConstantAlpha;
            case BlendFactor.Source1Color:
                return VkBlendFactor.Src1Color;
            case BlendFactor.OneMinusSource1Color:
                return VkBlendFactor.OneMinusSrc1Color;
            case BlendFactor.Source1Alpha:
                return VkBlendFactor.Src1Alpha;
            case BlendFactor.OneMinusSource1Alpha:
                return VkBlendFactor.OneMinusSrc1Alpha;
            default:
                return VkBlendFactor.Zero;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBlendOp ToVk(this BlendOperation value)
    {
        switch (value)
        {
            case BlendOperation.Subtract: return VkBlendOp.Subtract;
            case BlendOperation.ReverseSubtract: return VkBlendOp.ReverseSubtract;
            case BlendOperation.Min: return VkBlendOp.Min;
            case BlendOperation.Max: return VkBlendOp.Max;
            default:
                return VkBlendOp.Add;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkColorComponentFlags ToVk(this ColorWriteMask value)
    {
        VkColorComponentFlags result = VkColorComponentFlags.None;

        if ((value & ColorWriteMask.Red) != 0)
            result |= VkColorComponentFlags.R;

        if ((value & ColorWriteMask.Green) != 0)
            result |= VkColorComponentFlags.G;

        if ((value & ColorWriteMask.Blue) != 0)
            result |= VkColorComponentFlags.B;

        if ((value & ColorWriteMask.Alpha) != 0)
            result |= VkColorComponentFlags.A;

        return result;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFilter ToVk(this SamplerMinMagFilter value)
    {
        return value switch
        {
            SamplerMinMagFilter.Nearest => VkFilter.Nearest,
            SamplerMinMagFilter.Linear => VkFilter.Linear,
            _ => VkFilter.Nearest,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkSamplerMipmapMode ToVk(this SamplerMipFilter value)
    {
        return value switch
        {
            SamplerMipFilter.Nearest => VkSamplerMipmapMode.Nearest,
            SamplerMipFilter.Linear => VkSamplerMipmapMode.Linear,
            _ => VkSamplerMipmapMode.Nearest,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkSamplerAddressMode ToVk(this SamplerAddressMode value, bool samplerMirrorClampToEdge)
    {
        return value switch
        {
            SamplerAddressMode.Repeat => VkSamplerAddressMode.Repeat,
            SamplerAddressMode.MirrorRepeat => VkSamplerAddressMode.MirroredRepeat,
            SamplerAddressMode.ClampToEdge => VkSamplerAddressMode.ClampToEdge,
            SamplerAddressMode.ClampToBorder => VkSamplerAddressMode.ClampToBorder,
            SamplerAddressMode.MirrorClampToEdge => samplerMirrorClampToEdge ? VkSamplerAddressMode.MirrorClampToEdge : VkSamplerAddressMode.MirroredRepeat,
            _ => VkSamplerAddressMode.Repeat,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBorderColor ToVk(this SamplerBorderColor value)
    {
        return value switch
        {
            SamplerBorderColor.FloatTransparentBlack => VkBorderColor.FloatTransparentBlack,
            SamplerBorderColor.FloatOpaqueBlack => VkBorderColor.FloatOpaqueBlack,
            SamplerBorderColor.FloatOpaqueWhite => VkBorderColor.FloatOpaqueWhite,
            SamplerBorderColor.UintTransparentBlack => VkBorderColor.IntTransparentBlack,
            SamplerBorderColor.UintOpaqueBlack => VkBorderColor.IntOpaqueBlack,
            SamplerBorderColor.UintOpaqueWhite => VkBorderColor.IntOpaqueWhite,
            _ => VkBorderColor.FloatTransparentBlack,
        };
    }

    private static readonly ResourceStateMapping[] s_resourceStateMap = [
        new(ResourceStates.Common,
            VkPipelineStageFlags2.TopOfPipe,
            VkAccessFlags2.None,
            VkImageLayout.Undefined),
        new(ResourceStates.ConstantBuffer,
            VkPipelineStageFlags2.AllCommands,
            VkAccessFlags2.UniformRead,
            VkImageLayout.Undefined),
        new(ResourceStates.VertexBuffer,
            VkPipelineStageFlags2.VertexInput,
            VkAccessFlags2.VertexAttributeRead,
            VkImageLayout.Undefined),
        new(ResourceStates.IndexBuffer,
            VkPipelineStageFlags2.VertexInput,
            VkAccessFlags2.IndexRead,
            VkImageLayout.Undefined),
        new(ResourceStates.IndirectArgument,
            VkPipelineStageFlags2.DrawIndirect,
            VkAccessFlags2.IndirectCommandRead,
            VkImageLayout.Undefined),
        new(ResourceStates.ShaderResource,
            VkPipelineStageFlags2.AllCommands,
            VkAccessFlags2.ShaderRead,
            VkImageLayout.ShaderReadOnlyOptimal),
        new(ResourceStates.UnorderedAccess,
            VkPipelineStageFlags2.AllCommands,
            VkAccessFlags2.ShaderRead | VkAccessFlags2.ShaderWrite,
            VkImageLayout.General),
        new(ResourceStates.RenderTarget,
            VkPipelineStageFlags2.ColorAttachmentOutput,
            VkAccessFlags2.ColorAttachmentRead | VkAccessFlags2.ColorAttachmentWrite,
            VkImageLayout.ColorAttachmentOptimal),
        new(ResourceStates.DepthWrite,
            VkPipelineStageFlags2.EarlyFragmentTests | VkPipelineStageFlags2.LateFragmentTests,
            VkAccessFlags2.DepthStencilAttachmentRead | VkAccessFlags2.DepthStencilAttachmentWrite,
            VkImageLayout.DepthStencilAttachmentOptimal),
        new(ResourceStates.DepthRead,
            VkPipelineStageFlags2.EarlyFragmentTests | VkPipelineStageFlags2.LateFragmentTests,
            VkAccessFlags2.DepthStencilAttachmentRead,
            VkImageLayout.DepthStencilReadOnlyOptimal),
        new(ResourceStates.StreamOut,
            VkPipelineStageFlags2.TransformFeedbackEXT,
            VkAccessFlags2.TransformFeedbackWriteEXT,
            VkImageLayout.Undefined),
        new(ResourceStates.CopyDest,
            VkPipelineStageFlags2.Transfer,
            VkAccessFlags2.TransferWrite,
            VkImageLayout.TransferDstOptimal),
        new(ResourceStates.CopySource,
            VkPipelineStageFlags2.Transfer,
            VkAccessFlags2.TransferRead,
            VkImageLayout.TransferSrcOptimal),
        new(ResourceStates.ResolveDest,
            VkPipelineStageFlags2.Transfer,
            VkAccessFlags2.TransferWrite,
            VkImageLayout.TransferDstOptimal),
        new(ResourceStates.ResolveSource,
            VkPipelineStageFlags2.Transfer,
            VkAccessFlags2.TransferRead,
            VkImageLayout.TransferSrcOptimal),
        new(ResourceStates.Present,
            VkPipelineStageFlags2.AllCommands,
            VkAccessFlags2.MemoryRead,
            VkImageLayout.PresentSrcKHR),
        new(ResourceStates.AccelStructRead,
            VkPipelineStageFlags2.RayTracingShaderKHR | VkPipelineStageFlags2.ComputeShader,
            VkAccessFlags2.AccelerationStructureReadKHR,
            VkImageLayout.Undefined),
        new(ResourceStates.AccelStructWrite,
            VkPipelineStageFlags2.AccelerationStructureBuildKHR,
            VkAccessFlags2.AccelerationStructureWriteKHR,
            VkImageLayout.Undefined),
        new(ResourceStates.AccelStructBuildInput,
            VkPipelineStageFlags2.AccelerationStructureBuildKHR,
            VkAccessFlags2.AccelerationStructureReadKHR,
            VkImageLayout.Undefined),
        new(ResourceStates.AccelStructBuildBlas,
            VkPipelineStageFlags2.AccelerationStructureBuildKHR,
            VkAccessFlags2.AccelerationStructureReadKHR,
            VkImageLayout.Undefined),
        new(ResourceStates.ShadingRateSurface,
            VkPipelineStageFlags2.FragmentShadingRateAttachmentKHR,
            VkAccessFlags2.FragmentShadingRateAttachmentReadKHR,
            VkImageLayout.FragmentShadingRateAttachmentOptimalKHR),
        new(ResourceStates.OpacityMicromapWrite,
            VkPipelineStageFlags2.MicromapBuildEXT,
            VkAccessFlags2.MicromapWriteEXT,
            VkImageLayout.Undefined),
        new(ResourceStates.OpacityMicromapBuildInput,
            VkPipelineStageFlags2.MicromapBuildEXT,
            VkAccessFlags2.ShaderRead,
            VkImageLayout.Undefined),
    ];

    public static ResourceStateMapping ConvertResourceState(ResourceStates state)
    {
        ResourceStates resultState = ResourceStates.Unknown;
        VkPipelineStageFlags2 resultStageFlags = VkPipelineStageFlags2.None;
        VkAccessFlags2 resultAccessMask = VkAccessFlags2.None;
        VkImageLayout resultImageLayout = VkImageLayout.Undefined;

        int numStateBits = s_resourceStateMap.Length;

        uint stateTmp = (uint)state;
        uint bitIndex = 0;

        while (stateTmp != 0 && bitIndex < numStateBits)
        {
            uint bit = (1u << (int)bitIndex);

            if ((stateTmp & bit) != 0)
            {
                ref ResourceStateMapping mapping = ref s_resourceStateMap[bitIndex];

                Debug.Assert((uint)mapping.State == bit);
                Debug.Assert(resultImageLayout == VkImageLayout.Undefined || mapping.ImageLayout == VkImageLayout.Undefined || resultImageLayout == mapping.ImageLayout);

                resultState |= mapping.State;
                resultAccessMask |= mapping.AccessMask;
                resultStageFlags |= mapping.StageFlags;
                if (mapping.ImageLayout != VkImageLayout.Undefined)
                {
                    resultImageLayout = mapping.ImageLayout;
                }

                stateTmp &= ~bit;
            }

            bitIndex++;
        }

        Debug.Assert(resultState == state);

        return new ResourceStateMapping(resultState, resultStageFlags, resultAccessMask, resultImageLayout);
    }

    public readonly record struct ResourceStateMapping(ResourceStates State, VkPipelineStageFlags2 StageFlags, VkAccessFlags2 AccessMask, VkImageLayout ImageLayout);
}
