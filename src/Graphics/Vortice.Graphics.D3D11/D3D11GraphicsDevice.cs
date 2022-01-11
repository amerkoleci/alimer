// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.Direct3D11.Debug;
using Vortice.DXGI;
using Vortice.DXGI.Debug;
using static Vortice.Direct3D11.D3D11;
using static Vortice.DXGI.DXGI;

namespace Vortice.Graphics;

public unsafe class D3D11GraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private static readonly FeatureLevel[] s_featureLevels = new[]
    {
        //FeatureLevel.Level_12_2,
        //FeatureLevel.Level_12_1,
        //FeatureLevel.Level_12_0,
        FeatureLevel.Level_11_1,
        FeatureLevel.Level_11_0,
        FeatureLevel.Level_10_1,
        FeatureLevel.Level_10_0
    };

    private readonly ID3D11Device1 _handle;
    private readonly ID3D11DeviceContext1 _immediateContext;

    private readonly GraphicsDeviceCaps _caps;

    /// <summary>
    /// Get value whether the Direct3D12 backend is supported.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    public D3D11GraphicsDevice(ValidationMode validationMode = ValidationMode.Disabled, GpuPowerPreference powerPreference = GpuPowerPreference.HighPerformance)
        : base(GpuBackend.Direct3D11)
    {
        // Create DXGI factory first
        {
            bool debugDXGI = false;

            if (validationMode != ValidationMode.Disabled)
            {
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

                    Vortice.DXGI.Debug.InfoQueueFilter filter = new()
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

            DXGIFactory = CreateDXGIFactory2<IDXGIFactory2>(debugDXGI);

            // Check tearing support
            {
                IDXGIFactory5? factory5 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory5>();
                if (factory5 != null)
                {
                    IsTearingSupported = factory5.PresentAllowTearing;
                    factory5.Dispose();
                }
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

                    break;
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

                    break;
                }
            }
        }

        if (Adapter == null)
        {
            throw new GraphicsException("No Direct3D11 device found");
        }

        DeviceCreationFlags creationFlags = DeviceCreationFlags.BgraSupport;
#if DEBUG
        if (validationMode != ValidationMode.Disabled)
        {
            if (SdkLayersAvailable())
            {
                creationFlags |= DeviceCreationFlags.Debug;
            }
        }
#endif

        if (D3D11CreateDevice(
            Adapter!,
            DriverType.Unknown,
            creationFlags,
            s_featureLevels,
            out ID3D11Device tempDevice, out FeatureLevel, out ID3D11DeviceContext tempContext).Failure)
        {
            // If the initialization fails, fall back to the WARP device.
            // For more information on WARP, see:
            // http://go.microsoft.com/fwlink/?LinkId=286690
            D3D11CreateDevice(
                null,
                DriverType.Warp,
                creationFlags,
                s_featureLevels,
                out tempDevice, out FeatureLevel, out tempContext).CheckError();
        }

        _handle = tempDevice.QueryInterface<ID3D11Device1>();
        _immediateContext = tempContext.QueryInterface<ID3D11DeviceContext1>();
        tempContext.Dispose();
        tempDevice.Dispose();

        // Configure debug device (if active).
        if (validationMode != ValidationMode.Disabled)
        {
            ID3D11Debug? d3d11Debug = _handle.QueryInterfaceOrNull<ID3D11Debug>();
            if (d3d11Debug != null)
            {
                ID3D11InfoQueue? d3d11InfoQueue = d3d11Debug!.QueryInterfaceOrNull<ID3D11InfoQueue>();

                if (d3d11InfoQueue != null)
                {
#if DEBUG
                    d3d11InfoQueue.SetBreakOnSeverity(MessageSeverity.Corruption, true);
                    d3d11InfoQueue.SetBreakOnSeverity(MessageSeverity.Error, true);
#endif
                    MessageId[] hide = new MessageId[]
                    {
                        MessageId.SetPrivateDataChangingParams,
                    };

                    Direct3D11.Debug.InfoQueueFilter filter = new()
                    {
                        DenyList = new()
                        {
                            Ids = hide,
                        }
                    };

                    d3d11InfoQueue.AddStorageFilterEntries(filter);

                    // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                    d3d11InfoQueue.SetBreakOnID(MessageId.DeviceRemovalProcessAtFault, true);
                    d3d11InfoQueue.Dispose();
                }

                d3d11Debug.Dispose();
            }
        }

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
                AdapterType = GpuAdapterType.DiscreteGPU;
            }


            AdapterName = adapterDesc.Description;

            //D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureDataOptions1 = NativeDevice->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS1>(D3D12_FEATURE_D3D12_OPTIONS1);
            //D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureDataOptions5 = NativeDevice->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS5>(D3D12_FEATURE_D3D12_OPTIONS5);
            //
            //SupportsRenderPass = false;
            //if (featureDataOptions5.RenderPassesTier > D3D12_RENDER_PASS_TIER_0
            //    && adapterDesc.VendorId != (uint)GpuVendorId.Intel)
            //{
            //    SupportsRenderPass = true;
            //}

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
                    Raytracing = false
                },
                Limits = new GraphicsDeviceLimits
                {
                    MaxVertexAttributes = 16,
                    MaxVertexBindings = 16,
                    MaxVertexAttributeOffset = 2047,
                    MaxVertexBindingStride = 2048,
                    MaxTextureDimension1D = ID3D11Resource.MaximumTexture1DSize,
                    MaxTextureDimension2D = ID3D11Resource.MaximumTexture2DSize,
                    MaxTextureDimension3D = ID3D11Resource.MaximumTexture3DSize,
                    MaxTextureDimensionCube = ID3D11Resource.MaximumTextureCubeSize,
                    MaxTextureArrayLayers = ID3D11Resource.MaximumTexture2DArraySize,
                    MaxColorAttachments = BlendDescription.SimultaneousRenderTargetCount,
                    MaxUniformBufferRange = ID3D11DeviceContext.ConstantBufferElementCount * 16,
                    MaxStorageBufferRange = uint.MaxValue,
                    MinUniformBufferOffsetAlignment = 256,
                    MinStorageBufferOffsetAlignment = 16u,
                    MaxSamplerAnisotropy = SamplerDescription.MaxMaxAnisotropy,
                    MaxViewports = ID3D11DeviceContext.ViewportAndScissorRectObjectCountPerPipeline,
                    MaxViewportWidth = ID3D11DeviceContext.ViewportBoundsMax,
                    MaxViewportHeight = ID3D11DeviceContext.ViewportBoundsMax,
                    MaxTessellationPatchSize = 32, //D3D11_IA_PATCH_MAX_CONTROL_POINT_COUNT,
                    MaxComputeSharedMemorySize = 16384, //D3D11_CS_THREAD_LOCAL_TEMP_REGISTER_POOL,
                    MaxComputeWorkGroupCountX = 65535, //D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                    MaxComputeWorkGroupCountY = 65535, //D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                    MaxComputeWorkGroupCountZ = 65535, //D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                    MaxComputeWorkGroupInvocations = 1024, // D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP,
                    MaxComputeWorkGroupSizeX = 1024, //D3D11_CS_THREAD_GROUP_MAX_X,
                    MaxComputeWorkGroupSizeY = 1024, //D3D11_CS_THREAD_GROUP_MAX_Y,
                    MaxComputeWorkGroupSizeZ = 65, //D3D11_CS_THREAD_GROUP_MAX_Z,
                }
            };
        }
    }

    private static bool CheckIsSupported()
    {
        try
        {
            return IsSupportedFeatureLevel(IntPtr.Zero, Direct3D.FeatureLevel.Level_11_0, DeviceCreationFlags.BgraSupport);
        }
        catch (DllNotFoundException)
        {
            return false;
        }
    }

    public IDXGIFactory2 DXGIFactory { get; }
    internal readonly IDXGIAdapter1 Adapter;

    public bool IsTearingSupported { get; private set; }

    public ID3D11Device1 NativeDevice => _handle;
    public readonly FeatureLevel FeatureLevel;

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

#if DEBUG
        uint refCount = _handle.Release();
        if (refCount > 0)
        {
            System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

            //using ComPtr<ID3D11Debug> d3d12DebugDevice = default;
            //if (SUCCEEDED(_handle.CopyTo(d3d12DebugDevice.GetAddressOf())))
            //{
            //    d3d12DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
            //}
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
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(in SwapChainSource source, in SwapChainDescriptor descriptor) => new D3D11SwapChain(this, source, descriptor);

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new D3D11Texture(this, descriptor);
}
