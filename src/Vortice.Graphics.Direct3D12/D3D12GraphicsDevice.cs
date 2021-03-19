// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;
using TerraFX.Interop;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_FLAGS;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.D3D12_MESSAGE_ID;
using static TerraFX.Interop.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.D3D12_RAYTRACING_TIER;
using static TerraFX.Interop.D3D12_RENDER_PASS_TIER;
using static TerraFX.Interop.D3D12_RLDO_FLAGS;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DXGI_DEBUG_RLO_FLAGS;
using static TerraFX.Interop.DXGI_FEATURE;
using static TerraFX.Interop.DXGI_GPU_PREFERENCE;
using static TerraFX.Interop.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.D3D12Utils;

namespace Vortice.Graphics.D3D12
{
    /// <summary>
    /// Direct3D12 graphics device implementation.
    /// </summary>
    public unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        // Feature levels to try creating devices. Listed in descending order so the highest supported level is used.
        private static readonly D3D_FEATURE_LEVEL[] s_featureLevels = new[]
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        private static bool? s_supportInitialized;
        private readonly uint _dxgiFactoryFlags = 0;
        private ComPtr<IDXGIFactory4> _dxgiFactory = default;
        private bool _tearingSupported = default;

        private readonly ComPtr<ID3D12Device2> d3d12Device;
        private GraphicsDeviceCaps _capabilities;

        private readonly D3D12MA_Allocator* allocator;

        private readonly ComPtr<ID3D12CommandQueue> directQueue;

        public static bool IsSupported()
        {
            if (s_supportInitialized.HasValue)
                return s_supportInitialized.Value;

            s_supportInitialized = false;

            try
            {
                HRESULT result = D3D12CreateDevice(null, D3D_FEATURE_LEVEL_11_0, __uuidof<ID3D12Device>(), null);
                s_supportInitialized = SUCCEEDED(result);
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
                using ComPtr<ID3D12Debug> d3D12Debug = default;
                using ComPtr<ID3D12Debug1> d3D12Debug1 = default;

                if (SUCCEEDED(D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), d3D12Debug.GetVoidAddressOf())))
                {
                    // Enable the D3D12 debug layer.
                    d3D12Debug.Get()->EnableDebugLayer();

                    if (SUCCEEDED(d3D12Debug.CopyTo(d3D12Debug1.GetAddressOf())))
                    {
                        d3D12Debug1.Get()->SetEnableGPUBasedValidation(EnableGPUBasedValidation ? TRUE : FALSE);
                        d3D12Debug1.Get()->SetEnableSynchronizedCommandQueueValidation(TRUE);
                    }
                }

#if DEBUG
                using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

                if (SUCCEEDED(DXGIGetDebugInterface1(0u, __uuidof<IDXGIInfoQueue>(), dxgiInfoQueue.GetVoidAddressOf())))
                {
                    _dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                    dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
                    dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);

                    int* denyIds = stackalloc int[1];
                    denyIds[0] = 80; /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */

                    DXGI_INFO_QUEUE_FILTER filters = new DXGI_INFO_QUEUE_FILTER
                    {
                        DenyList = new DXGI_INFO_QUEUE_FILTER_DESC
                        {
                            NumIDs = 1,
                            pIDList = denyIds
                        }
                    };

                    ThrowIfFailed(dxgiInfoQueue.Get()->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filters));
                }
#endif
            }

            if (FAILED(CreateDXGIFactory2(_dxgiFactoryFlags, __uuidof<IDXGIFactory4>(), _dxgiFactory.GetVoidAddressOf())))
            {
                return;
            }

            using ComPtr<IDXGIFactory5> dxgiFactory5 = default;
            if (SUCCEEDED(_dxgiFactory.CopyTo(dxgiFactory5.GetAddressOf())))
            {
                int allowTearing = FALSE;
                HRESULT hr = dxgiFactory5.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, (uint)sizeof(int));

                if (FAILED(hr) || allowTearing == 0)
                {
                    _tearingSupported = false;
                    Debug.WriteLine("WARNING: Variable refresh rate displays not supported");
                }
                else
                {
                    _tearingSupported = true;
                }
            }

            bool shouldBreak = false;

            using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;
            using ComPtr<IDXGIFactory6> dxgiFactor6 = default;
            if (SUCCEEDED(_dxgiFactory.CopyTo(dxgiFactor6.GetAddressOf())))
            {
                uint adapterIndex = 0;
                while (!shouldBreak)
                {
                    DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
                    if (powerPreference == PowerPreference.LowPower)
                    {
                        gpuPreference = DXGI_GPU_PREFERENCE_MINIMUM_POWER;
                    }

                    HRESULT result = dxgiFactor6.Get()->EnumAdapterByGpuPreference(
                        adapterIndex++,
                        gpuPreference,
                        __uuidof<IDXGIAdapter1>(),
                        (void**)dxgiAdapter1.ReleaseAndGetAddressOf());

                    if (result == DXGI_ERROR_NOT_FOUND)
                    {
                        shouldBreak = true;
                        break;
                    }

                    DXGI_ADAPTER_DESC1 desc;
                    ThrowIfFailed(dxgiAdapter1.Get()->GetDesc1(&desc));

                    if ((desc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    for (int i = 0; i < s_featureLevels.Length; i++)
                    {
                        if (SUCCEEDED(D3D12CreateDevice(
                            dxgiAdapter1.AsIUnknown().Get(),
                            s_featureLevels[i],
                            __uuidof<ID3D12Device2>(),
                            d3d12Device.GetVoidAddressOf())))
                        {

                            shouldBreak = true;
                            break;
                        }
                    }
                }
            }
            else
            {
                uint adapterIndex = 0;

                while (!shouldBreak)
                {
                    HRESULT result = _dxgiFactory.Get()->EnumAdapters1(adapterIndex++, dxgiAdapter1.ReleaseAndGetAddressOf());

                    if (result == DXGI_ERROR_NOT_FOUND)
                    {
                        shouldBreak = true;
                        break;
                    }

                    DXGI_ADAPTER_DESC1 desc;
                    ThrowIfFailed(dxgiAdapter1.Get()->GetDesc1(&desc));

                    if ((desc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    for (int i = 0; i < s_featureLevels.Length; i++)
                    {
                        if (SUCCEEDED(D3D12CreateDevice(
                            dxgiAdapter1.AsIUnknown().Get(),
                            s_featureLevels[i],
                            __uuidof<ID3D12Device2>(),
                            d3d12Device.GetVoidAddressOf())))
                        {
                            shouldBreak = true;
                            break;
                        }
                    }
                }
            }

            // Configure debug device (if active).
            {
                using ComPtr<ID3D12InfoQueue> d3d12InfoQueue = default;
                if (SUCCEEDED(d3d12Device.CopyTo(d3d12InfoQueue.GetAddressOf())))
                {
#if DEBUG
                    d3d12InfoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                    d3d12InfoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
#endif
                    D3D12_MESSAGE_ID[] hide = new[]
                    {
                        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,

                        D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                        D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                        //D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE
                    };

                    fixed (D3D12_MESSAGE_ID* pIdList = &hide[0])
                    {
                        D3D12_INFO_QUEUE_FILTER filter = new D3D12_INFO_QUEUE_FILTER
                        {
                            DenyList = new D3D12_INFO_QUEUE_FILTER_DESC
                            {
                                NumIDs = (uint)hide.Length,
                                pIDList = pIdList
                            }
                        };

                        ThrowIfFailed(d3d12InfoQueue.Get()->AddStorageFilterEntries(&filter));
                    }

                    // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                    d3d12InfoQueue.Get()->SetBreakOnID(D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT, TRUE);
                }
            }

            // Init capabilites.
            _capabilities.BackendType = BackendType.Direct3D12;

            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(dxgiAdapter1.Get()->GetDesc1(&adapterDesc));
            _capabilities.VendorId = new GPUVendorId(adapterDesc.VendorId);
            _capabilities.AdapterId = adapterDesc.DeviceId;
            _capabilities.AdapterName = new string((char*)adapterDesc.Description);

            if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                _capabilities.AdapterType = GraphicsAdapterType.CPU;
            }
            else
            {
                D3D12_FEATURE_DATA_ARCHITECTURE1 featureDataArchitecture = d3d12Device.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_ARCHITECTURE1>(D3D12_FEATURE_ARCHITECTURE1);
                _capabilities.AdapterType = (featureDataArchitecture.UMA == TRUE) ? GraphicsAdapterType.IntegratedGPU : GraphicsAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = featureDataArchitecture.CacheCoherentUMA != 0;
            }

            D3D12_FEATURE_DATA_D3D12_OPTIONS1 dataOptions1 = d3d12Device.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS1>(D3D12_FEATURE_D3D12_OPTIONS1);
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 dataOptions5 = d3d12Device.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS5>(D3D12_FEATURE_D3D12_OPTIONS5);

            SupportsRenderPass = false;
            if (dataOptions5.RenderPassesTier > D3D12_RENDER_PASS_TIER_0
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
                Raytracing = dataOptions5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0
            };

            _capabilities.Limits = new GraphicsDeviceLimits
            {
                MaxVertexAttributes = 16,
                MaxVertexBindings = 16,
                MaxVertexAttributeOffset = 2047,
                MaxVertexBindingStride = 2048,
                MaxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION,
                MaxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
                MaxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
                MaxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION,
                MaxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,
                MaxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT,
                MaxUniformBufferRange = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16,
                MaxStorageBufferRange = uint.MaxValue,
                MinUniformBufferOffsetAlignment = 256u,
                MinStorageBufferOffsetAlignment = 16u,
                MaxSamplerAnisotropy = D3D12_MAX_MAXANISOTROPY,
                MaxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
                MaxViewportWidth = D3D12_VIEWPORT_BOUNDS_MAX,
                MaxViewportHeight = D3D12_VIEWPORT_BOUNDS_MAX,
                MaxTessellationPatchSize = D3D12_IA_PATCH_MAX_CONTROL_POINT_COUNT,
                MaxComputeSharedMemorySize = D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL,
                MaxComputeWorkGroupCountX = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                MaxComputeWorkGroupCountY = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                MaxComputeWorkGroupCountZ = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                MaxComputeWorkGroupInvocations = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP,
                MaxComputeWorkGroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X,
                MaxComputeWorkGroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y,
                MaxComputeWorkGroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z,
            };

            // Create allocator
            D3D12MA_ALLOCATOR_DESC allocatorDesc = default;
            allocatorDesc.pDevice = (ID3D12Device*)d3d12Device.Get();
            allocatorDesc.pAdapter = (IDXGIAdapter*)dxgiAdapter1.Get();

            D3D12MA_Allocator* newAllocator;
            ThrowIfFailed(D3D12MemAlloc.D3D12MA_CreateAllocator(&allocatorDesc, &newAllocator));
            allocator = newAllocator;

            // Create queue
            D3D12_COMMAND_QUEUE_DESC queueDesc;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.NodeMask = 0;

            ThrowIfFailed(d3d12Device.Get()->CreateCommandQueue(&queueDesc,
                __uuidof<ID3D12CommandQueue>(),
                directQueue.GetVoidAddressOf())
                );
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
                allocator->Release();

                directQueue.Dispose();

#if DEBUG
                uint refCount = d3d12Device.Reset();
                if (refCount > 0)
                {
                    Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    using ComPtr<ID3D12DebugDevice> debugDevice = default;
                    if (SUCCEEDED(d3d12Device.CopyTo(debugDevice.GetAddressOf())))
                    {
                        debugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    }
                }
#else
                d3d12Device.Dispose();
#endif
                _dxgiFactory.Dispose();

#if DEBUG
                using ComPtr<IDXGIDebug1> dxgiDebug1 = default;
                if (SUCCEEDED(DXGIGetDebugInterface1(0u, __uuidof<IDXGIDebug1>(), dxgiDebug1.GetVoidAddressOf())))
                {
                    dxgiDebug1.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
                }
#endif
            }
        }

        internal IDXGIFactory4* DXGIFactory => _dxgiFactory;
        public bool IsTearingSupported => _tearingSupported;

        public bool SupportsRenderPass { get; private set; }

        internal ID3D12Device2* D3D12Device => d3d12Device;

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        internal bool IsCacheCoherentUMA { get; }

        internal ID3D12CommandQueue* DirectQueue => directQueue;

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities => _capabilities;

        protected override SwapChain CreateSwapChainCore(IntPtr windowHandle, in SwapChainDescriptor descriptor) => new D3D12SwapChain(this, windowHandle, descriptor);
    }
}
