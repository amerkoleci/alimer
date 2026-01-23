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
            case VertexFormat.Unorm10_10_10_2: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case VertexFormat.Unorm8x4BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
            //case VertexFormat.RG11B10Float:         return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            //case VertexFormat.RGB9E5Float:          return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

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
            TextureViewDimension.Texture2DArray => VK_IMAGE_VIEW_TYPE_2D_ARRAY,
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
            LoadAction.Clear => VkAttachmentLoadOp.Clear,
            LoadAction.Discard => VkAttachmentLoadOp.DontCare,
            _ => VkAttachmentLoadOp.Load,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkAttachmentStoreOp ToVk(this StoreAction value)
    {
        return value switch
        {
            StoreAction.Discard => VkAttachmentStoreOp.DontCare,
            _ => VkAttachmentStoreOp.Store,
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
            BlendFactor.OneMinusSourceColor => VkBlendFactor.OneMinusSrcColor,
            BlendFactor.SourceAlpha => VkBlendFactor.SrcAlpha,
            BlendFactor.OneMinusSourceAlpha => VkBlendFactor.OneMinusSrcAlpha,
            BlendFactor.DestinationColor => VkBlendFactor.DstColor,
            BlendFactor.OneMinusDestinationColor => VkBlendFactor.OneMinusDstColor,
            BlendFactor.DestinationAlpha => VkBlendFactor.DstAlpha,
            BlendFactor.OneMinusDestinationAlpha => VkBlendFactor.OneMinusDstAlpha,
            BlendFactor.SourceAlphaSaturate => VkBlendFactor.SrcAlphaSaturate,
            BlendFactor.BlendColor => VkBlendFactor.ConstantColor,
            BlendFactor.OneMinusBlendColor => VkBlendFactor.OneMinusConstantColor,
            BlendFactor.BlendAlpha => VkBlendFactor.ConstantAlpha,
            BlendFactor.OneMinusBlendAlpha => VkBlendFactor.OneMinusConstantAlpha,
            BlendFactor.Source1Color => VkBlendFactor.Src1Color,
            BlendFactor.OneMinusSource1Color => VkBlendFactor.OneMinusSrc1Color,
            BlendFactor.Source1Alpha => VkBlendFactor.Src1Alpha,
            BlendFactor.OneMinusSource1Alpha => VkBlendFactor.OneMinusSrc1Alpha,
            _ => VkBlendFactor.Zero,
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
            SamplerAddressMode.MirrorRepeat => VkSamplerAddressMode.MirroredRepeat,
            SamplerAddressMode.ClampToEdge => VkSamplerAddressMode.ClampToEdge,
            SamplerAddressMode.ClampToBorder => VkSamplerAddressMode.ClampToBorder,
            SamplerAddressMode.MirrorClampToEdge => samplerMirrorClampToEdge ? VkSamplerAddressMode.MirrorClampToEdge : VkSamplerAddressMode.MirroredRepeat,
            _ => VK_SAMPLER_ADDRESS_MODE_REPEAT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkBorderColor ToVk(this SamplerBorderColor value)
    {
        return value switch
        {
            SamplerBorderColor.FloatTransparentBlack => VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
            SamplerBorderColor.FloatOpaqueBlack => VkBorderColor.FloatOpaqueBlack,
            SamplerBorderColor.FloatOpaqueWhite => VkBorderColor.FloatOpaqueWhite,
            SamplerBorderColor.UintTransparentBlack => VkBorderColor.IntTransparentBlack,
            SamplerBorderColor.UintOpaqueBlack => VkBorderColor.IntOpaqueBlack,
            SamplerBorderColor.UintOpaqueWhite => VkBorderColor.IntOpaqueWhite,
            _ => VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        };
    }

    private static readonly VkBufferStateMapping[] s_BufferStateMap = [
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
        switch (layout)
        {
            case TextureLayout.Undefined:
                return new(
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                    0
                    );

            case TextureLayout.CopySource:
                return new(
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    VK_ACCESS_2_TRANSFER_READ_BIT
                    );

            case TextureLayout.CopyDest:
                return new(
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT
                    );

            case TextureLayout.ResolveSource:
                return new(
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    VK_ACCESS_2_TRANSFER_READ_BIT
                    );

            case TextureLayout.ResolveDest:
                return new(
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT
                    );

            case TextureLayout.ShaderResource:
                //return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT };
                return new(
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                        VK_ACCESS_2_SHADER_READ_BIT
                    );

            case TextureLayout.UnorderedAccess:
                return new(
                    VK_IMAGE_LAYOUT_GENERAL,
                        VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                        VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT
                    );

            case TextureLayout.RenderTarget:
                return new(
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
                    );

            case TextureLayout.DepthWrite:
                return new(
                    depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                    );

            case TextureLayout.DepthRead:
                return new(
                    depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                    );

            case TextureLayout.Present:
                return new(
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                    VK_ACCESS_2_MEMORY_READ_BIT
                    );

            case TextureLayout.ShadingRateSurface:
                return new(
                    VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
                        VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                        VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR
                    );

            default:
                return ThrowHelper.ThrowArgumentException<VkImageLayoutMapping>();
        }
    }

    public static VkBufferStateMapping ConvertBufferState(BufferStates state)
    {
        BufferStates resultState = BufferStates.Undefined;
        VkPipelineStageFlags2 resultStageFlags = VkPipelineStageFlags2.None;
        VkAccessFlags2 resultAccessMask = VkAccessFlags2.None;

        int numStateBits = s_BufferStateMap.Length;

        uint stateTmp = (uint)state;
        uint bitIndex = 0;

        while (stateTmp != 0 && bitIndex < numStateBits)
        {
            uint bit = (1u << (int)bitIndex);

            if ((stateTmp & bit) != 0)
            {
                ref VkBufferStateMapping mapping = ref s_BufferStateMap[bitIndex];

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
