// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Text;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanGraphicsAdapter : GraphicsAdapter
{
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

    // Core 1.4
    public readonly VkPhysicalDeviceMaintenance5Features Maintenance5Features = default;
    public readonly VkPhysicalDeviceMaintenance6Features Maintenance6Features = default;
    public readonly VkPhysicalDeviceMaintenance6Properties Maintenance6Properties = default;
    public readonly VkPhysicalDevicePushDescriptorProperties PushDescriptorProperties = default;

    public readonly VkPhysicalDeviceMemoryProperties2 MemoryProperties = default;

    // Extensions
    public readonly VkPhysicalDeviceSamplerFilterMinmaxProperties SamplerFilterMinmaxProperties = default;
    public readonly VkPhysicalDeviceDepthStencilResolveProperties DepthStencilResolveProperties = default;
    public readonly VkPhysicalDeviceDepthClipEnableFeaturesEXT DepthClipEnableFeatures = default;
    public readonly VkPhysicalDeviceConservativeRasterizationPropertiesEXT ConservativeRasterizationProperties = default;
    public readonly VkPhysicalDeviceSampleLocationsPropertiesEXT SampleLocationsProperties = default;
    public readonly VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures = default;
    public readonly VkPhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructureProperties = default;
    public readonly VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures = default;
    public readonly VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPipelineProperties = default;
    public readonly VkPhysicalDeviceRayQueryFeaturesKHR RayQueryFeatures = default;
    public readonly VkPhysicalDeviceFragmentShadingRateFeaturesKHR FragmentShadingRateFeatures = default;
    public readonly VkPhysicalDeviceFragmentShadingRatePropertiesKHR FragmentShadingRateProperties = default;
    public readonly VkPhysicalDeviceMeshShaderFeaturesEXT MeshShaderFeatures = default;
    public readonly VkPhysicalDeviceMeshShaderPropertiesEXT MeshShaderProperties = default;
    public readonly VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR UnifiedImageLayoutsFeatures = default;
    public readonly VkPhysicalDeviceDescriptorHeapFeaturesEXT DescriptorHeapFeaturesEXT = default;
    public readonly VkPhysicalDeviceDescriptorHeapPropertiesEXT DescriptorHeapPropertiesEXT = default;
    private readonly GraphicsMemoryHeapInfo[] _memoryHeaps = [];

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
        VkPhysicalDeviceVulkan13Features features13 = new();
        VkPhysicalDeviceVulkan14Features features14 = default;
        VkPhysicalDeviceVulkan11Properties properties11 = new();
        VkPhysicalDeviceVulkan12Properties properties12 = new();
        VkPhysicalDeviceVulkan13Properties properties13 = new();
        VkPhysicalDeviceVulkan14Properties properties14 = default;

        // Core in 1.4
        VkPhysicalDeviceMaintenance5Features maintenance5Features = default;
        VkPhysicalDeviceMaintenance6Features maintenance6Features = default;
        VkPhysicalDeviceMaintenance6Properties maintenance6Properties = default;
        VkPhysicalDevicePushDescriptorProperties pushDescriptorProperties = default;

        // Extensions
        VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = new();
        VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = new();
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = default;
        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProperties = default;
        VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProperties = default;
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = default;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = default;
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = default;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = default;
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = default;
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = default;
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = default;
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = default;
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = default;
        VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeatures = default;

        VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeaturesEXT = default;
        VkPhysicalDeviceDescriptorHeapPropertiesEXT descriptorHeapPropertiesEXT = default;

        // Setup pNext chains
        VkBaseOutStructure* featureChainCurrent = (VkBaseOutStructure*)&features2;
        VkBaseOutStructure* propertiesChainCurrent = (VkBaseOutStructure*)&properties2;

        AddToFeatureChain(&features11);
        AddToFeatureChain(&features12);
        AddToFeatureChain(&features13);
        AddToPropertiesChain(&properties11);
        AddToPropertiesChain(&properties12);
        AddToPropertiesChain(&properties13);

        if (properties2.properties.apiVersion >= VK_API_VERSION_1_4)
        {
            features14 = new();
            properties14 = new();
            AddToFeatureChain(&features14);
            AddToPropertiesChain(&properties14);
        }
        else
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

        AddToPropertiesChain(&samplerFilterMinmaxProperties);
        AddToPropertiesChain(&depthStencilResolveProperties);

        if (Extensions.DepthClipEnable)
        {
            depthClipEnableFeatures = new();
            AddToFeatureChain(&depthClipEnableFeatures);
        }

        if (extensions.ConservativeRasterization)
        {
            conservativeRasterizationProperties = new();
            AddToPropertiesChain(&conservativeRasterizationProperties);
        }

        if (extensions.SampleLocations)
        {
            sampleLocationsProperties = new();
            AddToPropertiesChain(&sampleLocationsProperties);
        }

        if (Extensions.AccelerationStructure)
        {
            Debug.Assert(Extensions.DeferredHostOperations);

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

        if (Extensions.UnifiedImageLayouts)
        {
            unifiedImageLayoutsFeatures = new();
            AddToFeatureChain(&unifiedImageLayoutsFeatures);
        }

        if (Extensions.DescriptorHeap)
        {
            descriptorHeapFeaturesEXT = new();
            AddToFeatureChain(&descriptorHeapFeaturesEXT);

            descriptorHeapPropertiesEXT = new();
            AddToPropertiesChain(&descriptorHeapPropertiesEXT);
        }

        manager.InstanceApi.vkGetPhysicalDeviceFeatures2(handle, &features2);

        if (!features2.features.textureCompressionBC &&
            !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
        {
            throw new GraphicsException("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
        }


        VkPhysicalDeviceMemoryProperties2 memoryProperties2 = new();
        manager.InstanceApi.vkGetPhysicalDeviceMemoryProperties2(handle, &memoryProperties2);
        MemoryProperties = memoryProperties2;

        Debug.Assert(features2.features.robustBufferAccess);
        Debug.Assert(features2.features.fullDrawIndexUint32);
        Debug.Assert(features2.features.depthClamp);
        Debug.Assert(features2.features.depthBiasClamp);
        Debug.Assert(features2.features.fragmentStoresAndAtomics);
        Debug.Assert(features2.features.imageCubeArray);
        Debug.Assert(features2.features.independentBlend);
        Debug.Assert(features2.features.sampleRateShading);
        Debug.Assert(features2.features.shaderClipDistance);
        Debug.Assert(features2.features.occlusionQueryPrecise);
        Debug.Assert(features12.timelineSemaphore);

        // Bindless (https://github.com/gfx-rs/wgpu/blob/trunk/wgpu-hal/src/vulkan/adapter.rs)
        Debug.Assert(features12.descriptorIndexing);
        Debug.Assert(features12.runtimeDescriptorArray);
        Debug.Assert(features12.descriptorBindingPartiallyBound);
        Debug.Assert(features12.shaderSampledImageArrayNonUniformIndexing);
        Debug.Assert(features12.descriptorBindingVariableDescriptorCount);

        // Synchronization2 and DynamicRendering are required by the engine
        Debug.Assert(features13.synchronization2);
        Debug.Assert(features13.dynamicRendering);

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
        UnifiedImageLayoutsFeatures = unifiedImageLayoutsFeatures;
        DescriptorHeapFeaturesEXT = descriptorHeapFeaturesEXT;

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
        ConservativeRasterizationProperties = conservativeRasterizationProperties;
        SampleLocationsProperties = sampleLocationsProperties;
        AccelerationStructureProperties = accelerationStructureProperties;
        RayTracingPipelineProperties = rayTracingPipelineProperties;
        FragmentShadingRateProperties = fragmentShadingRateProperties;
        MeshShaderProperties = meshShaderProperties;
        DescriptorHeapPropertiesEXT = descriptorHeapPropertiesEXT;

        // Core 1.4
        Maintenance6Properties = maintenance6Properties;
        PushDescriptorProperties = pushDescriptorProperties;
        //PushDescriptor = features14.pushDescriptor || pushDescriptorProperties.maxPushDescriptors > 0;
        MaxPushDescriptors = Math.Max(Properties14.maxPushDescriptors, PushDescriptorProperties.maxPushDescriptors);

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

        if (properties11.deviceLUIDValid)
        {
            Debug.Assert(LUIDSize == VK_LUID_SIZE);
            DeviceLUID = *(ulong*)properties11.deviceLUID;
        }

        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
        _memoryHeaps = new GraphicsMemoryHeapInfo[memoryProperties2.memoryProperties.memoryHeapCount];
        for (int i = 0; i < MemoryProperties.memoryProperties.memoryHeapCount; i++)
        {
            _memoryHeaps[i] = new GraphicsMemoryHeapInfo()
            {
                Properties = 0,
                Size = MemoryProperties.memoryProperties.memoryHeaps[i].size
            };

            if ((MemoryProperties.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
            {
                _memoryHeaps[i].Properties |= GraphicsHeapProperty.DeviceLocal;
            }
        }

        for (int i = 0; i < MemoryProperties.memoryProperties.memoryTypeCount; i++)
        {
            VkMemoryType memoryType = MemoryProperties.memoryProperties.memoryTypes[i];
            if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
            {
                _memoryHeaps[memoryType.heapIndex].Properties |= GraphicsHeapProperty.HostVisible;
            }
            if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
            {
                _memoryHeaps[memoryType.heapIndex].Properties |= GraphicsHeapProperty.HostCoherent;
            }
            if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0)
            {
                _memoryHeaps[memoryType.heapIndex].Properties |= GraphicsHeapProperty.HostCached;
            }
            else
            {
                _memoryHeaps[memoryType.heapIndex].Properties |= GraphicsHeapProperty.HostUncached;
            }
        }

        if (Features12.descriptorIndexing &&
            Features12.runtimeDescriptorArray &&
            Features12.descriptorBindingPartiallyBound &&
            Features12.shaderSampledImageArrayNonUniformIndexing &&
            Features12.descriptorBindingVariableDescriptorCount &&
            Features12.shaderSampledImageArrayNonUniformIndexing &&
            Features12.shaderStorageBufferArrayNonUniformIndexing &&
            Features12.shaderStorageImageArrayNonUniformIndexing &&
            Features12.shaderUniformTexelBufferArrayNonUniformIndexing &&
            Features12.shaderStorageTexelBufferArrayNonUniformIndexing &&
            Features12.descriptorBindingSampledImageUpdateAfterBind &&
            Features12.descriptorBindingStorageImageUpdateAfterBind &&
            Features12.descriptorBindingStorageBufferUpdateAfterBind &&
            Features12.descriptorBindingUniformTexelBufferUpdateAfterBind &&
            Features12.descriptorBindingStorageTexelBufferUpdateAfterBind &&
            Features12.descriptorBindingUpdateUnusedWhilePending &&
            Features12.descriptorBindingPartiallyBound
        //&& extendedFeatures.mutableDescriptorTypeFeatures.mutableDescriptorType
        )
        {
            Bindless = true;
        }

        Debug.Assert(properties2.properties.limits.maxDescriptorSetUniformBuffersDynamic >= DynamicConstantBufferCount);

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

    /// <inheritdoc />
    public override string DeviceName { get; }

    /// <inheritdoc />
    public override uint VendorId { get; }

    /// <inheritdoc />
    public override uint DeviceId { get; }

    /// <inheritdoc />
    public override GraphicsAdapterType Type { get; }

    /// <inheritdoc />
    public override ulong DeviceLUID { get; }

    /// <inheritdoc />
    public override ReadOnlySpan<GraphicsMemoryHeapInfo> MemoryHeaps => _memoryHeaps;

    public bool Bindless { get; }
    public bool Maintenance5 => Features14.maintenance5 || Maintenance5Features.maintenance5;
    public bool Maintenance6 => Features14.maintenance6 || Maintenance6Features.maintenance6;
    public uint MaxPushDescriptors { get; }

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new VulkanGraphicsDevice(this, description);
}
