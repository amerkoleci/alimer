// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using System.Diagnostics;
using static Alimer.Utilities.MemoryUtilities;
using static Alimer.Utilities.MarshalUtilities;
using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;
using System.Text;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanGraphicsAdapter : GraphicsAdapter
{
    private readonly GraphicsDeviceLimits _limits;

    // Features
    public readonly VkVersion ApiVersion;
    public readonly VkPhysicalDeviceFeatures2 Features2 = default;
    public readonly VkPhysicalDeviceVulkan11Features Features11 = default;
    public readonly VkPhysicalDeviceVulkan12Features Features12 = default;
    public readonly VkPhysicalDeviceVulkan13Features Features13 = default;
    public readonly VkPhysicalDeviceVulkan14Features Features14 = default;

#if TODO
    // Core 1.4
    public readonly VkPhysicalDeviceMaintenance6Features Maintenance6Features = default;
    public readonly VkPhysicalDeviceMaintenance6Properties Maintenance6Properties = default;
    public readonly VkPhysicalDevicePushDescriptorProperties PushDescriptorProps = default;

    // Core in 1.3
    public readonly VkPhysicalDeviceMaintenance4Features Maintenance4Features = default;
    public readonly VkPhysicalDeviceMaintenance4Properties Maintenance4Properties = default;
    public readonly VkPhysicalDeviceDynamicRenderingFeatures DynamicRenderingFeatures = default;
    public readonly VkPhysicalDeviceSynchronization2Features Synchronization2Features = default;
    public readonly VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ExtendedDynamicStateFeatures = default;
    public readonly VkPhysicalDeviceExtendedDynamicState2FeaturesEXT ExtendedDynamicState2Features = default;
    public readonly VkPhysicalDevicePipelineCreationCacheControlFeatures PipelineCreationCacheControlFeatures = default;

#endif
    // Extensions
    public readonly VkPhysicalDeviceSamplerFilterMinmaxProperties SamplerFilterMinmaxProperties = default;
    public readonly VkPhysicalDeviceDepthStencilResolveProperties DepthStencilResolveProperties = default;
    public readonly VkPhysicalDeviceDepthClipEnableFeaturesEXT DepthClipEnableFeatures = default;
    public readonly VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures = default;
    public readonly VkPhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructureProperties = default;
    public readonly VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures = default;
    public readonly VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPipelineProperties = default;
    public readonly VkPhysicalDeviceRayQueryFeaturesKHR RayQueryFeatures = default;

    // Properties
    public readonly VkPhysicalDeviceProperties2 Properties2 = default;
    public readonly VkPhysicalDeviceVulkan11Properties Properties11 = default;
    public readonly VkPhysicalDeviceVulkan12Properties Properties12 = default;
    public readonly VkPhysicalDeviceVulkan13Properties Properties13 = default;
    public readonly VkPhysicalDeviceVulkan14Properties Properties14 = default;

    public VulkanGraphicsAdapter(VulkanGraphicsManager manager, in VkPhysicalDevice handle, in VulkanPhysicalDeviceExtensions extensions)
        : base(manager)
    {
        Handle = handle;
        Extensions = extensions;

        VkPhysicalDeviceProperties2 properties2 = new();
        manager.InstanceApi.vkGetPhysicalDeviceProperties2(handle, &properties2);

        // Features and properties
        VkPhysicalDeviceFeatures2 features2 = new();
        VkPhysicalDeviceVulkan11Features features11 = new();
        VkPhysicalDeviceVulkan12Features features12 = new();
        VkPhysicalDeviceVulkan13Features features13 = default;
        VkPhysicalDeviceVulkan14Features features14 = default;
        VkPhysicalDeviceVulkan11Properties properties11 = new();
        VkPhysicalDeviceVulkan12Properties properties12 = new();
        VkPhysicalDeviceVulkan13Properties properties13 = default;
        VkPhysicalDeviceVulkan14Properties properties14 = default;

        // Core in 1.3
        VkPhysicalDeviceMaintenance4Features maintenance4Features = default;
        VkPhysicalDeviceMaintenance4Properties maintenance4Properties = default;
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = default;
        VkPhysicalDeviceSynchronization2Features synchronization2Features = default;
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = default;
        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = default;
        VkPhysicalDevicePipelineCreationCacheControlFeatures pipelineCreationCacheControlFeatures = default;
        VkPhysicalDeviceTextureCompressionASTCHDRFeatures astcHdrFeatures = default;

        // Core in 1.4
        VkPhysicalDeviceMaintenance5Features maintenance5Features = default;
        VkPhysicalDeviceMaintenance6Features maintenance6Features = default;
        VkPhysicalDeviceMaintenance6Properties maintenance6Properties = default;
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProps = default;

        // Extensions
        VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = new();
        VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = new();
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = default;
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = default;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = default;
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = default;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = default;
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = default;

        // Setup pNext chains
        VkBaseOutStructure* featureChainCurrent = (VkBaseOutStructure*)&features2;
        VkBaseOutStructure* propertiesChainCurrent = (VkBaseOutStructure*)&properties2;

        AddToFeatureChain(&features11);
        AddToFeatureChain(&features12);
        AddToPropertiesChain(&properties11);
        AddToPropertiesChain(&properties12);
        if (properties2.properties.apiVersion >= VkVersion.Version_1_3)
        {
            features13 = new();
            properties13 = new();
            AddToFeatureChain(&features13);
            AddToPropertiesChain(&properties13);
        }

        if (properties2.properties.apiVersion >= VkVersion.Version_1_4)
        {
            features14 = new();
            properties14 = new();
            AddToFeatureChain(&features14);
            AddToPropertiesChain(&properties14);
        }

        AddToPropertiesChain(&samplerFilterMinmaxProperties);
        AddToPropertiesChain(&depthStencilResolveProperties);

        if (Extensions.DepthClipEnable)
        {
            depthClipEnableFeatures = new();
            AddToFeatureChain(&depthClipEnableFeatures);
        }

        // Core in 1.3
        if (properties2.properties.apiVersion < VK_API_VERSION_1_3)
        {
            if (Extensions.Maintenance4)
            {
                //enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

                maintenance4Features = new();
                maintenance4Properties = new();
                AddToFeatureChain(&maintenance4Features);
                AddToPropertiesChain(&maintenance4Properties);
            }

            if (Extensions.DynamicRendering)
            {
                //enabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

                dynamicRenderingFeatures = new();
                AddToFeatureChain(&dynamicRenderingFeatures);
            }

            if (Extensions.Synchronization2)
            {
                //enabledDeviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

                synchronization2Features = new();
                AddToFeatureChain(&synchronization2Features);
            }

            if (Extensions.ExtendedDynamicState)
            {
                //enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

                extendedDynamicStateFeatures = new();
                AddToFeatureChain(&extendedDynamicStateFeatures);
            }

            if (Extensions.ExtendedDynamicState2)
            {
                //enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);

                extendedDynamicState2Features = new();
                AddToFeatureChain(&extendedDynamicState2Features);
            }

            if (Extensions.PipelineCreationCacheControl)
            {
                //enabledDeviceExtensions.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);

                pipelineCreationCacheControlFeatures = new();
                AddToFeatureChain(&pipelineCreationCacheControlFeatures);
            }

            if (Extensions.TextureCompressionAstcHdr)
            {
                //enabledDeviceExtensions.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);

                astcHdrFeatures = new();
                AddToFeatureChain(&astcHdrFeatures);
            }
        }
        else
        {
            // Core in 1.4
            if (properties2.properties.apiVersion < VK_API_VERSION_1_4)
            {
                if (Extensions.Maintenance5)
                {
                    //enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

                    maintenance5Features = new();
                    AddToFeatureChain(&maintenance5Features);
                }

                if (Extensions.Maintenance6)
                {
                    //enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);

                    maintenance6Features = new();
                    maintenance6Properties = new();

                    AddToFeatureChain(&maintenance6Features);
                    AddToPropertiesChain(&maintenance6Properties);
                }

                if (Extensions.PushDescriptor)
                {
                    //enabledDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

                    pushDescriptorProps = new();
                    AddToPropertiesChain(&pushDescriptorProps);
                }
            }
        }

        if (Extensions.AccelerationStructure)
        {
            Guard.IsTrue(Extensions.DeferredHostOperations);

            accelerationStructureFeatures = new();
            AddToFeatureChain(&accelerationStructureFeatures);

            accelerationStructureProperties = new();
            AddToPropertiesChain(&accelerationStructureProperties);

            if (Extensions.RaytracingPipeline)
            {
                // Required by VK_KHR_pipeline_library
                rayTracingPipelineFeatures = new();
                rayTracingPipelineProperties = new();

                AddToFeatureChain(&rayTracingPipelineFeatures);
                AddToPropertiesChain(&rayTracingPipelineProperties);
            }

            if (Extensions.RayQuery)
            {
                rayQueryFeatures = new();
                AddToFeatureChain(&rayQueryFeatures);
            }
        }

        manager.InstanceApi.vkGetPhysicalDeviceFeatures2(handle, &features2);

        if (!features2.features.textureCompressionBC &&
            !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
        {
            throw new GraphicsException("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
        }

        Guard.IsTrue(features2.features.robustBufferAccess);
        Guard.IsTrue(features2.features.fullDrawIndexUint32);
        Guard.IsTrue(features2.features.depthClamp);
        Guard.IsTrue(features2.features.depthBiasClamp);
        Guard.IsTrue(features2.features.fragmentStoresAndAtomics);
        Guard.IsTrue(features2.features.imageCubeArray);
        Guard.IsTrue(features2.features.independentBlend);
        Guard.IsTrue(features2.features.sampleRateShading);
        Guard.IsTrue(features2.features.shaderClipDistance);
        Guard.IsTrue(features2.features.occlusionQueryPrecise);

        // Bindless (https://github.com/gfx-rs/wgpu/blob/trunk/wgpu-hal/src/vulkan/adapter.rs)
        Guard.IsTrue(features12.descriptorIndexing);
        Guard.IsTrue(features12.runtimeDescriptorArray);
        Guard.IsTrue(features12.descriptorBindingPartiallyBound);
        Guard.IsTrue(features12.descriptorBindingVariableDescriptorCount);
        Guard.IsTrue(features12.shaderSampledImageArrayNonUniformIndexing);
        Guard.IsTrue(features12.timelineSemaphore);

        bool synchronization2 = features13.synchronization2 || synchronization2Features.synchronization2;
        bool dynamicRendering = features13.dynamicRendering || dynamicRenderingFeatures.dynamicRendering;

        Guard.IsTrue(synchronization2);
        Guard.IsTrue(dynamicRendering);

        Features2 = features2;
        Features11 = features11;
        Features12 = features12;
        Features13 = features13;
        Features14 = features14;
        SamplerFilterMinmaxProperties = samplerFilterMinmaxProperties;
        DepthStencilResolveProperties = depthStencilResolveProperties;
        DepthClipEnableFeatures = depthClipEnableFeatures;
        AccelerationStructureFeatures = accelerationStructureFeatures;
        RayTracingPipelineFeatures = rayTracingPipelineFeatures;
        RayQueryFeatures = rayQueryFeatures;

        // Properties
        manager.InstanceApi.vkGetPhysicalDeviceProperties2(handle, &properties2);
        ApiVersion = properties2.properties.apiVersion;
        Properties2 = properties2;
        Properties11 = properties11;
        Properties12 = properties12;
        Properties13 = properties13;
        Properties14 = properties14;
        AccelerationStructureProperties = accelerationStructureProperties;
        RayTracingPipelineProperties = rayTracingPipelineProperties;

        // 
        DeviceName = Encoding.UTF8.GetString(properties2.properties.deviceName, (int)VK_MAX_PHYSICAL_DEVICE_NAME_SIZE).TrimEnd('\0');
        VendorId = properties2.properties.vendorID;
        DeviceId = properties2.properties.deviceID;

        Type = properties2.properties.deviceType switch
        {
            VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU => GraphicsAdapterType.DiscreteGpu,
            VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU => GraphicsAdapterType.IntegratedGpu,
            VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU => GraphicsAdapterType.VirtualGpu,
            VK_PHYSICAL_DEVICE_TYPE_CPU => GraphicsAdapterType.Cpu,
            _ => GraphicsAdapterType.Other
        };

        SupportsD24S8 = IsDepthStencilFormatSupported(VkFormat.D24UnormS8Uint);
        SupportsD32S8 = IsDepthStencilFormatSupported(VkFormat.D32SfloatS8Uint);

        Debug.Assert(SupportsD24S8 || SupportsD32S8);

        void AddToFeatureChain(void* next)
        {
            VkBaseOutStructure* n = (VkBaseOutStructure*)next;
            featureChainCurrent->pNext = n;
            featureChainCurrent = n;
        }

        void AddToPropertiesChain(void* next)
        {
            VkBaseOutStructure* n = (VkBaseOutStructure*)next;
            propertiesChainCurrent->pNext = n;
            propertiesChainCurrent = n;
        }
    }

    public VkPhysicalDevice Handle { get; }
    public VulkanPhysicalDeviceExtensions Extensions { get; }
    public VulkanGraphicsManager VkGraphicsManager => (VulkanGraphicsManager)Manager;
    public bool SupportsD24S8 { get; }
    public bool SupportsD32S8 { get; }

    /// <inheritdoc />
    public override string DeviceName { get; }

    /// <inheritdoc />
    public override uint VendorId { get; }

    /// <inheritdoc />
    public override uint DeviceId { get; }

    /// <inheritdoc />
    public override GraphicsAdapterType Type { get; }

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            case Feature.Depth32FloatStencil8:
                return SupportsD32S8;

            case Feature.TimestampQuery:
                return Properties2.properties.limits.timestampComputeAndGraphics == true;

            case Feature.PipelineStatisticsQuery:
                return Features2.features.pipelineStatisticsQuery == true;

#if TODO
            case Feature.TextureCompressionBC:
                return PhysicalDeviceFeatures2.features.textureCompressionBC == true;

            case Feature.TextureCompressionETC2:
                return PhysicalDeviceFeatures2.features.textureCompressionETC2 == true;

            case Feature.TextureCompressionASTC:
                return PhysicalDeviceFeatures2.features.textureCompressionASTC_LDR == true;

            case Feature.TextureCompressionASTC_HDR:
                return _textureCompressionASTC_HDR;

            case Feature.IndirectFirstInstance:
                return PhysicalDeviceFeatures2.features.drawIndirectFirstInstance == true;

            case Feature.ShaderFloat16:
                // VK_KHR_16bit_storage core in 1.1
                // VK_KHR_shader_float16_int8 core in 1.2
                return true;

            case Feature.RG11B10UfloatRenderable:
                _instanceApi.vkGetPhysicalDeviceFormatProperties(PhysicalDevice, VkFormat.B10G11R11UfloatPack32, out VkFormatProperties rg11b10Properties);
                if ((rg11b10Properties.optimalTilingFeatures & (VkFormatFeatureFlags.ColorAttachment | VkFormatFeatureFlags.ColorAttachmentBlend)) != 0u)
                {
                    return true;
                }

                return false;

            case Feature.BGRA8UnormStorage:
                VkFormatProperties bgra8unormProperties;
                _instanceApi.vkGetPhysicalDeviceFormatProperties(PhysicalDevice, VkFormat.B8G8R8A8Unorm, &bgra8unormProperties);
                if ((bgra8unormProperties.optimalTilingFeatures & VkFormatFeatureFlags.StorageImage) != 0)
                {
                    return true;
                }
                return false;

            case Feature.TessellationShader:
                return PhysicalDeviceFeatures2.features.tessellationShader == true;

            case Feature.DepthBoundsTest:
                return PhysicalDeviceFeatures2.features.depthBounds == true;

            case Feature.SamplerClampToBorder:
                return true;

            case Feature.SamplerMirrorClampToEdge:
                return PhysicalDeviceFeatures1_2.samplerMirrorClampToEdge == true;

            case Feature.SamplerMinMax:
                return PhysicalDeviceFeatures1_2.samplerFilterMinmax == true;

            case Feature.DepthResolveMinMax:
                return DepthResolveMinMax;

            case Feature.StencilResolveMinMax:
                return StencilResolveMinMax;

            case Feature.CacheCoherentUMA:
                if (_memoryProperties2.memoryProperties.memoryHeapCount == 1u &&
                    _memoryProperties2.memoryProperties.memoryHeaps[0].flags.HasFlag(VkMemoryHeapFlags.DeviceLocal))
                {
                    return true;
                }

                return false;

            case Feature.Predication:
                return _conditionalRenderingFeatures.conditionalRendering;

            case Feature.DescriptorIndexing:
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.runtimeDescriptorArray);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.descriptorBindingPartiallyBound);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.descriptorBindingVariableDescriptorCount);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.shaderSampledImageArrayNonUniformIndexing);
                return PhysicalDeviceFeatures1_2.descriptorIndexing;

            case Feature.VariableRateShading:
                return _fragmentShadingRateFeatures.pipelineFragmentShadingRate;

            case Feature.VariableRateShadingTier2:
                return _fragmentShadingRateFeatures.attachmentFragmentShadingRate;

            case Feature.RayTracing:
                return PhysicalDeviceFeatures1_2.bufferDeviceAddress &&
                    _accelerationStructureFeatures.accelerationStructure &&
                    _rayTracingPipelineFeatures.rayTracingPipeline;

            case Feature.RayTracingTier2:
                return _raytracingQueryFeatures.rayQuery && QueryFeatureSupport(Feature.RayTracing);

            case Feature.MeshShader:
                return (_meshShaderFeatures.meshShader && _meshShaderFeatures.taskShader); 
#endif
            default:
                return false;
        }
    }

    /// <inheritdoc />
    public override bool QueryPixelFormatSupport(PixelFormat format)
    {
        // TODO:
        return false;
    }

#if TODO
    /// <inheritdoc />
    public override bool QueryVertexFormatSupport(VertexFormat format)
    {
        VkFormat vkFormat = format.ToVk();
        if (vkFormat == VkFormat.Undefined)
            return false;

        VkFormatProperties2 props = new();
        vkGetPhysicalDeviceFormatProperties2(PhysicalDevice, vkFormat, &props);

        // TODO:
        return false;
    } 
#endif

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new VulkanGraphicsDevice(this, description);

    public bool IsDepthStencilFormatSupported(VkFormat format)
    {
        Debug.Assert(format == VkFormat.D16UnormS8Uint || format == VkFormat.D24UnormS8Uint || format == VkFormat.D32SfloatS8Uint || format == VkFormat.S8Uint);
        VkGraphicsManager.InstanceApi.vkGetPhysicalDeviceFormatProperties(
            Handle,
            format,
            out VkFormatProperties properties
            );
        return (properties.optimalTilingFeatures & VkFormatFeatureFlags.DepthStencilAttachment) != 0;
    }

    public VkFormat ToVkFormat(PixelFormat format)
    {
        //if (format == PixelFormat.Stencil8 && !SupportsS8)
        //{
        //    return VkFormat.D24UnormS8Uint;
        //}

        if (format == PixelFormat.Depth24UnormStencil8 && !SupportsD24S8)
        {
            return VkFormat.D32SfloatS8Uint;
        }

        VkFormat vkFormat = VulkanUtils.ToVkFormat(format);
        return vkFormat;
    }
}
