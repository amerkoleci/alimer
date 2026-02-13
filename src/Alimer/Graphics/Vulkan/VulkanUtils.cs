// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal static unsafe class VulkanUtils
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFormat ToVkFormat(PixelFormat format)
    {
        return format switch
        {
            // 8-bit formats
            PixelFormat.R8Unorm => VK_FORMAT_R8_UNORM,
            PixelFormat.R8Snorm => VK_FORMAT_R8_SNORM,
            PixelFormat.R8Uint => VK_FORMAT_R8_UINT,
            PixelFormat.R8Sint => VK_FORMAT_R8_SINT,
            // 16-bit formats
            PixelFormat.R16Uint => VK_FORMAT_R16_UINT,
            PixelFormat.R16Sint => VK_FORMAT_R16_SINT,
            PixelFormat.R16Unorm => VK_FORMAT_R16_UNORM,
            PixelFormat.R16Snorm => VK_FORMAT_R16_SNORM,
            PixelFormat.R16Float => VK_FORMAT_R16_SFLOAT,
            PixelFormat.RG8Unorm => VK_FORMAT_R8G8_UNORM,
            PixelFormat.RG8Snorm => VK_FORMAT_R8G8_SNORM,
            PixelFormat.RG8Uint => VK_FORMAT_R8G8_UINT,
            PixelFormat.RG8Sint => VK_FORMAT_R8G8_SINT,
            // Packed 16-Bit Pixel Formats
            PixelFormat.BGRA4Unorm => VK_FORMAT_B4G4R4A4_UNORM_PACK16,
            PixelFormat.B5G6R5Unorm => VK_FORMAT_B5G6R5_UNORM_PACK16,
            PixelFormat.BGR5A1Unorm => VK_FORMAT_B5G5R5A1_UNORM_PACK16,
            // 32-bit formats
            PixelFormat.R32Uint => VK_FORMAT_R32_UINT,
            PixelFormat.R32Sint => VK_FORMAT_R32_SINT,
            PixelFormat.R32Float => VK_FORMAT_R32_SFLOAT,
            PixelFormat.RG16Uint => VK_FORMAT_R16G16_UINT,
            PixelFormat.RG16Sint => VK_FORMAT_R16G16_SINT,
            PixelFormat.RG16Unorm => VK_FORMAT_R16G16_UNORM,
            PixelFormat.RG16Snorm => VK_FORMAT_R16G16_SNORM,
            PixelFormat.RG16Float => VK_FORMAT_R16G16_SFLOAT,
            PixelFormat.RGBA8Unorm => VK_FORMAT_R8G8B8A8_UNORM,
            PixelFormat.RGBA8UnormSrgb => VK_FORMAT_R8G8B8A8_SRGB,
            PixelFormat.RGBA8Snorm => VK_FORMAT_R8G8B8A8_SNORM,
            PixelFormat.RGBA8Uint => VK_FORMAT_R8G8B8A8_UINT,
            PixelFormat.RGBA8Sint => VK_FORMAT_R8G8B8A8_SINT,
            PixelFormat.BGRA8Unorm => VK_FORMAT_B8G8R8A8_UNORM,
            PixelFormat.BGRA8UnormSrgb => VK_FORMAT_B8G8R8A8_SRGB,
            // Packed 32-Bit formats
            PixelFormat.RGB10A2Unorm => VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            PixelFormat.RGB10A2Uint => VK_FORMAT_A2R10G10B10_UINT_PACK32,
            PixelFormat.RG11B10Float => VK_FORMAT_B10G11R11_UFLOAT_PACK32,
            PixelFormat.RGB9E5Float => VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
            // 64-Bit formats
            PixelFormat.RG32Uint => VK_FORMAT_R32G32_UINT,
            PixelFormat.RG32Sint => VK_FORMAT_R32G32_SINT,
            PixelFormat.RG32Float => VK_FORMAT_R32G32_SFLOAT,
            PixelFormat.RGBA16Uint => VK_FORMAT_R16G16B16A16_UINT,
            PixelFormat.RGBA16Sint => VK_FORMAT_R16G16B16A16_SINT,
            PixelFormat.RGBA16Unorm => VK_FORMAT_R16G16B16A16_UNORM,
            PixelFormat.RGBA16Snorm => VK_FORMAT_R16G16B16A16_SNORM,
            PixelFormat.RGBA16Float => VK_FORMAT_R16G16B16A16_SFLOAT,
            // 128-Bit formats
            PixelFormat.Rgba32Uint => VK_FORMAT_R32G32B32A32_UINT,
            PixelFormat.Rgba32Sint => VK_FORMAT_R32G32B32A32_SINT,
            PixelFormat.Rgba32Float => VK_FORMAT_R32G32B32A32_SFLOAT,
            // Depth-stencil formats
            //case PixelFormat::Stencil8:
            //    return supportsS8 ? VK_FORMAT_S8_UINT : VK_FORMAT_D24_UNORM_S8_UINT;
            PixelFormat.Depth16Unorm => VK_FORMAT_D16_UNORM,
            PixelFormat.Depth24UnormStencil8 => VK_FORMAT_D24_UNORM_S8_UINT,
            PixelFormat.Depth32Float => VK_FORMAT_D32_SFLOAT,
            PixelFormat.Depth32FloatStencil8 => VK_FORMAT_D32_SFLOAT_S8_UINT,
            // Compressed BC formats
            PixelFormat.Bc1RgbaUnorm => VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
            PixelFormat.Bc1RgbaUnormSrgb => VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
            PixelFormat.Bc2RgbaUnorm => VK_FORMAT_BC2_UNORM_BLOCK,
            PixelFormat.Bc2RgbaUnormSrgb => VK_FORMAT_BC2_SRGB_BLOCK,
            PixelFormat.Bc3RgbaUnorm => VK_FORMAT_BC3_UNORM_BLOCK,
            PixelFormat.Bc3RgbaUnormSrgb => VK_FORMAT_BC3_SRGB_BLOCK,
            PixelFormat.Bc4RUnorm => VK_FORMAT_BC4_UNORM_BLOCK,
            PixelFormat.Bc4RSnorm => VK_FORMAT_BC4_SNORM_BLOCK,
            PixelFormat.Bc5RgUnorm => VK_FORMAT_BC5_UNORM_BLOCK,
            PixelFormat.Bc5RgSnorm => VK_FORMAT_BC5_SNORM_BLOCK,
            PixelFormat.Bc6hRgbUfloat => VK_FORMAT_BC6H_UFLOAT_BLOCK,
            PixelFormat.Bc6hRgbFloat => VK_FORMAT_BC6H_SFLOAT_BLOCK,
            PixelFormat.Bc7RgbaUnorm => VK_FORMAT_BC7_UNORM_BLOCK,
            PixelFormat.Bc7RgbaUnormSrgb => VK_FORMAT_BC7_SRGB_BLOCK,
            // EAC/ETC compressed formats
            PixelFormat.Etc2Rgb8Unorm => VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
            PixelFormat.Etc2Rgb8UnormSrgb => VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
            PixelFormat.Etc2Rgb8A1Unorm => VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
            PixelFormat.Etc2Rgb8A1UnormSrgb => VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
            PixelFormat.Etc2Rgba8Unorm => VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
            PixelFormat.Etc2Rgba8UnormSrgb => VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
            PixelFormat.EacR11Unorm => VK_FORMAT_EAC_R11_UNORM_BLOCK,
            PixelFormat.EacR11Snorm => VK_FORMAT_EAC_R11_SNORM_BLOCK,
            PixelFormat.EacRg11Unorm => VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
            PixelFormat.EacRg11Snorm => VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
            // ASTC compressed formats
            PixelFormat.Astc4x4Unorm => VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
            PixelFormat.Astc4x4UnormSrgb => VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
            PixelFormat.Astc5x4Unorm => VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
            PixelFormat.Astc5x4UnormSrgb => VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
            PixelFormat.Astc5x5Unorm => VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
            PixelFormat.Astc5x5UnormSrgb => VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
            PixelFormat.Astc6x5Unorm => VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
            PixelFormat.Astc6x5UnormSrgb => VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
            PixelFormat.Astc6x6Unorm => VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
            PixelFormat.Astc6x6UnormSrgb => VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
            PixelFormat.Astc8x5Unorm => VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
            PixelFormat.Astc8x5UnormSrgb => VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
            PixelFormat.Astc8x6Unorm => VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
            PixelFormat.Astc8x6UnormSrgb => VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
            PixelFormat.Astc8x8Unorm => VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
            PixelFormat.Astc8x8UnormSrgb => VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
            PixelFormat.Astc10x5Unorm => VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
            PixelFormat.Astc10x5UnormSrgb => VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
            PixelFormat.Astc10x6Unorm => VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
            PixelFormat.Astc10x6UnormSrgb => VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
            PixelFormat.Astc10x8Unorm => VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
            PixelFormat.Astc10x8UnormSrgb => VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
            PixelFormat.Astc10x10Unorm => VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
            PixelFormat.Astc10x10UnormSrgb => VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
            PixelFormat.Astc12x10Unorm => VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
            PixelFormat.Astc12x10UnormSrgb => VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
            PixelFormat.Astc12x12Unorm => VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
            PixelFormat.Astc12x12UnormSrgb => VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
            // ASTC HDR compressed formats
            PixelFormat.Astc4x4Hdr => VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
            PixelFormat.Astc5x4Hdr => VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
            PixelFormat.Astc5x5Hdr => VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
            PixelFormat.Astc6x5Hdr => VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
            PixelFormat.Astc6x6Hdr => VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
            PixelFormat.Astc8x5Hdr => VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
            PixelFormat.Astc8x6Hdr => VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
            PixelFormat.Astc8x8Hdr => VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
            PixelFormat.Astc10x5Hdr => VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
            PixelFormat.Astc10x6Hdr => VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
            PixelFormat.Astc10x8Hdr => VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
            PixelFormat.Astc10x10Hdr => VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
            PixelFormat.Astc12x10Hdr => VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
            PixelFormat.Astc12x12Hdr => VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
            //case PixelFormat.R8BG8Biplanar420Unorm: return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
            //case PixelFormat.R10X6BG10X6Biplanar420Unorm: return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
            _ => VkFormat.Undefined,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFormat ToVk(this VertexAttributeFormat format)
    {
        return format switch
        {
            VertexAttributeFormat.UInt8 => VK_FORMAT_R8_UINT,
            VertexAttributeFormat.UInt8x2 => VK_FORMAT_R8G8_UINT,
            VertexAttributeFormat.UInt8x4 => VK_FORMAT_R8G8B8A8_UINT,
            VertexAttributeFormat.SInt8 => VK_FORMAT_R8_SINT,
            VertexAttributeFormat.SInt8x2 => VK_FORMAT_R8G8_SINT,
            VertexAttributeFormat.SInt8x4 => VK_FORMAT_R8G8B8A8_SINT,
            VertexAttributeFormat.UNorm8 => VK_FORMAT_R8_UNORM,
            VertexAttributeFormat.UNorm8x2 => VK_FORMAT_R8G8_UNORM,
            VertexAttributeFormat.UNorm8x4 => VK_FORMAT_R8G8B8A8_UNORM,
            VertexAttributeFormat.SNorm8 => VK_FORMAT_R8_SNORM,
            VertexAttributeFormat.SNorm8x2 => VK_FORMAT_R8G8_SNORM,
            VertexAttributeFormat.SNorm8x4 => VK_FORMAT_R8G8B8A8_SNORM,
            VertexAttributeFormat.UInt16 => VK_FORMAT_R16_UINT,
            VertexAttributeFormat.UInt16x2 => VK_FORMAT_R16G16_UINT,
            VertexAttributeFormat.UInt16x4 => VK_FORMAT_R16G16B16A16_UINT,
            VertexAttributeFormat.SInt16 => VK_FORMAT_R16_SINT,
            VertexAttributeFormat.SInt16x2 => VK_FORMAT_R16G16_SINT,
            VertexAttributeFormat.SInt16x4 => VK_FORMAT_R16G16B16A16_SINT,
            VertexAttributeFormat.UNorm16 => VK_FORMAT_R16_UNORM,
            VertexAttributeFormat.UNorm16x2 => VK_FORMAT_R16G16_UNORM,
            VertexAttributeFormat.UNorm16x4 => VK_FORMAT_R16G16B16A16_UNORM,
            VertexAttributeFormat.SNorm16 => VK_FORMAT_R16_SNORM,
            VertexAttributeFormat.SNorm16x2 => VK_FORMAT_R16G16_SNORM,
            VertexAttributeFormat.SNorm16x4 => VK_FORMAT_R16G16B16A16_SNORM,
            VertexAttributeFormat.Float16 => VK_FORMAT_R16_SFLOAT,
            VertexAttributeFormat.Float16x2 => VK_FORMAT_R16G16_SFLOAT,
            VertexAttributeFormat.Float16x4 => VK_FORMAT_R16G16B16A16_SFLOAT,
            VertexAttributeFormat.Float32 => VK_FORMAT_R32_SFLOAT,
            VertexAttributeFormat.Float32x2 => VK_FORMAT_R32G32_SFLOAT,
            VertexAttributeFormat.Float32x3 => VK_FORMAT_R32G32B32_SFLOAT,
            VertexAttributeFormat.Float32x4 => VK_FORMAT_R32G32B32A32_SFLOAT,
            VertexAttributeFormat.UInt32 => VK_FORMAT_R32_UINT,
            VertexAttributeFormat.UInt32x2 => VK_FORMAT_R32G32_UINT,
            VertexAttributeFormat.UInt32x3 => VK_FORMAT_R32G32B32_UINT,
            VertexAttributeFormat.UInt32x4 => VK_FORMAT_R32G32B32A32_UINT,
            VertexAttributeFormat.SInt32 => VK_FORMAT_R32_SINT,
            VertexAttributeFormat.SInt32x2 => VK_FORMAT_R32G32_SINT,
            VertexAttributeFormat.SInt32x3 => VK_FORMAT_R32G32B32_SINT,
            VertexAttributeFormat.SInt32x4 => VK_FORMAT_R32G32B32A32_SINT,
            //case VertexFormat.Int1010102Normalized: return VkFormat.A2B10G10R10SnormPack32;
            VertexAttributeFormat.UNorm10_10_10_2 => VK_FORMAT_A2B10G10R10_UNORM_PACK32,
            VertexAttributeFormat.UNorm8x4BGRA => VK_FORMAT_B8G8R8A8_UNORM,
            //case VertexFormat.RG11B10Float:         return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            //case VertexFormat.RGB9E5Float:          return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            _ => VkFormat.Undefined,
        };
    }

    public static VkSampleCountFlags ToVk(this TextureSampleCount sampleCount)
    {
        return sampleCount switch
        {
            TextureSampleCount.Count2 => VK_SAMPLE_COUNT_2_BIT,
            TextureSampleCount.Count4 => VK_SAMPLE_COUNT_4_BIT,
            TextureSampleCount.Count8 => VK_SAMPLE_COUNT_8_BIT,
            TextureSampleCount.Count16 => VK_SAMPLE_COUNT_16_BIT,
            TextureSampleCount.Count32 => VK_SAMPLE_COUNT_32_BIT,
            _ => VK_SAMPLE_COUNT_1_BIT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkImageType ToVk(this TextureDimension value)
    {
        return value switch
        {
            TextureDimension.Texture1D => VK_IMAGE_TYPE_1D,
            TextureDimension.Texture2D => VK_IMAGE_TYPE_2D,
            TextureDimension.Texture3D => VK_IMAGE_TYPE_3D,
            _ => VK_IMAGE_TYPE_2D,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkImageViewType ToVk(this TextureViewDimension value)
    {
        return value switch
        {
            TextureViewDimension.View1D => VK_IMAGE_VIEW_TYPE_1D,
            TextureViewDimension.View1DArray => VK_IMAGE_VIEW_TYPE_1D_ARRAY,
            TextureViewDimension.View2D => VK_IMAGE_VIEW_TYPE_2D,
            TextureViewDimension.View2DArray => VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            TextureViewDimension.ViewCube => VK_IMAGE_VIEW_TYPE_CUBE,
            TextureViewDimension.ViewCubeArray => VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
            TextureViewDimension.View3D => VK_IMAGE_VIEW_TYPE_3D,
            _ => VK_IMAGE_VIEW_TYPE_2D,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkImageAspectFlags GetImageAspectFlags(this VkFormat format, TextureAspect aspect)
    {
        return aspect switch
        {
            TextureAspect.All => format switch
            {
                VK_FORMAT_D16_UNORM_S8_UINT or VK_FORMAT_D24_UNORM_S8_UINT or VK_FORMAT_D32_SFLOAT_S8_UINT => VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                VK_FORMAT_D16_UNORM or VK_FORMAT_D32_SFLOAT or VK_FORMAT_X8_D24_UNORM_PACK32 => VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_FORMAT_S8_UINT => VK_IMAGE_ASPECT_STENCIL_BIT,
                _ => VK_IMAGE_ASPECT_COLOR_BIT,
            },
            TextureAspect.DepthOnly => VK_IMAGE_ASPECT_DEPTH_BIT,
            TextureAspect.StencilOnly => VK_IMAGE_ASPECT_STENCIL_BIT,
            _ => VK_IMAGE_ASPECT_COLOR_BIT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkPresentModeKHR ToVk(this PresentMode value)
    {
        return value switch
        {
            PresentMode.Immediate => VK_PRESENT_MODE_IMMEDIATE_KHR,
            PresentMode.Mailbox => VK_PRESENT_MODE_MAILBOX_KHR,
            _ => VK_PRESENT_MODE_FIFO_KHR,
        };
    }

    public static uint MinImageCountForPresentMode(this VkPresentModeKHR mode)
    {
        return mode switch
        {
            VkPresentModeKHR.Mailbox => 3,
            _ => 2,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkQueryType ToVk(this QueryType value)
    {
        return value switch
        {
            //QueryType.Occlusion or QueryType.BinaryOcclusion => VK_QUERY_TYPE_OCCLUSION,
            QueryType.Occlusion => VK_QUERY_TYPE_OCCLUSION,
            QueryType.PipelineStatistics => VK_QUERY_TYPE_PIPELINE_STATISTICS,
            _ => VK_QUERY_TYPE_TIMESTAMP,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkAttachmentLoadOp ToVk(this LoadAction value)
    {
        return value switch
        {
            LoadAction.Clear => VK_ATTACHMENT_LOAD_OP_CLEAR,
            LoadAction.Discard => VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            _ => VK_ATTACHMENT_LOAD_OP_LOAD,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkAttachmentStoreOp ToVk(this StoreAction value)
    {
        return value switch
        {
            StoreAction.Discard => VK_ATTACHMENT_STORE_OP_DONT_CARE,
            _ => VK_ATTACHMENT_STORE_OP_STORE,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkShaderStageFlags ToVk(this ShaderStages stage)
    {
        if (stage == ShaderStages.All)
            return VK_SHADER_STAGE_ALL;

#if RAYTRACING
        if ((stage & ShaderStages.Library) != 0)
            return VK_SHADER_STAGE_ALL;
#endif

        VkShaderStageFlags result = VkShaderStageFlags.None;
        if ((stage & ShaderStages.Vertex) != 0)
            result |= VK_SHADER_STAGE_VERTEX_BIT;

        if ((stage & ShaderStages.Fragment) != 0)
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;

        if ((stage & ShaderStages.Compute) != 0)
            result |= VK_SHADER_STAGE_COMPUTE_BIT;

        if ((stage & ShaderStages.Mesh) != 0)
            result |= VK_SHADER_STAGE_MESH_BIT_EXT;

        if ((stage & ShaderStages.Amplification) != 0)
            result |= VK_SHADER_STAGE_TASK_BIT_EXT;

        return result;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkPrimitiveTopology ToVk(this PrimitiveTopology type)
    {
        return type switch
        {
            PrimitiveTopology.PointList => VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            PrimitiveTopology.LineList => VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            PrimitiveTopology.LineStrip => VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
            PrimitiveTopology.TriangleStrip => VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            _ => VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkCompareOp ToVk(this CompareFunction value)
    {
        return value switch
        {
            CompareFunction.Never => VK_COMPARE_OP_NEVER,
            CompareFunction.Less => VK_COMPARE_OP_LESS,
            CompareFunction.Equal => VK_COMPARE_OP_EQUAL,
            CompareFunction.LessEqual => VK_COMPARE_OP_LESS_OR_EQUAL,
            CompareFunction.Greater => VK_COMPARE_OP_GREATER,
            CompareFunction.NotEqual => VK_COMPARE_OP_NOT_EQUAL,
            CompareFunction.GreaterEqual => VK_COMPARE_OP_GREATER_OR_EQUAL,
            CompareFunction.Always => VK_COMPARE_OP_ALWAYS,
            _ => VK_COMPARE_OP_NEVER,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkVertexInputRate ToVk(this VertexStepMode value)
    {
        return value switch
        {
            VertexStepMode.Instance => VK_VERTEX_INPUT_RATE_INSTANCE,
            _ => VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkCullModeFlags ToVk(this CullMode value)
    {
        return value switch
        {
            CullMode.Front => VK_CULL_MODE_FRONT_BIT,
            CullMode.None => VK_CULL_MODE_NONE,
            _ => VK_CULL_MODE_BACK_BIT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFrontFace ToVk(this FrontFace value)
    {
        return value switch
        {
            FrontFace.Clockwise => VK_FRONT_FACE_CLOCKWISE,
            _ => VK_FRONT_FACE_COUNTER_CLOCKWISE,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkStencilOp ToVk(this StencilOperation value)
    {
        return value switch
        {
            StencilOperation.Keep => VK_STENCIL_OP_KEEP,
            StencilOperation.Zero => VK_STENCIL_OP_ZERO,
            StencilOperation.Replace => VK_STENCIL_OP_REPLACE,
            StencilOperation.Invert => VK_STENCIL_OP_INVERT,
            StencilOperation.IncrementClamp => VK_STENCIL_OP_INCREMENT_AND_CLAMP,
            StencilOperation.DecrementClamp => VK_STENCIL_OP_DECREMENT_AND_CLAMP,
            StencilOperation.IncrementWrap => VK_STENCIL_OP_INCREMENT_AND_WRAP,
            StencilOperation.DecrementWrap => VK_STENCIL_OP_DECREMENT_AND_WRAP,
            _ => VK_STENCIL_OP_KEEP,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBlendFactor ToVk(this BlendFactor value)
    {
        return value switch
        {
            BlendFactor.Zero => VK_BLEND_FACTOR_ZERO,
            BlendFactor.One => VK_BLEND_FACTOR_ONE,
            BlendFactor.SourceColor => VK_BLEND_FACTOR_SRC_COLOR,
            BlendFactor.OneMinusSourceColor => VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
            BlendFactor.SourceAlpha => VK_BLEND_FACTOR_SRC_ALPHA,
            BlendFactor.OneMinusSourceAlpha => VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            BlendFactor.DestinationColor => VK_BLEND_FACTOR_DST_COLOR,
            BlendFactor.OneMinusDestinationColor => VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
            BlendFactor.DestinationAlpha => VK_BLEND_FACTOR_DST_ALPHA,
            BlendFactor.OneMinusDestinationAlpha => VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
            BlendFactor.SourceAlphaSaturate => VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
            BlendFactor.BlendColor => VK_BLEND_FACTOR_CONSTANT_COLOR,
            BlendFactor.OneMinusBlendColor => VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
            BlendFactor.Source1Color => VK_BLEND_FACTOR_SRC1_COLOR,
            BlendFactor.OneMinusSource1Color => VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
            BlendFactor.Source1Alpha => VK_BLEND_FACTOR_SRC1_ALPHA,
            BlendFactor.OneMinusSource1Alpha => VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
            _ => VK_BLEND_FACTOR_ZERO,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBlendOp ToVk(this BlendOperation value)
    {
        return value switch
        {
            BlendOperation.Subtract => VK_BLEND_OP_SUBTRACT,
            BlendOperation.ReverseSubtract => VK_BLEND_OP_REVERSE_SUBTRACT,
            BlendOperation.Min => VK_BLEND_OP_MIN,
            BlendOperation.Max => VK_BLEND_OP_MAX,
            _ => VK_BLEND_OP_ADD,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkColorComponentFlags ToVk(this ColorWriteMask value)
    {
        VkColorComponentFlags result = 0;

        if ((value & ColorWriteMask.Red) != 0)
            result |= VK_COLOR_COMPONENT_R_BIT;

        if ((value & ColorWriteMask.Green) != 0)
            result |= VK_COLOR_COMPONENT_G_BIT;

        if ((value & ColorWriteMask.Blue) != 0)
            result |= VK_COLOR_COMPONENT_B_BIT;

        if ((value & ColorWriteMask.Alpha) != 0)
            result |= VK_COLOR_COMPONENT_A_BIT;

        return result;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFilter ToVk(this SamplerMinMagFilter value)
    {
        return value switch
        {
            SamplerMinMagFilter.Nearest => VK_FILTER_NEAREST,
            SamplerMinMagFilter.Linear => VK_FILTER_LINEAR,
            _ => VK_FILTER_NEAREST,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkSamplerMipmapMode ToVk(this SamplerMipFilter value)
    {
        return value switch
        {
            SamplerMipFilter.Nearest => VK_SAMPLER_MIPMAP_MODE_NEAREST,
            SamplerMipFilter.Linear => VK_SAMPLER_MIPMAP_MODE_LINEAR,
            _ => VK_SAMPLER_MIPMAP_MODE_NEAREST,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkSamplerAddressMode ToVk(this SamplerAddressMode value, bool samplerMirrorClampToEdge)
    {
        return value switch
        {
            SamplerAddressMode.Repeat => VK_SAMPLER_ADDRESS_MODE_REPEAT,
            SamplerAddressMode.MirrorRepeat => VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            SamplerAddressMode.ClampToEdge => VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            SamplerAddressMode.ClampToBorder => VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            SamplerAddressMode.MirrorClampToEdge => samplerMirrorClampToEdge ? VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            _ => VK_SAMPLER_ADDRESS_MODE_REPEAT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBorderColor ToVk(this SamplerBorderColor value)
    {
        return value switch
        {
            SamplerBorderColor.FloatTransparentBlack => VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
            SamplerBorderColor.FloatOpaqueBlack => VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            SamplerBorderColor.FloatOpaqueWhite => VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            SamplerBorderColor.UIntTransparentBlack => VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
            SamplerBorderColor.UIntOpaqueBlack => VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            SamplerBorderColor.UIntOpaqueWhite => VK_BORDER_COLOR_INT_OPAQUE_WHITE,
            _ => VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkComponentSwizzle ToVk(this TextureSwizzle value)
    {
        return value switch
        {
            TextureSwizzle.Red => VK_COMPONENT_SWIZZLE_R,
            TextureSwizzle.Green => VK_COMPONENT_SWIZZLE_G,
            TextureSwizzle.Blue => VK_COMPONENT_SWIZZLE_B,
            TextureSwizzle.Alpha => VK_COMPONENT_SWIZZLE_A,
            TextureSwizzle.Zero => VK_COMPONENT_SWIZZLE_ZERO,
            TextureSwizzle.One => VK_COMPONENT_SWIZZLE_ONE,
            _ => VK_COMPONENT_SWIZZLE_IDENTITY,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkComponentMapping ToVk(this TextureSwizzleChannels value)
    {
        return new VkComponentMapping(value.Red.ToVk(), value.Green.ToVk(), value.Blue.ToVk(), value.Alpha.ToVk());
    }

    private static readonly VkBufferStateMapping[] s_bufferStateMap = [
        new(BufferStates.CopyDest, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT),
        new(BufferStates.CopySource, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT),
        new(BufferStates.ShaderResource, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT),
        new(BufferStates.UnorderedAccess, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT),
        new(BufferStates.VertexBuffer, VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT),
        new(BufferStates.IndexBuffer, VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT),
        new(BufferStates.ConstantBuffer, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_UNIFORM_READ_BIT),
        new(BufferStates.Predication, VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT),
#if TODO
            { ResourceStates::IndirectArgument, VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT },
            { ResourceStates::StreamOut, VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT },
            { ResourceStates::AccelerationStructureRead, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR },
            { ResourceStates::AccelerationStructureWrite, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR },
            { ResourceStates::AccelerationStructureBuildInput, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR},
            { ResourceStates::OpacityMicromapWrite, VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT },
            { ResourceStates::OpacityMicromapBuildInput, VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_SHADER_READ_BIT },
#endif // TODO        
    ];

    public static VkImageLayoutMapping ConvertImageLayout(TextureLayout layout, bool depthOnlyFormat)
    {
        return layout switch
        {
            TextureLayout.Undefined => new(
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                                0
                                ),
            TextureLayout.CopySource => new(
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                VK_ACCESS_2_TRANSFER_READ_BIT
                                ),
            TextureLayout.CopyDest => new(
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                VK_ACCESS_2_TRANSFER_WRITE_BIT
                                ),
            TextureLayout.ResolveSource => new(
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                VK_ACCESS_2_TRANSFER_READ_BIT
                                ),
            TextureLayout.ResolveDest => new(
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                VK_ACCESS_2_TRANSFER_WRITE_BIT
                                ),
            TextureLayout.ShaderResource => new(
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                                    VK_ACCESS_2_SHADER_READ_BIT
                                ),//return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT };
            TextureLayout.UnorderedAccess => new(
                                VK_IMAGE_LAYOUT_GENERAL,
                                    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                    VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT
                                ),
            TextureLayout.RenderTarget => new(
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
                                ),
            TextureLayout.DepthWrite => new(
                                depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                                ),
            TextureLayout.DepthRead => new(
                                depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                                ),
            TextureLayout.Present => new(
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                VK_ACCESS_2_MEMORY_READ_BIT
                                ),
            TextureLayout.ShadingRateSurface => new(
                                VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
                                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                    VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR
                                ),
            _ => ThrowHelper.ThrowArgumentException<VkImageLayoutMapping>(),
        };
    }

    public static VkBufferStateMapping ConvertBufferState(BufferStates state)
    {
        BufferStates resultState = BufferStates.Undefined;
        VkPipelineStageFlags2 resultStageFlags = VkPipelineStageFlags2.None;
        VkAccessFlags2 resultAccessMask = VkAccessFlags2.None;

        int numStateBits = s_bufferStateMap.Length;

        uint stateTmp = (uint)state;
        uint bitIndex = 0;

        while (stateTmp != 0 && bitIndex < numStateBits)
        {
            uint bit = (1u << (int)bitIndex);

            if ((stateTmp & bit) != 0)
            {
                ref VkBufferStateMapping mapping = ref s_bufferStateMap[bitIndex];

                Debug.Assert((uint)mapping.State == bit);

                resultState |= mapping.State;
                resultAccessMask |= mapping.AccessMask;
                resultStageFlags |= mapping.StageFlags;

                stateTmp &= ~bit;
            }

            bitIndex++;
        }

        Debug.Assert(resultState == state);

        return new(resultState, resultStageFlags, resultAccessMask);
    }

    public readonly record struct VkImageLayoutMapping(VkImageLayout Layout, VkPipelineStageFlags2 StageFlags, VkAccessFlags2 AccessMask);
    public readonly record struct VkBufferStateMapping(BufferStates State, VkPipelineStageFlags2 StageFlags, VkAccessFlags2 AccessMask);
}
