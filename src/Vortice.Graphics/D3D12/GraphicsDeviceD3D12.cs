// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using FX = TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.D3D12_RLDO_FLAGS;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.D3D12_MESSAGE_ID;
using static TerraFX.Interop.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.D3D12_RENDER_PASS_TIER;
using static TerraFX.Interop.D3D12_RAYTRACING_TIER;
using System;
using System.Diagnostics;
using static Vortice.Graphics.Utilities;
using static Vortice.Graphics.D3D12.D3D12Utils;

namespace Vortice.Graphics.D3D12
{
    public sealed unsafe class GraphicsDeviceD3D12 : GraphicsDevice
    {
        private ComPtr<ID3D12Device2> _nativeDevice;

        /// <summary>
        /// The <see cref="D3D12MA_Allocator"/> in use associated to the current device.
        /// </summary>
        private UniquePtr<D3D12MA_Allocator> _allocator;

        private readonly GraphicsDeviceCaps _caps;

        internal GraphicsDeviceD3D12(ComPtr<IDXGIAdapter1> dxgiAdapter1)
        {
            HRESULT result = FX.D3D12CreateDevice(
                dxgiAdapter1.AsIUnknown().Get(),
                D3D_FEATURE_LEVEL_11_0,
                FX.__uuidof<ID3D12Device>(),
                _nativeDevice.GetVoidAddressOf());

            result.Assert();

            // Configure debug device (if active).
            using ComPtr<ID3D12InfoQueue> d3d12InfoQueue = default;
            if (FX.SUCCEEDED(_nativeDevice.CopyTo(d3d12InfoQueue.GetAddressOf())))
            {
#if DEBUG
                d3d12InfoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, FX.TRUE);
                d3d12InfoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FX.TRUE);
#endif
                Span<D3D12_MESSAGE_ID> hide = stackalloc D3D12_MESSAGE_ID[]
                {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE
                };

                fixed (D3D12_MESSAGE_ID* pIDList = hide)
                {
                    D3D12_INFO_QUEUE_FILTER filter = new()
                    {
                        DenyList = new D3D12_INFO_QUEUE_FILTER_DESC()
                        {
                            NumIDs = (uint)hide.Length,
                            pIDList = pIDList
                        }
                    };

                    d3d12InfoQueue.Get()->AddStorageFilterEntries(&filter);
                }

                // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                d3d12InfoQueue.Get()->SetBreakOnID(D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT, FX.TRUE);
            }

            // Create memory allocator.
            D3D12MA_ALLOCATOR_DESC allocatorDesc = default;
            allocatorDesc.pDevice = (ID3D12Device*)_nativeDevice.Get();
            allocatorDesc.pAdapter = (IDXGIAdapter*)dxgiAdapter1.Get();
            fixed (D3D12MA_Allocator** allocator = _allocator)
            {
                D3D12MemAlloc.D3D12MA_CreateAllocator(&allocatorDesc, allocator).Assert();
            }

            DXGI_ADAPTER_DESC1 adapterDesc;
            dxgiAdapter1.Get()->GetDesc1(&adapterDesc).Assert();

            // Init capabilites.
            GPUAdapterType adapterType;
            if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                adapterType = GPUAdapterType.CPU;
            }
            else
            {
                D3D12_FEATURE_DATA_ARCHITECTURE1 featureDataArchitecture = _nativeDevice.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_ARCHITECTURE1>(D3D12_FEATURE_ARCHITECTURE1);

                adapterType = (featureDataArchitecture.UMA != 0) ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = featureDataArchitecture.CacheCoherentUMA != 0;
            }

            D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureDataOptions1 = _nativeDevice.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS1>(D3D12_FEATURE_D3D12_OPTIONS1);
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureDataOptions5 = _nativeDevice.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS5>(D3D12_FEATURE_D3D12_OPTIONS5);

            SupportsRenderPass = false;
            if (featureDataOptions5.RenderPassesTier > D3D12_RENDER_PASS_TIER_0
                && adapterDesc.VendorId != (int)KnownVendorId.Intel)
            {
                SupportsRenderPass = true;
            }

            _caps = new GraphicsDeviceCaps()
            {
                BackendType = GraphicsBackend.Direct3D12,
                VendorId = new GPUVendorId((uint)adapterDesc.VendorId),
                AdapterId = (uint)adapterDesc.DeviceId,
                AdapterType = adapterType,
                AdapterName = GetUtf16Span(in adapterDesc.Description[0], 128).GetString(),
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
                    Raytracing = featureDataOptions5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0
                },
                Limits = new GraphicsDeviceLimits
                {
                    MaxVertexAttributes = 16,
                    MaxVertexBindings = 16,
                    MaxVertexAttributeOffset = 2047,
                    MaxVertexBindingStride = 2048,
                    MaxTextureDimension1D = FX.D3D12_REQ_TEXTURE1D_U_DIMENSION,
                    MaxTextureDimension2D = FX.D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
                    MaxTextureDimension3D = FX.D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
                    MaxTextureDimensionCube = FX.D3D12_REQ_TEXTURECUBE_DIMENSION,
                    MaxTextureArrayLayers = FX.D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,
                    MaxColorAttachments = FX.D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT,
                    MaxUniformBufferRange = FX.D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16,
                    MaxStorageBufferRange = uint.MaxValue,
                    MinUniformBufferOffsetAlignment = 256u,
                    MinStorageBufferOffsetAlignment = 16u,
                    MaxSamplerAnisotropy = FX.D3D12_MAX_MAXANISOTROPY,
                    MaxViewports = FX.D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
                    MaxViewportWidth = FX.D3D12_VIEWPORT_BOUNDS_MAX,
                    MaxViewportHeight = FX.D3D12_VIEWPORT_BOUNDS_MAX,
                    MaxTessellationPatchSize = FX.D3D12_IA_PATCH_MAX_CONTROL_POINT_COUNT,
                    MaxComputeSharedMemorySize = FX.D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL,
                    MaxComputeWorkGroupCountX = FX.D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                    MaxComputeWorkGroupCountY = FX.D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                    MaxComputeWorkGroupCountZ = FX.D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                    MaxComputeWorkGroupInvocations = FX.D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP,
                    MaxComputeWorkGroupSizeX = FX.D3D12_CS_THREAD_GROUP_MAX_X,
                    MaxComputeWorkGroupSizeY = FX.D3D12_CS_THREAD_GROUP_MAX_Y,
                    MaxComputeWorkGroupSizeZ = FX.D3D12_CS_THREAD_GROUP_MAX_Z,
                }
            };
        }

        public ID3D12Device2* NativeDevice => _nativeDevice;

        /// <summary>
        /// Gets the underlying <see cref="D3D12MA_Allocator"/> wrapped by the current instance.
        /// </summary>
        internal D3D12MA_Allocator* Allocator => _allocator;

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        public bool IsCacheCoherentUMA { get; }

        public bool SupportsRenderPass { get; }

        /// <inheritdoc />
        public override GraphicsDeviceCaps Capabilities => _caps;

        /// <summary>
        /// Finalizes an instance of the <see cref="GraphicsDeviceD3D12" /> class.
        /// </summary>
        ~GraphicsDeviceD3D12() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                D3D12MA_Stats stats;
                Allocator->CalculateStats(&stats);

                if (stats.Total.UsedBytes > 0)
                {
                    Console.WriteLine($"Total device memory leaked: {stats.Total.UsedBytes} bytes.");
                }

                _allocator.Dispose();
#if DEBUG
                var d3dDevice = _nativeDevice.Get();
                uint refCount = _nativeDevice.Reset();
                if (refCount > 0)
                {
                    Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    using ComPtr<ID3D12DebugDevice> d3d12DebugDevice = default;
                    if (FX.SUCCEEDED(d3dDevice->QueryInterface(FX.__uuidof<ID3D12DebugDevice>(), d3d12DebugDevice.GetVoidAddressOf())))
                    {
                        d3d12DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    }
                }
#else
                _nativeDevice.Dispose();
#endif
            }
        }

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new TextureD3D12(this, descriptor);
    }
}
