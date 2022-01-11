// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using Vortice.DXGI.Debug;
using static Vortice.Direct3D12.D3D12;
using static Vortice.DXGI.DXGI;

namespace Vortice.Graphics;

public unsafe class D3D12GraphicsDevice : GraphicsDevice
{
    public static readonly FeatureLevel MinFeatureLevel = FeatureLevel.Level_11_0;
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly D3D12Queue[] _queues = new D3D12Queue[(int)CommandQueueType.Count];

    private readonly GraphicsDeviceCaps _caps;

    /// <summary>
    /// Get value whether the Direct3D12 backend is supported.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    public D3D12GraphicsDevice(ValidationMode validationMode = ValidationMode.Disabled, GpuPowerPreference powerPreference = GpuPowerPreference.HighPerformance)
        : base(GpuBackend.Direct3D12)
    {
        // Create DXGI factory first
        {
            bool debugDXGI = false;

            if (validationMode != ValidationMode.Disabled)
            {
                if (D3D12GetDebugInterface(out ID3D12Debug? d3d12Debug).Success)
                {
                    d3d12Debug!.EnableDebugLayer();

                    if (validationMode == ValidationMode.GPU)
                    {
                        ID3D12Debug1? d3d12Debug1 = d3d12Debug.QueryInterfaceOrNull<ID3D12Debug1>();
                        if (d3d12Debug1 != null)
                        {
                            d3d12Debug1.SetEnableGPUBasedValidation(true);
                            d3d12Debug1.SetEnableSynchronizedCommandQueueValidation(true);
                            d3d12Debug1.Dispose();
                        }

                        ID3D12Debug2? d3d12Debug2 = d3d12Debug.QueryInterfaceOrNull<ID3D12Debug2>();
                        if (d3d12Debug1 != null)
                        {
                            d3d12Debug2.SetGPUBasedValidationFlags(GpuBasedValidationFlags.None);
                            d3d12Debug2.Dispose();
                        }
                    }

                    d3d12Debug.Dispose();
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
                }

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIInfoQueue? dxgiInfoQueue).Success)
                {
                    debugDXGI = true;

                    dxgiInfoQueue!.SetBreakOnSeverity(DebugAll, InfoQueueMessageSeverity.Error, true);
                    dxgiInfoQueue.SetBreakOnSeverity(DebugAll, InfoQueueMessageSeverity.Corruption, true);

                    int[] hide = new int[]
                    {
                        80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                    };

                    DXGI.Debug.InfoQueueFilter filter = new()
                    {
                        DenyList = new()
                        {
                            Ids = hide,
                        }
                    };

                    dxgiInfoQueue.AddStorageFilterEntries(DebugDxgi, filter);
                    dxgiInfoQueue.Dispose();
                }
#endif
            }

            DXGIFactory = CreateDXGIFactory2<IDXGIFactory4>(debugDXGI);

            // Check tearing support
            IDXGIFactory5? dxgiFactory5 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;
                dxgiFactory5.Dispose();
            }
        }

        // Get adapter and create device
        {
            IDXGIFactory6? dxgiFactory6 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory6>();
            if (dxgiFactory6 != null)
            {
                for (int adapterIndex = 0;
                    dxgiFactory6.EnumAdapterByGpuPreference(adapterIndex,
                        powerPreference == GpuPowerPreference.HighPerformance ? GpuPreference.HighPerformance : GpuPreference.MinimumPower,
                        out Adapter).Code != (int)Vortice.DXGI.ResultCode.NotFound;
                    adapterIndex++)
                {
                    AdapterDescription1 desc = Adapter!.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                    {
                        // Don't select the Basic Render Driver adapter.
                        Adapter.Dispose();
                        continue;
                    }

                    if (D3D12CreateDevice(Adapter, MinFeatureLevel, out ID3D12Device2? device).Success)
                    {
                        NativeDevice = device!;
                        break;
                    }
                }
            }
            else
            {
                for (int adapterIndex = 0;
                    DXGIFactory.EnumAdapters1(adapterIndex, out Adapter).Code != (int)Vortice.DXGI.ResultCode.NotFound; adapterIndex++)
                {
                    AdapterDescription1 desc = Adapter!.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != AdapterFlags.None)
                    {
                        // Don't select the Basic Render Driver adapter.
                        Adapter.Dispose();
                        continue;
                    }

                    if (D3D12CreateDevice(Adapter, MinFeatureLevel, out ID3D12Device2? device).Success)
                    {
                        NativeDevice = device!;
                        break;
                    }
                }
            }
        }

        if (NativeDevice == null)
        {
            throw new GraphicsException("No Direct3D 12 device found");
        }

        // Configure debug device (if active).
        if (validationMode != ValidationMode.Disabled)
        {
            ID3D12InfoQueue? d3d12InfoQueue = NativeDevice.QueryInterfaceOrNull<ID3D12InfoQueue>();
            if (d3d12InfoQueue != null)
            {
#if DEBUG
                d3d12InfoQueue.SetBreakOnSeverity(MessageSeverity.Corruption, true);
                d3d12InfoQueue.SetBreakOnSeverity(MessageSeverity.Error, true);
#endif
                MessageId[] hide = new MessageId[]
                {
                    MessageId.ClearRenderTargetViewMismatchingClearValue,
                    MessageId.ClearDepthStencilViewMismatchingClearValue,
                    MessageId.MapInvalidNullRange,
                    MessageId.UnmapInvalidNullRange,
                    MessageId.ExecuteCommandListsWrongSwapChainBufferReference,
                    MessageId.ResourceBarrierMismatchingCommandListType,
                };

                Direct3D12.Debug.InfoQueueFilter filter = new()
                {
                    DenyList = new()
                    {
                        Ids = hide,
                    }
                };

                d3d12InfoQueue.AddStorageFilterEntries(filter);


                // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                d3d12InfoQueue.SetBreakOnID(MessageId.DeviceRemovalProcessAtFault, true);
                d3d12InfoQueue.Dispose();
            }
        }

        //if (!string.IsNullOrEmpty(name))
        //{
        //    NativeDevice->SetName(name);
        //}

        // Create queues
        _queues[(int)CommandQueueType.Graphics] = new D3D12Queue(this, CommandQueueType.Graphics);
        _queues[(int)CommandQueueType.Compute] = new D3D12Queue(this, CommandQueueType.Compute);

        // Init capabilites.
        {
            AdapterDescription1 adapterDesc = Adapter.Description1;

            // Init capabilites.
            VendorId = (GpuVendorId)adapterDesc.VendorId;
            AdapterId = (uint)adapterDesc.DeviceId;

            if ((adapterDesc.Flags & AdapterFlags.Software) != AdapterFlags.Software)
            {
                AdapterType = GpuAdapterType.CPU;
            }
            else
            {
                FeatureDataArchitecture1 architecture1 = NativeDevice.Architecture1;

                AdapterType = architecture1.Uma ? GpuAdapterType.IntegratedGPU : GpuAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = architecture1.CacheCoherentUMA;
            }


            AdapterName = adapterDesc.Description;

            FeatureDataD3D12Options1 featureDataOptions1 = NativeDevice.Options1;
            FeatureDataD3D12Options5 featureDataOptions5 = NativeDevice.Options5;

            SupportsRenderPass = false;
            if (featureDataOptions5.RenderPassesTier > RenderPassTier.Tier0
                && adapterDesc.VendorId != (uint)GpuVendorId.Intel)
            {
                SupportsRenderPass = true;
            }

            _caps = new GraphicsDeviceCaps()
            {
                Features = new GraphicsDeviceFeatures
                {
                    IndependentBlend = true,
                    ComputeShader = true,
                    TessellationShader = true,
                    MultiViewport = true,
                    IndexUInt32 = true,
                    MultiDrawIndirect = true,
                    FillModeNonSolid = true,
                    SamplerAnisotropy = true,
                    TextureCompressionETC2 = false,
                    TextureCompressionASTC_LDR = false,
                    TextureCompressionBC = true,
                    TextureCubeArray = true,
                    Raytracing = featureDataOptions5.RaytracingTier >= RaytracingTier.Tier1_0
                },
                Limits = new GraphicsDeviceLimits
                {
                    MaxVertexAttributes = 16,
                    MaxVertexBindings = 16,
                    MaxVertexAttributeOffset = 2047,
                    MaxVertexBindingStride = 2048,
                    MaxTextureDimension1D = RequestTexture1DUDimension,
                    MaxTextureDimension2D = RequestTexture2DUOrVDimension,
                    MaxTextureDimension3D = RequestTexture3DUVOrWDimension,
                    MaxTextureDimensionCube = RequestTextureCubeDimension,
                    MaxTextureArrayLayers = RequestTexture2DArrayAxisDimension,
                    MaxColorAttachments = SimultaneousRenderTargetCount,
                    MaxUniformBufferRange = RequestConstantBufferElementCount * 16,
                    MaxStorageBufferRange = uint.MaxValue,
                    MinUniformBufferOffsetAlignment = 256u,
                    MinStorageBufferOffsetAlignment = 16u,
                    MaxSamplerAnisotropy = MaxMaxAnisotropy,
                    MaxViewports = ViewportAndScissorRectObjectCountPerPipeline,
                    MaxViewportWidth = ViewportBoundsMax,
                    MaxViewportHeight = ViewportBoundsMax,
                    MaxTessellationPatchSize = InputAssemblerPatchMaxControlPointCount,
                    MaxComputeSharedMemorySize = ComputeShaderThreadLocalTempRegisterPool,
                    MaxComputeWorkGroupCountX = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    MaxComputeWorkGroupCountY = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    MaxComputeWorkGroupCountZ = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    MaxComputeWorkGroupInvocations = ComputeShaderThreadGroupMaxThreadsPerGroup,
                    MaxComputeWorkGroupSizeX = ComputeShaderThreadGroupMaxX,
                    MaxComputeWorkGroupSizeY = ComputeShaderThreadGroupMaxY,
                    MaxComputeWorkGroupSizeZ = ComputeShaderThreadGroupMaxZ,
                }
            };
        }
    }

    private static bool CheckIsSupported()
    {
        return D3D12.IsSupported(IntPtr.Zero, MinFeatureLevel);
    }

    internal readonly IDXGIFactory4 DXGIFactory;
    internal readonly IDXGIAdapter1 Adapter;

    internal readonly bool IsTearingSupported;
    internal readonly ID3D12Device2 NativeDevice;

    internal D3D12Queue GetQueue(CommandQueueType type = CommandQueueType.Graphics) => _queues[(int)type];

    internal bool SupportsRenderPass { get; }

    /// <summary>
    /// Gets whether or not the current device has a cache coherent UMA architecture.
    /// </summary>
    internal bool IsCacheCoherentUMA { get; }

    // <inheritdoc />
    public override GpuVendorId VendorId { get; }

    /// <inheritdoc />
    public override uint AdapterId { get; }

    /// <inheritdoc />
    public override GpuAdapterType AdapterType { get; }

    /// <inheritdoc />
    public override string AdapterName { get; }

    /// <inheritdoc />
    public override GraphicsDeviceCaps Capabilities => _caps;

    /// <inheritdoc />
    protected override void OnDispose()
    {
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
        {
            _queues[i]?.Dispose();
        }

#if DEBUG
        uint refCount = NativeDevice.Release();
        if (refCount > 0)
        {
            System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

            ID3D12DebugDevice? d3d12DebugDevice = NativeDevice.QueryInterfaceOrNull<ID3D12DebugDevice>();
            if (d3d12DebugDevice != null)
            {
                d3d12DebugDevice.ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                d3d12DebugDevice.Dispose();
            }
        }
#else
            _handle.Dispose();
#endif
        Adapter.Dispose();
        DXGIFactory.Dispose();

#if DEBUG
        if (DXGIGetDebugInterface1(out IDXGIDebug1? dxgiDebug1).Success)
        {
            dxgiDebug1!.ReportLiveObjects(DebugAll, ReportLiveObjectFlags.Summary | ReportLiveObjectFlags.IgnoreInternal);
            dxgiDebug1.Dispose();
        }
#endif
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
        {
            _queues[i]?.WaitIdle();
        }
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(in SwapChainSource source, in SwapChainDescriptor descriptor) => new D3D12SwapChain(this, source, descriptor);

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new D3D12Texture(this, descriptor);
}
