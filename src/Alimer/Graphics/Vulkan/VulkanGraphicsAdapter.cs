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

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanGraphicsAdapter : GraphicsAdapter
{
    // Features
    public readonly VkPhysicalDeviceFeatures2 Features2 = default;
    public readonly VkPhysicalDeviceVulkan11Features Features11 = default;
    public readonly VkPhysicalDeviceVulkan12Features Features12 = default;
    public readonly VkPhysicalDeviceVulkan13Features Features13 = default;
    public readonly VkPhysicalDeviceVulkan14Features Features14 = default;

    // Properties
    public readonly VkPhysicalDeviceProperties2 Properties2 = default;

    public VulkanGraphicsAdapter(VulkanGraphicsManager manager, in VkPhysicalDevice handle)
        : base(manager)
    {
        Handle = handle;

        // We require minimum 1.2
        VkPhysicalDeviceProperties properties;
        manager.InstanceApi.vkGetPhysicalDeviceProperties(handle, &properties);
        if (properties.apiVersion < VkVersion.Version_1_2)
        {
            return;
        }

        Extensions = VulkanPhysicalDeviceExtensions.Query(manager.InstanceApi, handle);
        if (!Extensions.Swapchain)
        {
            return;
        }

        // Features
        VkPhysicalDeviceFeatures2 features2 = new();
        VkPhysicalDeviceVulkan11Features features11 = new();
        VkPhysicalDeviceVulkan12Features features12 = new();
        VkPhysicalDeviceVulkan13Features features13 = default;
        VkPhysicalDeviceVulkan14Features features14 = default;
        // Core 1.4
        VkPhysicalDeviceMaintenance6Features maintenance6Features = new();
        VkPhysicalDeviceMaintenance6Properties maintenance6Properties = new();
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProps = new();
        // Core in 1.3
        VkPhysicalDeviceMaintenance4Features maintenance4Features = new();
        VkPhysicalDeviceMaintenance4Properties maintenance4Properties = new();
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures  = new();
        VkPhysicalDeviceSynchronization2Features synchronization2Features = new();
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = new(); ;
        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = new();
        VkPhysicalDevicePipelineCreationCacheControlFeatures pipelineCreationCacheControlFeatures = new();

        VkPhysicalDeviceMaintenance5Features maintenance5Features = new();
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = new();
        VkPhysicalDevicePerformanceQueryFeaturesKHR performanceQueryFeatures = new();
        VkPhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures = new();
        VkPhysicalDeviceTextureCompressionASTCHDRFeatures astcHdrFeatures = new();
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = new();
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = new();
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = new();
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = new();
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = new();
        VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = new();

        VkBaseOutStructure* featureChainCurrent = (VkBaseOutStructure*)&features2;
        AddToFeatureChain(&features11);
        AddToFeatureChain(&features12);
        if (properties.apiVersion >= VkVersion.Version_1_3)
        {
            features13 = new();
            AddToFeatureChain(&features13);
        }

        if (properties.apiVersion >= VkVersion.Version_1_4)
        {
            features14 = new();
            AddToFeatureChain(&features14);
        }

        manager.InstanceApi.vkGetPhysicalDeviceFeatures2(handle, &features2);

        if (!features2.features.textureCompressionBC &&
            !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
        {
            throw new GraphicsException("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
        }

        Guard.IsTrue(features2.features.robustBufferAccess);
        Guard.IsTrue(features2.features.depthBiasClamp);
        Guard.IsTrue(features2.features.fragmentStoresAndAtomics);
        Guard.IsTrue(features2.features.imageCubeArray);
        Guard.IsTrue(features2.features.independentBlend);
        Guard.IsTrue(features2.features.fullDrawIndexUint32);
        Guard.IsTrue(features2.features.sampleRateShading);
        Guard.IsTrue(features2.features.shaderClipDistance);
        Guard.IsTrue(features2.features.depthClamp);

        //ALIMER_VERIFY(features2.features.occlusionQueryPrecise == VK_TRUE);
        Guard.IsTrue(features12.descriptorIndexing);
        Guard.IsTrue(features12.runtimeDescriptorArray);
        Guard.IsTrue(features12.descriptorBindingPartiallyBound);
        Guard.IsTrue(features12.descriptorBindingVariableDescriptorCount);
        Guard.IsTrue(features12.shaderSampledImageArrayNonUniformIndexing);
        Guard.IsTrue(features12.timelineSemaphore);
        //Guard.IsTrue(features_1_3.synchronization2 == VK_TRUE);
        //Guard.IsTrue(features_1_3.dynamicRendering == VK_TRUE);

        Features2 = features2;
        Features11 = features11;
        Features12 = features12;
        Features13 = features13;
        Features14 = features14;

        // Properties
        VkPhysicalDeviceProperties2 properties2 = new();
        VkPhysicalDeviceVulkan11Properties properties11 = new();
        VkPhysicalDeviceVulkan12Properties properties12 = new();
        VkPhysicalDeviceVulkan13Properties properties13 = default;
        VkPhysicalDeviceVulkan14Properties properties14 = default;

        VkBaseOutStructure* propertiesChainCurrent = (VkBaseOutStructure*)&properties2;
        AddToPropertiesChain(&properties11);
        AddToPropertiesChain(&properties12);

        if (properties.apiVersion >= VkVersion.Version_1_3)
        {
            properties13 = new();
            AddToPropertiesChain(&properties13);
        }

        if (properties.apiVersion >= VkVersion.Version_1_4)
        {
            properties14 = new();
            AddToPropertiesChain(&properties14);
        }

        manager.InstanceApi.vkGetPhysicalDeviceProperties2(handle, &properties2);

        Type = properties2.properties.deviceType switch
        {
            VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU => GraphicsAdapterType.DiscreteGpu,
            VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU => GraphicsAdapterType.IntegratedGpu,
            VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU => GraphicsAdapterType.VirtualGpu,
            VK_PHYSICAL_DEVICE_TYPE_CPU => GraphicsAdapterType.Cpu,
            _ => GraphicsAdapterType.Other
        };

        IsSuitable = true;

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
        ;
    }

    public VkPhysicalDevice Handle { get; }
    public bool IsSuitable { get; }
    public VulkanPhysicalDeviceExtensions Extensions { get; }
    public VulkanGraphicsManager VkGraphicsManager => (VulkanGraphicsManager)Manager;

    public override GraphicsAdapterProperties AdapterInfo => throw new NotImplementedException();

    public override GraphicsDeviceLimits Limits => throw new NotImplementedException();

    public override string DeviceName => throw new NotImplementedException();

    public override GraphicsAdapterType Type { get; }

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new VulkanGraphicsDevice(this, description);
}
