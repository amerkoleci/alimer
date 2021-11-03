// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.D3D12_MESSAGE_ID;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.D3D12_RENDER_PASS_TIER;
using static TerraFX.Interop.D3D12_RAYTRACING_TIER;
using static TerraFX.Interop.D3D12_GPU_BASED_VALIDATION_FLAGS;
using static TerraFX.Interop.DXGI_FEATURE;
using static TerraFX.Interop.DXGI_GPU_PREFERENCE;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;

#if DEBUG
using static TerraFX.Interop.DXGI_DEBUG_RLO_FLAGS;
using static TerraFX.Interop.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.D3D12_RLDO_FLAGS;
#endif

namespace Vortice.Graphics
{
    public unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        public static readonly D3D_FEATURE_LEVEL MinFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

        private readonly ComPtr<IDXGIFactory4> _dxgiFactory4;
        private readonly ComPtr<IDXGIAdapter1> _adapter;
        private readonly ComPtr<ID3D12Device2> _handle;
        private readonly ComPtr<D3D12MA_Allocator> _allocator;
        private readonly D3D12Queue[] _queues = new D3D12Queue[(int)CommandQueueType.Count];

        private readonly GraphicsDeviceCaps _caps;

        /// <summary>
        /// Get value whether the Direct3D12 backend is supported.
        /// </summary>
        public static bool IsSupported => s_isSupported.Value;

        public D3D12GraphicsDevice(ValidationMode validationMode = ValidationMode.Disabled, GPUPowerPreference powerPreference = GPUPowerPreference.HighPerformance)
            : base(GraphicsBackend.Direct3D12)
        {
            // Create DXGI factory first
            {
                uint factoryFlags = 0;

                if (validationMode != ValidationMode.Disabled)
                {
                    factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                    using ComPtr<ID3D12Debug> d3d12Debug = default;

                    if (SUCCEEDED(D3D12GetDebugInterface(__uuidof<ID3D12Debug>(), d3d12Debug.GetVoidAddressOf())))
                    {
                        d3d12Debug.Get()->EnableDebugLayer();

                        if (validationMode == ValidationMode.GPU)
                        {
                            using ComPtr<ID3D12Debug1> d3d12Debug1 = default;

                            if (SUCCEEDED(d3d12Debug.CopyTo(d3d12Debug1.GetAddressOf())))
                            {
                                d3d12Debug1.Get()->SetEnableGPUBasedValidation(TRUE);
                                d3d12Debug1.Get()->SetEnableSynchronizedCommandQueueValidation(TRUE);
                            }

                            using ComPtr<ID3D12Debug2> d3d12Debug2 = default;
                            if (SUCCEEDED(d3d12Debug.CopyTo(d3d12Debug2.GetAddressOf())))
                            {
                                d3d12Debug2.Get()->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                            }
                        }
                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine("WARNING: Direct3D Debug Device is not available");
                    }

#if DEBUG
                    using ComPtr<IDXGIInfoQueue> dxgiInfoQueue = default;

                    if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof<IDXGIInfoQueue>(), dxgiInfoQueue.GetVoidAddressOf())))
                    {
                        dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
                        dxgiInfoQueue.Get()->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);

                        ReadOnlySpan<int> hide = stackalloc int[]
                        {
                            80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                        };

                        fixed (int* pIDList = hide)
                        {
                            DXGI_INFO_QUEUE_FILTER filter = new()
                            {
                                DenyList = new()
                                {
                                    NumIDs = (uint)hide.Length,
                                    pIDList = pIDList
                                }
                            };

                            dxgiInfoQueue.Get()->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
                        }
                    }
#endif
                }

                CreateDXGIFactory2(factoryFlags, __uuidof<IDXGIFactory4>(), _dxgiFactory4.GetVoidAddressOf()).Assert();

                // Check tearing support
                {
                    using ComPtr<IDXGIFactory5> dxgiFactory5 = default;
                    if (SUCCEEDED(_dxgiFactory4.CopyTo(dxgiFactory5.GetAddressOf())))
                    {
                        int allowTearing = FALSE;
                        if (FAILED(dxgiFactory5.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, (uint)sizeof(int)))
                            && allowTearing == FALSE)
                        {
                            IsTearingSupported = false;
                        }
                        else
                        {
                            IsTearingSupported = true;
                        }
                    }
                }
            }

            // Get adapter and create device
            {
                using ComPtr<IDXGIFactory6> dxgiFactory6 = default;
                if (SUCCEEDED(_dxgiFactory4.CopyTo(dxgiFactory6.GetAddressOf())))
                {
                    using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;

                    for (uint adapterIndex = 0;
                        dxgiFactory6.Get()->EnumAdapterByGpuPreference(adapterIndex,
                            powerPreference == GPUPowerPreference.HighPerformance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_MINIMUM_POWER,
                            __uuidof<IDXGIAdapter1>(),
                            dxgiAdapter1.GetVoidAddressOf()) != DXGI_ERROR_NOT_FOUND;
                        adapterIndex++)
                    {
                        DXGI_ADAPTER_DESC1 adapterDesc;
                        dxgiAdapter1.Get()->GetDesc1(&adapterDesc).Assert();

                        if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
                        {
                            // Don't select the Basic Render Driver adapter.
                            continue;
                        }

                        using ComPtr<ID3D12Device2> d3dDevice = default;

                        if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.AsIUnknown().Get(),
                            MinFeatureLevel,
                            __uuidof<ID3D12Device2>(),
                            d3dDevice.GetVoidAddressOf())))
                        {
                            _adapter = dxgiAdapter1.Move();
                            _handle = d3dDevice.Move();
                            break;
                        }
                    }
                }
                else
                {
                    using ComPtr<IDXGIAdapter1> dxgiAdapter1 = default;

                    for (uint adapterIndex = 0; _dxgiFactory4.Get()->EnumAdapters1(adapterIndex, dxgiAdapter1.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; adapterIndex++)
                    {
                        DXGI_ADAPTER_DESC1 adapterDesc;
                        dxgiAdapter1.Get()->GetDesc1(&adapterDesc).Assert();

                        if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
                        {
                            // Don't select the Basic Render Driver adapter.
                            continue;
                        }

                        using ComPtr<ID3D12Device2> d3dDevice = default;

                        if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.AsIUnknown().Get(),
                            MinFeatureLevel,
                            __uuidof<ID3D12Device2>(),
                            d3dDevice.GetVoidAddressOf())))
                        {
                            _adapter = dxgiAdapter1.Move();
                            _handle = d3dDevice.Move();
                            break;
                        }
                    }
                }
            }

            // Configure debug device (if active).
            if (validationMode != ValidationMode.Disabled)
            {
                using ComPtr<ID3D12InfoQueue> d3d12InfoQueue = default;

                if (SUCCEEDED(_handle.CopyTo(d3d12InfoQueue.GetAddressOf())))
                {
#if DEBUG
                    d3d12InfoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                    d3d12InfoQueue.Get()->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
#endif
                    ReadOnlySpan<D3D12_MESSAGE_ID> hide = stackalloc D3D12_MESSAGE_ID[]
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
                            DenyList = new()
                            {
                                NumIDs = (uint)hide.Length,
                                pIDList = pIDList
                            }
                        };

                        d3d12InfoQueue.Get()->AddStorageFilterEntries(&filter);
                    }

                    // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                    d3d12InfoQueue.Get()->SetBreakOnID(D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT, TRUE);
                }
            }

            //if (!string.IsNullOrEmpty(name))
            //{
            //    NativeDevice->SetName(name);
            //}

            // Create allocator.
            D3D12MA_ALLOCATOR_DESC allocatorDesc = default;
            allocatorDesc.pDevice = (ID3D12Device*)_handle.Get();
            allocatorDesc.pAdapter = (IDXGIAdapter*)_adapter.Get();
            HRESULT hr = D3D12MemAlloc.D3D12MA_CreateAllocator(&allocatorDesc, _allocator.GetAddressOf());
            hr.Assert();

            // Create queues
            _queues[(int)CommandQueueType.Graphics] = new D3D12Queue(this, CommandQueueType.Graphics);
            _queues[(int)CommandQueueType.Compute] = new D3D12Queue(this, CommandQueueType.Compute);

            // Init capabilites.
            {
                DXGI_ADAPTER_DESC1 adapterDesc;
                Adapter->GetDesc1(&adapterDesc).Assert();

                // Init capabilites.
                VendorId = (VendorId)adapterDesc.VendorId;
                AdapterId = adapterDesc.DeviceId;

                if (((DXGI_ADAPTER_FLAG)adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    AdapterType = GPUAdapterType.CPU;
                }
                else
                {
                    D3D12_FEATURE_DATA_ARCHITECTURE1 architecture1 = _handle.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_ARCHITECTURE1>(D3D12_FEATURE_ARCHITECTURE1);

                    AdapterType = architecture1.UMA == TRUE ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                    IsCacheCoherentUMA = architecture1.CacheCoherentUMA == TRUE;
                }


                AdapterName = new string((char*)adapterDesc.Description);

                D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureDataOptions1 = NativeDevice->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS1>(D3D12_FEATURE_D3D12_OPTIONS1);
                D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureDataOptions5 = NativeDevice->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS5>(D3D12_FEATURE_D3D12_OPTIONS5);

                SupportsRenderPass = false;
                if (featureDataOptions5.RenderPassesTier > D3D12_RENDER_PASS_TIER_0
                    && adapterDesc.VendorId != (uint)VendorId.Intel)
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
                        Raytracing = featureDataOptions5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0
                    },
                    Limits = new GraphicsDeviceLimits
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
                        MinUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
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
                    }
                };
            }
        }

        private static bool CheckIsSupported()
        {
            try
            {
                return SUCCEEDED(D3D12CreateDevice(null, MinFeatureLevel, null, null));
            }
            catch (DllNotFoundException)
            {
                return false;
            }
        }

        public IDXGIFactory4* DXGIFactory => _dxgiFactory4;
        public IDXGIAdapter1* Adapter => _adapter;

        public bool IsTearingSupported { get; private set; }

        public ID3D12Device2* NativeDevice => _handle;

        internal D3D12MA_Allocator* Allocator => _allocator;

        internal D3D12Queue GetQueue(CommandQueueType type = CommandQueueType.Graphics) => _queues[(int)type];

        internal bool SupportsRenderPass { get; }

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        internal bool IsCacheCoherentUMA { get; }

        // <inheritdoc />
        public override VendorId VendorId { get; }

        /// <inheritdoc />
        public override uint AdapterId { get; }

        /// <inheritdoc />
        public override GPUAdapterType AdapterType { get; }

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

            // Allocator.
            {
                D3D12MA_Stats stats;
                _allocator.Get()->CalculateStats(&stats);

                if (stats.Total.UsedBytes > 0)
                {
                    //LOGI("Total device memory leaked: {} bytes.", stats.Total.UsedBytes);
                }

                _allocator.Dispose();
            }

#if DEBUG
            uint refCount = _handle.Get()->Release();
            if (refCount > 0)
            {
                System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                using ComPtr<ID3D12DebugDevice> d3d12DebugDevice = default;
                if (SUCCEEDED(_handle.CopyTo(d3d12DebugDevice.GetAddressOf())))
                {
                    d3d12DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                }
            }
#else
            _handle.Dispose();
#endif
            _adapter.Dispose();
            _dxgiFactory4.Dispose();


#if DEBUG
            using ComPtr<IDXGIDebug1> dxgiDebug1 = default;

            if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof<IDXGIDebug1>(), dxgiDebug1.GetVoidAddressOf())))
            {
                dxgiDebug1.Get()->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
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
}
