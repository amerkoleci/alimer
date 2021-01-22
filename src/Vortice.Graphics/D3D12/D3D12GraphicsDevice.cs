// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;
using static Vortice.Direct3D12.D3D12;
using static Vortice.Graphics.D3D12.D3D12Utils;
using Vortice.Direct3D12.Debug;

namespace Vortice.Graphics.D3D12
{
    /// <summary>
    /// Direct3D12 graphics device implementation.
    /// </summary>
    internal unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        private static bool? _supportInitialized;
        private static readonly FeatureLevel s_minFeatureLevel = FeatureLevel.Level_11_0;
        private IDXGIFactory4 _dxgiFactory;
        public readonly bool IsTearingSupported;
        public readonly ID3D12Device NativeDevice;
        private GraphicsDeviceCaps _capabilities;
        public bool SupportsRenderPass { get; private set; }

        public static bool IsSupported()
        {
            if (_supportInitialized.HasValue)
                return _supportInitialized.Value;

            try
            {
                if (CreateDXGIFactory2(false, out IDXGIFactory4 dxgiFactory).Failure)
                {
                    return false;
                }

                using (IDXGIAdapter1? adapter = GetAdapter(dxgiFactory, s_minFeatureLevel, false))
                {
                    _supportInitialized = adapter != null;
                }
                dxgiFactory.Dispose();

                return _supportInitialized.Value;
            }
            catch
            {
                _supportInitialized = false;

                return false;
            }
        }

        public D3D12GraphicsDevice(GraphicsAdapterType adapterPreference)
            : base()
        {
            bool debug = false;

            if (EnableValidation || EnableGPUBasedValidation)
            {
                if (D3D12GetDebugInterface(out ID3D12Debug debugController).Success)
                {
                    // Enable the D3D12 debug layer.
                    debugController.EnableDebugLayer();

                    using (ID3D12Debug1? debugController1 = debugController.QueryInterfaceOrNull<ID3D12Debug1>())
                    {
                        if (debugController1 != null)
                        {
                            debugController1.SetEnableGPUBasedValidation(EnableGPUBasedValidation);
                            debugController1.SetEnableSynchronizedCommandQueueValidation(false);
                        }
                    }
                }

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIInfoQueue dxgiInfoQueue).Success)
                {
                    debug = true;

                    dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Error, true);
                    dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Corruption, true);

                    dxgiInfoQueue.AddStorageFilterEntries(Dxgi, new Vortice.DXGI.InfoQueueFilter
                    {
                        DenyList = new Vortice.DXGI.InfoQueueFilterDescription
                        {
                            Ids = new int[]
                            {
                                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                            }
                        }
                    });

                    dxgiInfoQueue.Release();
                }
#endif
            }

            if (CreateDXGIFactory2(debug, out _dxgiFactory).Failure)
            {
                throw new NotSupportedException("IDXGIFactory4 creation failed");
            }

            // Check for tearing support.
            using (IDXGIFactory5? dxgiFactory5 = _dxgiFactory.QueryInterfaceOrNull<IDXGIFactory5>())
            {
                if (dxgiFactory5 != null)
                {
                    IsTearingSupported = dxgiFactory5.PresentAllowTearing;
                }
            }

            // Find adapter now.
            using (IDXGIAdapter1? adapter = GetAdapter(_dxgiFactory, s_minFeatureLevel, adapterPreference != GraphicsAdapterType.DiscreteGPU))
            {
                if (adapter == null)
                {
                    throw new NotSupportedException("Direct3D12: Cannot find suitable adapter with Direct3D12 support.");
                }

                D3D12CreateDevice(adapter, s_minFeatureLevel, out NativeDevice).CheckError();

                // Init capabilites.
                InitCapabilites(adapter);
            }
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
#if DEBUG
                uint refCount = NativeDevice.Release();
                if (refCount > 0)
                {
                    System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    using (ID3D12DebugDevice debugDevice = NativeDevice.QueryInterfaceOrNull<ID3D12DebugDevice>())
                    {
                        debugDevice?.ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                    }
                }
#else
                NativeDevice.Dispose();
#endif

                _dxgiFactory.Dispose();

#if DEBUG
                if (DXGIGetDebugInterface1(out IDXGIDebug1? dxgiDebug).Success)
                {
                    dxgiDebug!.ReportLiveObjects(All, ReportLiveObjectFlags.Detail | ReportLiveObjectFlags.IgnoreInternal);
                    dxgiDebug!.Dispose();
                }
#endif
            }
        }

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities => _capabilities;

        private void InitCapabilites(IDXGIAdapter1 adapter)
        {
            AdapterDescription1 desc = adapter.Description1;

            _capabilities.BackendType = BackendType.Direct3D12;
            _capabilities.VendorId = new GPUVendorId((uint)desc.VendorId);
            _capabilities.AdapterId = (uint)desc.DeviceId;

            if ((desc.Flags & AdapterFlags.Software) != 0)
            {
                _capabilities.AdapterType = GraphicsAdapterType.CPU;
            }
            else
            {
                FeatureDataArchitecture featureDataArchitecture = NativeDevice.Architecture;
                _capabilities.AdapterType = featureDataArchitecture.Uma ? GraphicsAdapterType.IntegratedGPU : GraphicsAdapterType.DiscreteGPU;
            }

            _capabilities.AdapterName = desc.Description;

            FeatureDataD3D12Options5 options5 = NativeDevice.Options5;

            SupportsRenderPass = false;
            if (options5.RenderPassesTier > RenderPassTier.Tier0
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
                Raytracing = options5.RaytracingTier >= RaytracingTier.Tier1_0
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
        }
    }
}
