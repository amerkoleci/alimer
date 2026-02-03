// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Text;
using CommunityToolkit.Diagnostics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

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

    // Properties
    public readonly VkPhysicalDeviceProperties2 Properties2 = default;
    public readonly VkPhysicalDeviceVulkan11Properties Properties11 = default;
    public readonly VkPhysicalDeviceVulkan12Properties Properties12 = default;
    public readonly VkPhysicalDeviceVulkan13Properties Properties13 = default;
    public readonly VkPhysicalDeviceVulkan14Properties Properties14 = default;

    // Core in 1.3
    public VkPhysicalDeviceMaintenance4Features Maintenance4Features = default;
    public readonly VkPhysicalDeviceMaintenance4Properties Maintenance4Properties = default;
    public readonly VkPhysicalDeviceDynamicRenderingFeatures DynamicRenderingFeatures = default;
    public readonly VkPhysicalDeviceSynchronization2Features Synchronization2Features = default;
    public readonly VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ExtendedDynamicStateFeatures = default;
    public readonly VkPhysicalDeviceExtendedDynamicState2FeaturesEXT ExtendedDynamicState2Features = default;
    public readonly VkPhysicalDevicePipelineCreationCacheControlFeatures PipelineCreationCacheControlFeatures = default;
    public readonly VkPhysicalDeviceTextureCompressionASTCHDRFeatures ASTC_HDRFeatures = default;
    public readonly VkPhysicalDeviceTextureCompressionASTC3DFeaturesEXT ASTC_3DFeaturesEXT = default;

    // Core 1.4
    public readonly VkPhysicalDeviceMaintenance5Features Maintenance5Features = default;
    public readonly VkPhysicalDeviceMaintenance6Features Maintenance6Features = default;
    public readonly VkPhysicalDeviceMaintenance6Properties Maintenance6Properties = default;
    public readonly VkPhysicalDevicePushDescriptorProperties PushDescriptorProperties = default;

    // Extensions
    public readonly VkPhysicalDeviceSamplerFilterMinmaxProperties SamplerFilterMinmaxProperties = default;
    public readonly VkPhysicalDeviceDepthStencilResolveProperties DepthStencilResolveProperties = default;
    public readonly VkPhysicalDeviceDepthClipEnableFeaturesEXT DepthClipEnableFeatures = default;
    public readonly VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures = default;
    public readonly VkPhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructureProperties = default;
    public readonly VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures = default;
    public readonly VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPipelineProperties = default;
    public readonly VkPhysicalDeviceRayQueryFeaturesKHR RayQueryFeatures = default;
    public readonly VkPhysicalDeviceFragmentShadingRateFeaturesKHR FragmentShadingRateFeatures = default;
    public readonly VkPhysicalDeviceFragmentShadingRatePropertiesKHR FragmentShadingRateProperties = default;
    public readonly VkPhysicalDeviceMeshShaderFeaturesEXT MeshShaderFeatures = default;
    public readonly VkPhysicalDeviceMeshShaderPropertiesEXT MeshShaderProperties = default;
    public readonly VkPhysicalDeviceConditionalRenderingFeaturesEXT ConditionalRenderingFeatures = default;

    public VulkanGraphicsAdapter(VulkanGraphicsManager manager, in VkPhysicalDevice handle, in VulkanPhysicalDeviceExtensions extensions)
        : base(manager)
    {
        Handle = handle;
        Extensions = extensions;
        InstanceApi = manager.InstanceApi;

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
        VkPhysicalDeviceTextureCompressionASTC3DFeaturesEXT astc3DFeaturesEXT = default;

        // Core in 1.4
        VkPhysicalDeviceMaintenance5Features maintenance5Features = default;
        VkPhysicalDeviceMaintenance6Features maintenance6Features = default;
        VkPhysicalDeviceMaintenance6Properties maintenance6Properties = default;
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProperties = default;

        // Extensions
        VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = new();
        VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = new();
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = default;
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = default;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = default;
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = default;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = default;
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = default;
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = default;
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = default;
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = default;
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = default;
        VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = default;

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
                maintenance4Features = new();
                maintenance4Properties = new();
                AddToFeatureChain(&maintenance4Features);
                AddToPropertiesChain(&maintenance4Properties);
            }

            if (Extensions.DynamicRendering)
            {
                dynamicRenderingFeatures = new();
                AddToFeatureChain(&dynamicRenderingFeatures);
            }

            if (Extensions.Synchronization2)
            {
                synchronization2Features = new();
                AddToFeatureChain(&synchronization2Features);
            }

            if (Extensions.ExtendedDynamicState)
            {
                extendedDynamicStateFeatures = new();
                AddToFeatureChain(&extendedDynamicStateFeatures);
            }

            if (Extensions.ExtendedDynamicState2)
            {
                extendedDynamicState2Features = new();
                AddToFeatureChain(&extendedDynamicState2Features);
            }

            if (Extensions.PipelineCreationCacheControl)
            {
                pipelineCreationCacheControlFeatures = new();
                AddToFeatureChain(&pipelineCreationCacheControlFeatures);
            }

            if (Extensions.TextureCompressionASTC_HDR)
            {
                astcHdrFeatures = new();
                AddToFeatureChain(&astcHdrFeatures);
            }

            if (Extensions.TextureCompressionASTC_3D)
            {
                astc3DFeaturesEXT = new();
                AddToFeatureChain(&astc3DFeaturesEXT);
            }
        }
        else
        {
            // Core in 1.4
            if (properties2.properties.apiVersion < VK_API_VERSION_1_4)
            {
                if (Extensions.Maintenance5)
                {
                    maintenance5Features = new();
                    AddToFeatureChain(&maintenance5Features);
                }

                if (Extensions.Maintenance6)
                {
                    maintenance6Features = new();
                    maintenance6Properties = new();

                    AddToFeatureChain(&maintenance6Features);
                    AddToPropertiesChain(&maintenance6Properties);
                }

                if (Extensions.PushDescriptor)
                {
                    pushDescriptorProperties = new();
                    AddToPropertiesChain(&pushDescriptorProperties);
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

        if (Extensions.FragmentShadingRate)
        {
            fragmentShadingRateFeatures = new();
            fragmentShadingRateProperties = new();

            AddToFeatureChain(&fragmentShadingRateFeatures);
            AddToPropertiesChain(&fragmentShadingRateProperties);
        }

        if (Extensions.MeshShader)
        {
            meshShaderFeatures = new();
            meshShaderProperties = new();

            AddToFeatureChain(&meshShaderFeatures);
            AddToPropertiesChain(&meshShaderProperties);
        }

        if (Extensions.ConditionalRendering)
        {
            conditionalRenderingFeatures = new();
            AddToFeatureChain(&conditionalRenderingFeatures);
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
        FragmentShadingRateFeatures = fragmentShadingRateFeatures;
        MeshShaderFeatures = meshShaderFeatures;
        ConditionalRenderingFeatures = conditionalRenderingFeatures;
        // Core in 1.3
        DynamicRenderingFeatures = dynamicRenderingFeatures;
        Synchronization2Features = synchronization2Features;
        ExtendedDynamicStateFeatures = extendedDynamicStateFeatures;
        ExtendedDynamicState2Features = extendedDynamicState2Features;
        PipelineCreationCacheControlFeatures = pipelineCreationCacheControlFeatures;
        Maintenance4Features = maintenance4Features;
        ASTC_HDRFeatures = astcHdrFeatures;
        ASTC_3DFeaturesEXT = astc3DFeaturesEXT;

        // Core 1.4
        Maintenance5Features = maintenance5Features;
        Maintenance6Features = maintenance6Features;

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
        FragmentShadingRateProperties = fragmentShadingRateProperties;
        MeshShaderProperties = meshShaderProperties;
        // Core in 1.3
        Maintenance4Properties = maintenance4Properties;
        // Core 1.4
        Maintenance6Properties = maintenance6Properties;
        PushDescriptorProperties = pushDescriptorProperties;

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
    public VkInstanceApi InstanceApi { get; }

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
