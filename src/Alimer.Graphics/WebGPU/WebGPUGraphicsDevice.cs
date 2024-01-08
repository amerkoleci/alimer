// Copyright (c) Amer Koleci and Contributors.
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
    private readonly Dictionary<SamplerDescriptor, WGPUSampler> _samplerCache = new();

    public static bool IsSupported() => s_isSupported.Value;

    public WebGPUGraphicsDevice(in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.WebGPU, description)
    {
        Guard.IsTrue(IsSupported(), nameof(WebGPUGraphicsDevice), "Vulkan is not supported");

        wgpuSetLogCallback(LogCallback);

        WGPUInstanceExtras extras = new()
        {
#if DEBUG
            flags = WGPUInstanceFlags.Validation
#endif
        };


        WGPUInstanceDescriptor instanceDescriptor = new()
        {
            nextInChain = (WGPUChainedStruct*)&extras
        };
        Instance = wgpuCreateInstance(&instanceDescriptor);

        WGPURequestAdapterOptions options = new()
        {
            nextInChain = null,
            //compatibleSurface = Surface,
            powerPreference = WGPUPowerPreference.HighPerformance
        };

        // Call to the WebGPU request adapter procedure
        WGPUAdapter result = WGPUAdapter.Null;
        wgpuInstanceRequestAdapter(
            Instance /* equivalent of navigator.gpu */,
            &options,
            &OnAdapterRequestEnded,
            new nint(&result)
        );
        Adapter = result;

        WGPUAdapterProperties properties;
        WGPUSupportedLimits supportedLimits;

        wgpuAdapterGetProperties(Adapter, &properties);
        wgpuAdapterGetLimits(Adapter, &supportedLimits);

        AdapterProperties = properties;
        AdapterLimits = supportedLimits;

        fixed (sbyte* pLabel = description.Label.GetUtf8Span())
        {
            WGPUFeatureName* requiredFeatures = stackalloc WGPUFeatureName[1]
            {
                WGPUFeatureName.Depth32FloatStencil8,
            };

            WGPURequiredLimits requiredLimits;
            requiredLimits.nextInChain = null;
            requiredLimits.limits = supportedLimits.limits;

            WGPUDeviceDescriptor deviceDesc = new()
            {
                nextInChain = null,
                label = pLabel,
                requiredFeatureCount = 1u,
                requiredFeatures = requiredFeatures,
                requiredLimits = &requiredLimits
            };
            deviceDesc.defaultQueue.nextInChain = null;
            //deviceDesc.defaultQueue.label = "The default queue";

            WGPUDevice device = WGPUDevice.Null;
            wgpuAdapterRequestDevice(
                Adapter,
                &deviceDesc,
                &OnDeviceRequestEnded,
                new nint(&device)
            );
            Handle = device;
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

        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = AdapterLimits.limits.maxTextureDimension1D,
            MaxTextureDimension2D = AdapterLimits.limits.maxTextureDimension2D,
            MaxTextureDimension3D = AdapterLimits.limits.maxTextureDimension3D,
            MaxTextureDimensionCube = 2048,
            MaxTextureArrayLayers = AdapterLimits.limits.maxTextureArrayLayers,
            MaxTexelBufferDimension2D = 1, //AdapterLimits.limits.maxTexelBufferElements,

            UploadBufferTextureRowAlignment = 1,
            UploadBufferTextureSliceAlignment = 1,
            MinConstantBufferOffsetAlignment = AdapterLimits.limits.minUniformBufferOffsetAlignment,
            MaxConstantBufferBindingSize = AdapterLimits.limits.maxUniformBufferBindingSize,
            MinStorageBufferOffsetAlignment = AdapterLimits.limits.minStorageBufferOffsetAlignment,
            MaxStorageBufferBindingSize = AdapterLimits.limits.maxStorageBufferBindingSize,

            MaxBufferSize = AdapterLimits.limits.maxBufferSize,
            MaxPushConstantsSize = 256,

            MaxVertexBuffers = AdapterLimits.limits.maxVertexBuffers,
            MaxVertexAttributes = AdapterLimits.limits.maxVertexAttributes,
            MaxVertexBufferArrayStride = AdapterLimits.limits.maxVertexBufferArrayStride,

            MaxViewports = 1u, //AdapterLimits.limits.maxViewports,
            MaxColorAttachments = Math.Max(8, AdapterLimits.limits.maxColorAttachments),

            MaxComputeWorkgroupStorageSize = AdapterLimits.limits.maxComputeWorkgroupStorageSize,
            MaxComputeInvocationsPerWorkGroup = AdapterLimits.limits.maxComputeInvocationsPerWorkgroup,
            MaxComputeWorkGroupSizeX = AdapterLimits.limits.maxComputeWorkgroupSizeX,
            MaxComputeWorkGroupSizeY = AdapterLimits.limits.maxComputeWorkgroupSizeY,
            MaxComputeWorkGroupSizeZ = AdapterLimits.limits.maxComputeWorkgroupSizeZ,
            MaxComputeWorkGroupsPerDimension = AdapterLimits.limits.maxComputeWorkgroupsPerDimension,

            SamplerMaxAnisotropy = 16,
            //SamplerMinLodBias = -PhysicalDeviceProperties.properties.limits.maxSamplerLodBias,
            //SamplerMaxLodBias = PhysicalDeviceProperties.properties.limits.maxSamplerLodBias,
        };
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

            foreach (WGPUSampler sampler in _samplerCache.Values)
            {
                wgpuSamplerRelease(sampler);
            }
            _samplerCache.Clear();


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

    [UnmanagedCallersOnly]
    private static void OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter candidateAdapter, sbyte* message, nint pUserData)
    {
        if (status == WGPURequestAdapterStatus.Success)
        {
            *(WGPUAdapter*)pUserData = candidateAdapter;
        }
        else
        {
            Log.Error("Could not get WebGPU adapter: " + Interop.GetString(message));
        }
    }

    [UnmanagedCallersOnly]
    private static void OnDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, sbyte* message, nint pUserData)
    {
        if (status == WGPURequestDeviceStatus.Success)
        {
            *(WGPUDevice*)pUserData = device;
        }
        else
        {
            Log.Error("Could not get WebGPU device: " + Interop.GetString(message));
        }
    }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            //case Feature.DepthClipControl:
            //    return wgpuDeviceHasFeature(Handle, WGPUFeatureName.DepthClipControl);

            case Feature.Depth32FloatStencil8:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.Depth32FloatStencil8);

            case Feature.TimestampQuery:
                return wgpuDeviceHasFeature(Handle, WGPUFeatureName.TimestampQuery);

            //case Feature.PipelineStatisticsQuery:
            //    return wgpuDeviceHasFeature(Handle, WGPUFeatureName.PipelineStatisticsQuery);

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

            case Feature.SamplerClampToBorder:
            case Feature.SamplerMirrorClampToEdge:
                return false;

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
    public override bool QueryPixelFormatSupport(PixelFormat format)
    {
        // TODO:
        return false;
    }

    /// <inheritdoc />
    public override bool QueryVertexFormatSupport(VertexFormat format)
    {
        // TODO:
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
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit();
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

    /// <inheritdoc />
    protected override GraphicsBuffer CreateBufferCore(in BufferDescription descriptor, void* initialData)
    {
        return new WebGPUBuffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData)
    {
        return new WebGPUTexture(this, descriptor, initialData);
    }

    public WGPUSampler GetOrCreateWGPUSampler(in SamplerDescriptor description)
    {
        if (!_samplerCache.TryGetValue(description, out WGPUSampler sampler))
        {
            fixed (sbyte* pLabel = description.Label.GetUtf8Span())
            {
                WGPUSamplerDescriptor descriptor = new()
                {
                    label = pLabel,
                    addressModeU = description.AddressModeU.ToWebGPU(),
                    addressModeV = description.AddressModeV.ToWebGPU(),
                    addressModeW = description.AddressModeW.ToWebGPU(),
                    magFilter = description.MagFilter.ToWebGPU(),
                    minFilter = description.MinFilter.ToWebGPU(),
                    mipmapFilter = description.MipFilter.ToWebGPU(),
                    lodMinClamp = description.LodMinClamp,
                    lodMaxClamp = description.LodMaxClamp,
                    compare = description.CompareFunction.ToWebGPU(),
                    maxAnisotropy = description.MaxAnisotropy
                };

                sampler = wgpuDeviceCreateSampler(Handle, &descriptor);

                if (sampler.IsNull)
                {
                    Log.Error("WebGPU: Failed to create sampler.");
                    return WGPUSampler.Null;
                }
            }

            _samplerCache.Add(description, sampler);
        }

        return sampler;
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescriptor descriptor)
    {
        return new WebGPUSampler(this, in descriptor);
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
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor)
    {
        return new WebGPUQueryHeap(this, descriptor);
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

    private static void LogCallback(WGPULogLevel level, string message, nint userdata = 0)
    {
        switch (level)
        {
            case WGPULogLevel.Error:
                throw new GraphicsException(message);
            case WGPULogLevel.Warn:
                //Log.Warn(message);
                break;
            case WGPULogLevel.Info:
            case WGPULogLevel.Debug:
            case WGPULogLevel.Trace:
                //Log.Info(message);
                break;
        }
    }

    private static void HandleUncapturedErrorCallback(WGPUErrorType type, string message)
    {
        Log.Error($"Uncaptured device error: type: {type} ({message})");
    }
}
