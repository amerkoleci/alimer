// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal static unsafe class VulkanUtils
{
    private static readonly VkImageType[] s_vkImageTypeMap = new VkImageType[(int)TextureDimension.Count] {
        VkImageType.Image1D,
        VkImageType.Image2D,
        VkImageType.Image3D,
    };

    #region Layers Methods
    public static bool ValidateLayers(List<string> required, VkLayerProperties* availableLayers, uint availableLayersCount)
    {
        foreach (string layer in required)
        {
            bool found = false;
            for (uint i = 0; i < availableLayersCount; i++)
            {
                string availableLayer = availableLayers[i].GetLayerName();

                if (availableLayer == layer)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                //Log.Warn("Validation Layer '{}' not found", layer);
                return false;
            }
        }

        return true;
    }

    public static void GetOptimalValidationLayers(VkLayerProperties* availableLayers, uint availableLayersCount, List<string> instanceLayers)
    {
        // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
        List<string> validationLayers = new()
        {
            "VK_LAYER_KHRONOS_validation"
        };
        if (ValidateLayers(validationLayers, availableLayers, availableLayersCount))
        {
            instanceLayers.AddRange(validationLayers);
            return;
        }

        // Otherwise we fallback to using the LunarG meta layer
        validationLayers = new()
        {
            "VK_LAYER_LUNARG_standard_validation"
        };
        if (ValidateLayers(validationLayers, availableLayers, availableLayersCount))
        {
            instanceLayers.AddRange(validationLayers);
            return;
        }

        // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
        validationLayers = new()
        {
            "VK_LAYER_GOOGLE_threading",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_GOOGLE_unique_objects",
        };

        if (ValidateLayers(validationLayers, availableLayers, availableLayersCount))
        {
            instanceLayers.AddRange(validationLayers);
            return;
        }

        // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
        validationLayers = new()
        {
            "VK_LAYER_LUNARG_core_validation"
        };

        if (ValidateLayers(validationLayers, availableLayers, availableLayersCount))
        {
            instanceLayers.AddRange(validationLayers);
            return;
        }
    }
    #endregion

    public static VulkanPhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
    {
        uint count = 0;
        VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, null, &count, null);
        if (result != VkResult.Success)
            return default;

        VkExtensionProperties* vk_extensions = stackalloc VkExtensionProperties[(int)count];
        vkEnumerateDeviceExtensionProperties(physicalDevice, null, &count, vk_extensions);

        VulkanPhysicalDeviceExtensions extensions = new();

        for (int i = 0; i < count; ++i)
        {
            string extensionName = vk_extensions[i].GetExtensionName();

            if (extensionName == VK_KHR_MAINTENANCE_4_EXTENSION_NAME)
            {
                extensions.Maintenance4 = true;
            }
            else if (extensionName == VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
            {
                extensions.DynamicRendering = true;
            }
            else if (extensionName == VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
            {
                extensions.Synchronization2 = true;
            }
            else if (extensionName == VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
            {
                extensions.ExtendedDynamicState = true;
            }
            else if (extensionName == VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)
            {
                extensions.ExtendedDynamicState2 = true;
            }
            else if (extensionName == VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME)
            {
                extensions.PipelineCreationCacheControl = true;
            }
            else if (extensionName == VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)
            {
                extensions.FormatFeatureFlags2 = true;
            }
            else if (extensionName == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
            {
                extensions.Swapchain = true;
            }
            else if (extensionName == VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME)
            {
                extensions.DepthClipEnable = true;
            }
            else if (extensionName == VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)
            {
                extensions.MemoryBudget = true;
            }
            else if (extensionName == VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)
            {
                extensions.AMD_DeviceCoherentMemory = true;
            }
            else if (extensionName == VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)
            {
                extensions.MemoryPriority = true;
            }
            else if (extensionName == VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)
            {
                extensions.performance_query = true;
            }
            else if (extensionName == VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)
            {
                extensions.host_query_reset = true;
            }
            else if (extensionName == VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
            {
                extensions.deferred_host_operations = true;
            }
            else if (extensionName == VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
            {
                extensions.PortabilitySubset = true;
            }
            else if (extensionName == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
            {
                extensions.accelerationStructure = true;
            }
            else if (extensionName == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
            {
                extensions.raytracingPipeline = true;
            }
            else if (extensionName == VK_KHR_RAY_QUERY_EXTENSION_NAME)
            {
                extensions.rayQuery = true;
            }
            else if (extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
            {
                extensions.FragmentShadingRate = true;
            }
            else if (extensionName == VK_EXT_MESH_SHADER_EXTENSION_NAME)
            {
                extensions.MeshShader = true;
            }
            else if (extensionName == VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME)
            {
                extensions.ConditionalRendering = true;
            }
            else if (extensionName == VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)
            {
                extensions.Video.Queue = true;
            }
            else if (extensionName == VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)
            {
                extensions.Video.DecodeQueue = true;
            }
            else if (extensionName == VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME)
            {
                extensions.Video.DecodeH264 = true;
            }
            else if (extensionName == VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME)
            {
                extensions.Video.DecodeH265 = true;
            }
            else if (extensionName == VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)
            {
                extensions.Video.EncodeQueue = true;
            }
            else if (extensionName == VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME)
            {
                extensions.Video.EncodeH264 = true;
            }
            else if (extensionName == VK_EXT_VIDEO_ENCODE_H265_EXTENSION_NAME)
            {
                extensions.Video.EncodeH265 = true;
            }

            if (OperatingSystem.IsWindows())
            {
                if (extensionName == VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME)
                {
                    extensions.win32_full_screen_exclusive = true;
                }
                else if (extensionName == VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME)
                {
                    extensions.SupportsExternalSemaphore = true;
                }
                else if (extensionName == VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME)
                {
                    extensions.SupportsExternalMemory = true;
                }
            }
            else
            {
                if (extensionName == VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)
                {
                    extensions.SupportsExternalSemaphore = true;
                }
                else if (extensionName == VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME)
                {
                    extensions.SupportsExternalMemory = true;
                }
            }
        }

        VkPhysicalDeviceProperties gpuProps;
        vkGetPhysicalDeviceProperties(physicalDevice, &gpuProps);

        // Core 1.3
        if (gpuProps.apiVersion >= VkVersion.Version_1_3)
        {
            extensions.Maintenance4 = true;
            extensions.DynamicRendering = true;
            extensions.Synchronization2 = true;
            extensions.ExtendedDynamicState = true;
            extensions.ExtendedDynamicState2 = true;
            extensions.PipelineCreationCacheControl = true;
            extensions.FormatFeatureFlags2 = true;
        }

        return extensions;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static VkFormat ToVkFormat(PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case PixelFormat.R8Unorm: return VkFormat.R8Unorm;
            case PixelFormat.R8Snorm: return VkFormat.R8Snorm;
            case PixelFormat.R8Uint: return VkFormat.R8Uint;
            case PixelFormat.R8Sint: return VkFormat.R8Sint;
            // 16-bit formats
            case PixelFormat.R16Unorm: return VkFormat.R16Unorm;
            case PixelFormat.R16Snorm: return VkFormat.R16Snorm;
            case PixelFormat.R16Uint: return VkFormat.R16Uint;
            case PixelFormat.R16Sint: return VkFormat.R16Sint;
            case PixelFormat.R16Float: return VkFormat.R16Sfloat;
            case PixelFormat.RG8Unorm: return VkFormat.R8G8Unorm;
            case PixelFormat.RG8Snorm: return VkFormat.R8G8Snorm;
            case PixelFormat.RG8Uint: return VkFormat.R8G8Uint;
            case PixelFormat.RG8Sint: return VkFormat.R8G8Sint;
            // Packed 16-Bit Pixel Formats
            case PixelFormat.BGRA4Unorm: return VkFormat.B4G4R4A4UnormPack16;
            case PixelFormat.B5G6R5Unorm: return VkFormat.B5G6R5UnormPack16;
            case PixelFormat.BGR5A1Unorm: return VkFormat.B5G5R5A1UnormPack16;
            // 32-bit formats
            case PixelFormat.R32Uint: return VkFormat.R32Uint;
            case PixelFormat.R32Sint: return VkFormat.R32Sint;
            case PixelFormat.R32Float: return VkFormat.R32Sfloat;
            case PixelFormat.RG16Unorm: return VkFormat.R16G16Unorm;
            case PixelFormat.RG16Snorm: return VkFormat.R16G16Snorm;
            case PixelFormat.RG16Uint: return VkFormat.R16G16Uint;
            case PixelFormat.RG16Sint: return VkFormat.R16G16Sint;
            case PixelFormat.RG16Float: return VkFormat.R16G16Sfloat;
            case PixelFormat.RGBA8Unorm: return VkFormat.R8G8B8A8Unorm;
            case PixelFormat.RGBA8UnormSrgb: return VkFormat.R8G8B8A8Srgb;
            case PixelFormat.RGBA8Snorm: return VkFormat.R8G8B8A8Snorm;
            case PixelFormat.RGBA8Uint: return VkFormat.R8G8B8A8Uint;
            case PixelFormat.RGBA8Sint: return VkFormat.R8G8B8A8Sint;
            case PixelFormat.BGRA8Unorm: return VkFormat.B8G8R8A8Unorm;
            case PixelFormat.BGRA8UnormSrgb: return VkFormat.B8G8R8A8Srgb;
            // Packed 32-Bit formats
            case PixelFormat.RGB10A2Unorm: return VkFormat.A2B10G10R10UnormPack32;
            case PixelFormat.RGB10A2Uint: return VkFormat.A2B10G10R10UintPack32;
            case PixelFormat.RG11B10UFloat: return VkFormat.B10G11R11UfloatPack32;
            case PixelFormat.RGB9E5UFloat: return VkFormat.E5B9G9R9UfloatPack32;
            // 64-Bit formats
            case PixelFormat.RG32Uint: return VkFormat.R32G32Uint;
            case PixelFormat.RG32Sint: return VkFormat.R32G32Sint;
            case PixelFormat.RG32Float: return VkFormat.R32G32Sfloat;
            case PixelFormat.RGBA16Unorm: return VkFormat.R16G16B16A16Unorm;
            case PixelFormat.RGBA16Snorm: return VkFormat.R16G16B16A16Snorm;
            case PixelFormat.RGBA16Uint: return VkFormat.R16G16B16A16Uint;
            case PixelFormat.RGBA16Sint: return VkFormat.R16G16B16A16Sint;
            case PixelFormat.RGBA16Float: return VkFormat.R16G16B16A16Sfloat;
            // 128-Bit formats
            case PixelFormat.RGBA32Uint:
                return VkFormat.R32G32B32A32Uint;
            case PixelFormat.RGBA32Sint:
                return VkFormat.R32G32B32A32Sint;
            case PixelFormat.RGBA32Float:
                return VkFormat.R32G32B32A32Sfloat;

            // Depth-stencil formats
            //case PixelFormat.Stencil8:
            //    return VkFormat.S8Uint;

            case PixelFormat.Depth16Unorm:
                return VkFormat.D16Unorm;

            case PixelFormat.Depth24UnormStencil8:
                return VkFormat.D24UnormS8Uint;

            case PixelFormat.Depth32Float:
                return VkFormat.D32Sfloat;

            case PixelFormat.Depth32FloatStencil8:
                return VkFormat.D32SfloatS8Uint;

            // Compressed BC formats
            case PixelFormat.BC1RGBAUnorm: return VkFormat.Bc1RgbaUnormBlock;
            case PixelFormat.BC1RGBAUnormSrgb: return VkFormat.Bc1RgbaSrgbBlock;
            case PixelFormat.BC2RGBAUnorm: return VkFormat.Bc2UnormBlock;
            case PixelFormat.BC2RGBAUnormSrgb: return VkFormat.Bc2SrgbBlock;
            case PixelFormat.BC3RGBAUnorm: return VkFormat.Bc3UnormBlock;
            case PixelFormat.BC3RGBAUnormSrgb: return VkFormat.Bc3SrgbBlock;
            case PixelFormat.BC4RSnorm: return VkFormat.Bc4UnormBlock;
            case PixelFormat.BC4RUnorm: return VkFormat.Bc4SnormBlock;
            case PixelFormat.BC5RGUnorm: return VkFormat.Bc5UnormBlock;
            case PixelFormat.BC5RGSnorm: return VkFormat.Bc5SnormBlock;
            case PixelFormat.BC6HRGBUfloat: return VkFormat.Bc6hUfloatBlock;
            case PixelFormat.BC6HRGBFloat: return VkFormat.Bc6hSfloatBlock;
            case PixelFormat.BC7RGBAUnorm: return VkFormat.Bc7UnormBlock;
            case PixelFormat.BC7RGBAUnormSrgb: return VkFormat.Bc7SrgbBlock;

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

            case VertexFormat.Int1010102Normalized: return VkFormat.A2B10G10R10SnormPack32;
            case VertexFormat.UInt1010102Normalized: return VkFormat.A2B10G10R10UnormPack32;

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
    public static VkShaderStageFlags ToVk(this ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage.Vertex:
                return VkShaderStageFlags.Vertex;
            case ShaderStage.Hull:
                return VkShaderStageFlags.TessellationControl;
            case ShaderStage.Domain:
                return VkShaderStageFlags.TessellationEvaluation;
            case ShaderStage.Geometry:
                return VkShaderStageFlags.Geometry;
            case ShaderStage.Fragment:
                return VkShaderStageFlags.Fragment;
            case ShaderStage.Compute:
                return VkShaderStageFlags.Compute;
            case ShaderStage.Amplification:
                return VkShaderStageFlags.TaskEXT;
            case ShaderStage.Mesh:
                return VkShaderStageFlags.MeshEXT;
            default:
                return VkShaderStageFlags.All;
        }
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

    private static readonly ResourceStateMapping[] s_resourceStateMap = new ResourceStateMapping[] {
        new(ResourceStates.Common,
            VkPipelineStageFlags2.TopOfPipe,
            VkAccessFlags2.None,
            VkImageLayout.Undefined),
        new(ResourceStates.ConstantBuffer,
            VkPipelineStageFlags2.AllCommands,
            VkAccessFlags2.UniformRead,
            VkImageLayout.Undefined),
        new (ResourceStates.VertexBuffer,
            VkPipelineStageFlags2.VertexInput,
            VkAccessFlags2.VertexAttributeRead,
            VkImageLayout.Undefined),
        new (ResourceStates.IndexBuffer,
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
    };

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

    public static ResourceStateMappingLegacy ConvertResourceStateLegacy(ResourceStates state)
    {
        ResourceStateMapping mapping = ConvertResourceState(state);

        // It's safe to cast vk::AccessFlags2 -> vk::AccessFlags and vk::PipelineStageFlags2 -> vk::PipelineStageFlags (as long as the enum exist in both versions!),
        // synchronization2 spec says: "The new flags are identical to the old values within the 32-bit range, with new stages and bits beyond that."
        // The below stages are exclustive to synchronization2
        Debug.Assert((mapping.StageFlags & VkPipelineStageFlags2.MicromapBuildEXT) != VkPipelineStageFlags2.MicromapBuildEXT);
        Debug.Assert((mapping.AccessMask & VkAccessFlags2.MicromapWriteEXT) != VkAccessFlags2.MicromapWriteEXT);

        return new ResourceStateMappingLegacy(mapping.State, (VkPipelineStageFlags)mapping.StageFlags, (VkAccessFlags)mapping.AccessMask, mapping.ImageLayout);
    }

    public readonly record struct ResourceStateMapping(ResourceStates State, VkPipelineStageFlags2 StageFlags, VkAccessFlags2 AccessMask, VkImageLayout ImageLayout);
    public readonly record struct ResourceStateMappingLegacy(ResourceStates State, VkPipelineStageFlags StageFlags, VkAccessFlags AccessMask, VkImageLayout ImageLayout);
}
