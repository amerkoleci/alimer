// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;
using static Vortice.Direct3D12.D3D12;
using InfoQueueFilter = Vortice.Direct3D12.Debug.InfoQueueFilter;
using InfoQueueFilterDescription = Vortice.Direct3D12.Debug.InfoQueueFilterDescription;

namespace Vortice.Graphics.D3D12
{
    /// <summary>
    /// Direct3D12 graphics device implementation.
    /// </summary>
    internal unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        // Feature levels to try creating devices. Listed in descending order so the highest supported level is used.
        private static readonly FeatureLevel[] s_featureLevels = new[]
        {
            FeatureLevel.Level_12_2,
            FeatureLevel.Level_12_1,
            FeatureLevel.Level_12_0,
            FeatureLevel.Level_11_1,
            FeatureLevel.Level_11_0,
        };

        private static bool? s_supportInitialized;
        private readonly bool _dxgiDebug;

        private readonly ID3D12Device2 _d3dDevice;
        private GraphicsDeviceCaps _capabilities;

        public static bool IsSupported()
        {
            if (s_supportInitialized.HasValue)
            {
                return s_supportInitialized.Value;
            }

            s_supportInitialized = false;

            try
            {
                s_supportInitialized = Direct3D12.D3D12.IsSupported(FeatureLevel.Level_11_0);
            }
            catch
            {
            }

            return s_supportInitialized.Value;
        }

        public D3D12GraphicsDevice(PowerPreference powerPreference = PowerPreference.HighPerformance)
        {
            if (EnableValidation || EnableGPUBasedValidation)
            {
                if (D3D12GetDebugInterface(out ID3D12Debug d3d12Debug).Success)
                {
                    // Enable the D3D12 debug layer.
                    d3d12Debug.EnableDebugLayer();

                    ID3D12Debug1? d3D12Debug1 = d3d12Debug.QueryInterfaceOrNull<ID3D12Debug1>();
                    if (d3D12Debug1 != null)
                    {
                        d3D12Debug1.SetEnableGPUBasedValidation(EnableGPUBasedValidation);
                        //d3D12Debug1?.SetEnableSynchronizedCommandQueueValidation(true);
                        d3D12Debug1.Dispose();
                    }
                }

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIInfoQueue dxgiInfoQueue).Success)
                {
                    _dxgiDebug = true;

                    dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Error, true);
                    dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Corruption, true);

                    var filters = new DXGI.InfoQueueFilter
                    {
                        DenyList = new DXGI.InfoQueueFilterDescription
                        {
                            Ids = new[]
                            {
                                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */
                            }
                        }
                    };

                    dxgiInfoQueue.AddStorageFilterEntries(Dxgi, filters);
                }
#endif
            }

            if (CreateDXGIFactory2(_dxgiDebug, out IDXGIFactory4 factory4).Failure)
            {
                return;
            }

            DXGIFactory = factory4;

            // Check for tearing support.
            IDXGIFactory5? dxgiFactory5 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory5>();
            if (dxgiFactory5 != null)
            {
                IsTearingSupported = dxgiFactory5.PresentAllowTearing;
            }

            if (!IsTearingSupported)
            {
                Debug.WriteLine("WARNING: Variable refresh rate displays not supported");
            }

            IDXGIAdapter1? adapter = null;
            using (IDXGIFactory6? dxgiFactory6 = DXGIFactory.QueryInterfaceOrNull<IDXGIFactory6>())
            {
                if (dxgiFactory6 != null)
                {
                    GpuPreference gpuPreference = GpuPreference.HighPerformance;
                    if (powerPreference == PowerPreference.LowPower)
                    {
                        gpuPreference = GpuPreference.MinimumPower;
                    }

                    for (int adapterIndex = 0;
                        dxgiFactory6.EnumAdapterByGpuPreference(adapterIndex, gpuPreference, out adapter).Success;
                        adapterIndex++)
                    {
                        AdapterDescription1 desc = adapter!.Description1;

                        if ((desc.Flags & AdapterFlags.Software) != 0)
                        {
                            // Don't select the Basic Render Driver adapter.
                            adapter.Dispose();
                            continue;
                        }

                        // Check to see if the adapter supports Direct3D 12.
                        for (int i = 0; i < s_featureLevels.Length; i++)
                        {
                            if (D3D12CreateDevice(adapter, s_featureLevels[i], out _d3dDevice).Success)
                            {
                                goto done;
                            }
                        }
                    }

                    dxgiFactory6!.Dispose();
                }
            }

            if (adapter == null)
            {
                for (int adapterIndex = 0;
                    DXGIFactory.EnumAdapters1(adapterIndex, out adapter).Success;
                    adapterIndex++)
                {
                    AdapterDescription1 desc = adapter.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter.Dispose();
                        continue;
                    }

                    for (int i = 0; i < s_featureLevels.Length; i++)
                    {
                        if (D3D12CreateDevice(adapter, s_featureLevels[i], out _d3dDevice).Success)
                        {
                            goto done;
                        }
                    }
                }
            }
        done:

            if (_d3dDevice == null)
            {
                throw new NotSupportedException("Direct3D12 is not supported");
            }

            // Configure debug device (if active).
            {
                ID3D12InfoQueue? d3d12InfoQueue = _d3dDevice.QueryInterfaceOrNull<ID3D12InfoQueue>();
                if (d3d12InfoQueue != null)
                {
#if DEBUG
                    d3d12InfoQueue.SetBreakOnSeverity(MessageSeverity.Corruption, true);
                    d3d12InfoQueue.SetBreakOnSeverity(MessageSeverity.Error, true);
#endif
                    MessageId[] hide = new[]
                    {
                        MessageId.ClearRenderTargetViewMismatchingClearValue,
                        MessageId.ClearDepthStencilViewMismatchingClearValue,

                        MessageId.MapInvalidNullRange,
                        MessageId.UnmapInvalidNullRange,
                        //MessageId.ExecutecommandlistsWrongswapchainbufferreference
                        //D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE
                    };

                    InfoQueueFilter filter = new()
                    {
                        DenyList = new InfoQueueFilterDescription()
                        {
                            Ids = hide
                        }
                    };

                    d3d12InfoQueue.AddStorageFilterEntries(filter);

                    // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                    d3d12InfoQueue.SetBreakOnID(MessageId.DeviceRemovalProcessAtFault, true);
                    d3d12InfoQueue.Dispose();
                }
            }

            // Init capabilites.
            _capabilities.BackendType = BackendType.Direct3D12;

            AdapterDescription1 adapterDesc = adapter.Description1;
            _capabilities.VendorId = new GPUVendorId((uint)adapterDesc.VendorId);
            _capabilities.AdapterId = (uint)adapterDesc.DeviceId;
            _capabilities.AdapterName = adapterDesc.Description;

            if ((adapterDesc.Flags & AdapterFlags.Software) != AdapterFlags.None)
            {
                _capabilities.AdapterType = GraphicsAdapterType.CPU;
            }
            else
            {
                FeatureDataArchitecture featureDataArchitecture = _d3dDevice.Architecture;
                _capabilities.AdapterType = (featureDataArchitecture.Uma) ? GraphicsAdapterType.IntegratedGPU : GraphicsAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = featureDataArchitecture.CacheCoherentUMA;
            }

            FeatureDataD3D12Options1 dataOptions1 = _d3dDevice.Options1;
            FeatureDataD3D12Options5 dataOptions5 = _d3dDevice.Options5; ;

            SupportsRenderPass = false;
            if (dataOptions5.RenderPassesTier > RenderPassTier.Tier0
                && _capabilities.VendorId.KnownVendor != KnownVendorId.Intel)
            {
                SupportsRenderPass = true;
            }

            _capabilities.Features = new GraphicsDeviceFeatures
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
                Raytracing = dataOptions5.RaytracingTier >= RaytracingTier.Tier1_0
            };

            _capabilities.Limits = new GraphicsDeviceLimits
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
            };
            adapter.Dispose();

            // Create queue's
            DirectQueue = _d3dDevice.CreateCommandQueue<ID3D12CommandQueue>(CommandListType.Direct, CommandQueuePriority.Normal);
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D12GraphicsDevice" /> class.
        /// </summary>
        ~D3D12GraphicsDevice() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                //allocator->Release();

                DirectQueue.Dispose();

#if DEBUG
                uint refCount = NativeDevice.Release();
                if (refCount > 0)
                {
                    Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    ID3D12DebugDevice? debugDevice = NativeDevice.QueryInterfaceOrNull<ID3D12DebugDevice>();
                    if (debugDevice != null)
                    {
                        debugDevice.ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                        debugDevice.Dispose();
                    }
                }
#else
                NativeDevice.Dispose();
#endif
                DXGIFactory!.Dispose();

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIDebug1 dxgiDebug1).Success)
                {
                    dxgiDebug1.ReportLiveObjects(All, ReportLiveObjectFlags.Summary | ReportLiveObjectFlags.IgnoreInternal);
                    dxgiDebug1.Dispose();
                }
#endif
            }
        }

        public IDXGIFactory4 DXGIFactory { get; }
        public bool IsTearingSupported { get; }
        public bool SupportsRenderPass { get; }

        public ID3D12Device2 NativeDevice => _d3dDevice;

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        public bool IsCacheCoherentUMA { get; }

        public ID3D12CommandQueue DirectQueue { get; }

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities => _capabilities;

        protected override SwapChain CreateSwapChainCore(IntPtr windowHandle, in SwapChainDescriptor descriptor)
        {
            return new D3D12SwapChain(this, windowHandle, descriptor);
        }
    }
}
