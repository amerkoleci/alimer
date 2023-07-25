// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using WebGPU;
using static WebGPU.WebGPU;
using static Alimer.Graphics.WebGPU.WebGPUUtils;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.WebGPU;

internal unsafe partial class WebGPUGraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly WebGPUCommandQueue[] _queues = new WebGPUCommandQueue[(int)QueueType.Count];

    private readonly GraphicsAdapterProperties _adapterProperties;
    private readonly GraphicsDeviceLimits _limits;

    public static bool IsSupported() => s_isSupported.Value;

    public WebGPUGraphicsDevice(in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.WebGPU, description)
    {
        Guard.IsTrue(IsSupported(), nameof(WebGPUGraphicsDevice), "Vulkan is not supported");

        WGPUInstanceDescriptor instanceDescriptor = new()
        {
            nextInChain = null
        };
        Instance = wgpuCreateInstance(&instanceDescriptor);

        WGPURequestAdapterOptions options = new()
        {
            nextInChain = null,
            //compatibleSurface = Surface,
            powerPreference = WGPUPowerPreference.HighPerformance
        };

        // Call to the WebGPU request adapter procedure
        wgpuInstanceRequestAdapter(
            Instance /* equivalent of navigator.gpu */,
            &options,
            OnAdapterRequestEnded,
            IntPtr.Zero
        );

        fixed (sbyte* pLabel = description.Label.GetUtf8Span())
        {
            WGPUDeviceDescriptor deviceDesc = new()
            {
                nextInChain = null,
                label = pLabel,
                requiredFeaturesCount = 0,
                requiredLimits = null
            };
            deviceDesc.defaultQueue.nextInChain = null;
            //deviceDesc.defaultQueue.label = "The default queue";

            wgpuAdapterRequestDevice(
                Adapter,
                &deviceDesc,
                OnDeviceRequestEnded,
                IntPtr.Zero
            );
        }

        wgpuDeviceSetUncapturedErrorCallback(Handle, HandleUncapturedErrorCallback);

        // Queues
        for (int i = 0; i < (int)QueueType.VideoDecode; i++)
        {
            _queues[i] = new WebGPUCommandQueue(this, (QueueType)i);
        }

        // Init adapter information
        GpuAdapterType adapterType = GpuAdapterType.Other;
        adapterType = AdapterProperties.adapterType switch
        {
            WGPUAdapterType.IntegratedGPU => GpuAdapterType.IntegratedGpu,
            WGPUAdapterType.DiscreteGPU => GpuAdapterType.DiscreteGpu,
            WGPUAdapterType.CPU => GpuAdapterType.Cpu,
            _ => GpuAdapterType.Other,
        };

        _adapterProperties = new()
        {
            VendorId = AdapterProperties.vendorID,
            DeviceId = AdapterProperties.deviceID,
            AdapterName = new string(AdapterProperties.name),
            DriverDescription = new string(AdapterProperties.driverDescription),
            AdapterType = adapterType,
        };

        //TimestampFrequency = (ulong)(1.0 / _properties2.properties.limits.timestampPeriod * 1000 * 1000 * 1000);

        //_limits = new GraphicsDeviceLimits
        //{
        //    MaxTextureDimension1D = _properties2.properties.limits.maxImageDimension1D,
        //    MaxTextureDimension2D = _properties2.properties.limits.maxImageDimension2D,
        //    MaxTextureDimension3D = _properties2.properties.limits.maxImageDimension3D,
        //    MaxTextureDimensionCube = _properties2.properties.limits.maxImageDimensionCube,
        //    MaxTextureArrayLayers = _properties2.properties.limits.maxImageArrayLayers,
        //    MaxTexelBufferDimension2D = _properties2.properties.limits.maxTexelBufferElements,

        //    UploadBufferTextureRowAlignment = 1,
        //    UploadBufferTextureSliceAlignment = 1,
        //    ConstantBufferMinOffsetAlignment = (uint)_properties2.properties.limits.minUniformBufferOffsetAlignment,
        //    ConstantBufferMaxRange = _properties2.properties.limits.maxUniformBufferRange,
        //    StorageBufferMinOffsetAlignment = (uint)_properties2.properties.limits.minStorageBufferOffsetAlignment,
        //    StorageBufferMaxRange = _properties2.properties.limits.maxStorageBufferRange,

        //    MaxBufferSize = ulong.MaxValue,
        //    MaxPushConstantsSize = _properties2.properties.limits.maxPushConstantsSize,

        //    MaxVertexBuffers = _properties2.properties.limits.maxVertexInputBindings,
        //    MaxVertexAttributes = _properties2.properties.limits.maxVertexInputAttributes,
        //    MaxVertexBufferArrayStride = Math.Min(_properties2.properties.limits.maxVertexInputBindingStride, _properties2.properties.limits.maxVertexInputAttributeOffset + 1),

        //    MaxViewports = _properties2.properties.limits.maxViewports,
        //    MaxColorAttachments = _properties2.properties.limits.maxColorAttachments,

        //    MaxComputeWorkgroupStorageSize = _properties2.properties.limits.maxComputeSharedMemorySize,
        //    MaxComputeInvocationsPerWorkGroup = _properties2.properties.limits.maxComputeWorkGroupInvocations,
        //    MaxComputeWorkGroupSizeX = _properties2.properties.limits.maxComputeWorkGroupSize[0],
        //    MaxComputeWorkGroupSizeY = _properties2.properties.limits.maxComputeWorkGroupSize[1],
        //    MaxComputeWorkGroupSizeZ = _properties2.properties.limits.maxComputeWorkGroupSize[2],

        //    MaxComputeWorkGroupsPerDimension = Math.Min(_properties2.properties.limits.maxComputeWorkGroupCount[0], Math.Min(_properties2.properties.limits.maxComputeWorkGroupCount[1], _properties2.properties.limits.maxComputeWorkGroupCount[2])),

        //    SamplerMaxAnisotropy = (ushort)PhysicalDeviceProperties.properties.limits.maxSamplerAnisotropy,
        //    SamplerMinLodBias = -PhysicalDeviceProperties.properties.limits.maxSamplerLodBias,
        //    SamplerMaxLodBias = PhysicalDeviceProperties.properties.limits.maxSamplerLodBias,
        //};

        //if (fragmentShadingRateFeatures.attachmentFragmentShadingRate)
        //{
        //    _limits.VariableRateShadingTileSize = Math.Min(fragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.width, fragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.height);
        //}

        //if (QueryFeatureSupport(Feature.RayTracing))
        //{
        //    _limits.RayTracingShaderGroupIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
        //    _limits.RayTracingShaderTableAligment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
        //    _limits.RayTracingShaderTableMaxStride = rayTracingPipelineProperties.maxShaderGroupStride;
        //    _limits.RayTracingShaderRecursionMaxDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
        //    _limits.RayTracingMaxGeometryCount = (uint)accelerationStructureProperties.maxGeometryCount;
        //}

        void OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter candidateAdapter, sbyte* message, nint pUserData)
        {
            if (status == WGPURequestAdapterStatus.Success)
            {
                Adapter = candidateAdapter;
                WGPUAdapterProperties properties;
                wgpuAdapterGetProperties(candidateAdapter, &properties);

                WGPUSupportedLimits limits;
                wgpuAdapterGetLimits(candidateAdapter, &limits);

                AdapterProperties = properties;
                AdapterLimits = limits;
            }
            else
            {
                Log.Error("Could not get WebGPU adapter: " + Interop.GetString(message));
            }
        }

        void OnDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, sbyte* message, nint pUserData)
        {
            if (status == WGPURequestDeviceStatus.Success)
            {
                Handle = device;
            }
            else
            {
                Log.Error("Could not get WebGPU device: " + Interop.GetString(message));
            }
        }
    }

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => _adapterProperties;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public WGPUInstance Instance { get; }

    public WGPUAdapterProperties AdapterProperties;
    public WGPUSupportedLimits AdapterLimits;
    public WGPUAdapter Adapter { get; private set; }
    public WGPUDevice Handle { get; private set; }
    public WebGPUCommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public WebGPUCommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public WebGPUCommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUGraphicsDevice" /> class.
    /// </summary>
    ~WebGPUGraphicsDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            WaitIdle();
            _shuttingDown = true;

            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _queues[i].Dispose();
            }

            _frameCount = ulong.MaxValue;
            ProcessDeletionQueue();
            _frameCount = 0;
            _frameIndex = 0;

            if (Handle.IsNotNull)
            {
                wgpuDeviceRelease(Handle);
            }

            wgpuAdapterRelease(Adapter);
            wgpuInstanceRelease(Instance);
        }
    }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            case Feature.DepthClipControl:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.DepthClipControl);

            case Feature.Depth32FloatStencil8:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.Depth32FloatStencil8);

            case Feature.TimestampQuery:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.TimestampQuery);

            case Feature.PipelineStatisticsQuery:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.PipelineStatisticsQuery);

            case Feature.TextureCompressionBC:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.TextureCompressionBC);

            case Feature.TextureCompressionETC2:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.TextureCompressionETC2);

            case Feature.TextureCompressionASTC:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.TextureCompressionASTC);

            case Feature.IndirectFirstInstance:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.IndirectFirstInstance);

            case Feature.ShaderFloat16:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.ShaderF16);

            case Feature.RG11B10UfloatRenderable:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.RG11B10UfloatRenderable);

            case Feature.BGRA8UnormStorage:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.BGRA8UnormStorage);

            case Feature.TessellationShader:
            case Feature.DepthBoundsTest:
                return false;

            case Feature.SamplerAnisotropy:
                return true;

            case Feature.SamplerMinMax:
            case Feature.DepthResolveMinMax:
            case Feature.StencilResolveMinMax:
            case Feature.Predication:
            case Feature.DescriptorIndexing:
            case Feature.VariableRateShading:
            case Feature.VariableRateShadingTier2:
            case Feature.RayTracing:
            case Feature.RayTracingTier2:
            case Feature.MeshShader:
                return false;
        }

        return false;
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        wgpuDevicePoll(Handle, true, null);
    }

    /// <inheritdoc />
    public override void FinishFrame()
    {
        // Final submits with fences
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            //_queues[i].Submit(_queues[i].FrameFence);
        }

        AdvanceFrame();

        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].FinishFrame();
        }

        ProcessDeletionQueue();
    }

    public override void WriteShadingRateValue(ShadingRate rate, void* dest)
    {
        // How to compute shading rate value texel data:
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#primsrast-fragment-shading-rate-attachment

        switch (rate)
        {
            default:
            case ShadingRate.Rate1x1:
                *(byte*)dest = 0;
                break;
            case ShadingRate.Rate1x2:
                *(byte*)dest = 0x1;
                break;
            case ShadingRate.Rate2x1:
                *(byte*)dest = 0x4;
                break;
            case ShadingRate.Rate2x2:
                *(byte*)dest = 0x5;
                break;
            case ShadingRate.Rate2x4:
                *(byte*)dest = 0x6;
                break;
            case ShadingRate.Rate4x2:
                *(byte*)dest = 0x9;
                break;
            case ShadingRate.Rate4x4:
                *(byte*)dest = 0xa;
                break;
        }
    }

    /// <inheritdoc />
    protected override GraphicsBuffer CreateBufferCore(in BufferDescription descriptor, void* initialData)
    {
        return new WebGPUBuffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescription descriptor, void* initialData)
    {
        return new WebGPUTexture(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescription description)
    {
        return new WebGPUSampler(this, description);
    }

    /// <inheritdoc />
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description)
    {
        return new WebGPUBindGroupLayout(this, description);
    }

    /// <inheritdoc />
    protected override BindGroup CreateBindGroupCore(BindGroupLayout layout, in BindGroupDescription description)
    {
        return new WebGPUBindGroup(this, layout, description);
    }

    /// <inheritdoc />
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description)
    {
        return new WebGPUPipelineLayout(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateRenderPipelineCore(in RenderPipelineDescription description)
    {
        return new WebGPUPipeline(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        return new WebGPUPipeline(this, description);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
    {
        return new WebGPUQueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription descriptor)
    {
        return new WebGPUSwapChain(this, surface, descriptor);
    }

    /// <inheritdoc />
    public override RenderContext BeginRenderContext(string? label = null)
    {
        return _queues[(int)QueueType.Graphics].BeginCommandContext(label);
    }

    private static bool CheckIsSupported()
    {
        return true;
    }

    private static void HandleUncapturedErrorCallback(WGPUErrorType type, sbyte* pMessage, nint pUserData)
    {
        string? message = Interop.GetString(pMessage);
        Log.Error($"Uncaptured device error: type: {type} ({message})");
    }
}
