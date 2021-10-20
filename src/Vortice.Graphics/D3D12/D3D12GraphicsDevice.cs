// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.D3D12_MESSAGE_ID;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.D3D12_RENDER_PASS_TIER;
using static TerraFX.Interop.D3D12_RAYTRACING_TIER;
using System;
#if DEBUG
using static TerraFX.Interop.D3D12_MESSAGE_SEVERITY;
using static TerraFX.Interop.D3D12_RLDO_FLAGS;
#endif

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        private readonly ComPtr<ID3D12Device2> _device;
        private readonly D3D12Queue[] _queues = new D3D12Queue[(int)CommandQueueType.Count];

#if ENABLE_D3D12MA
        private readonly ReferenceCountPtr<D3D12MA_Allocator> _allocator;
#endif

        private readonly GraphicsDeviceCaps _caps;

        internal D3D12GraphicsDevice(D3D12PhysicalDevice physicalDevice, string? name = null)
            : base(GraphicsBackend.Direct3D12, physicalDevice)
        {
            D3D12CreateDevice((IUnknown*)physicalDevice.Adapter, D3D_FEATURE_LEVEL_12_0, __uuidof<ID3D12Device2>(), _device.GetVoidAddressOf()).Assert();

            // Configure debug device (if active).
            if (physicalDevice.Factory.ValidationMode != ValidationMode.Disabled)
            {
                using ComPtr<ID3D12InfoQueue> d3d12InfoQueue = default;

                if (SUCCEEDED(_device.CopyTo(d3d12InfoQueue.GetAddressOf())))
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

            if (!string.IsNullOrEmpty(name))
            {
                _device.Get()->SetName(name);
            }

            // Create allocator.
#if ENABLE_D3D12MA
            D3D12MA_ALLOCATOR_DESC allocatorDesc = default;
            allocatorDesc.pDevice = (ID3D12Device*)_device.Get();
            allocatorDesc.pAdapter = (IDXGIAdapter*)physicalDevice.Adapter;
            D3D12MemAlloc.D3D12MA_CreateAllocator(&allocatorDesc, _allocator.GetAddressOf()).Assert();
#endif

            // Create queues
            _queues[(int)CommandQueueType.Graphics] = new D3D12Queue(this, CommandQueueType.Graphics);
            _queues[(int)CommandQueueType.Compute] = new D3D12Queue(this, CommandQueueType.Compute);

            // Init capabilites.
            var featureDataOptions1 = _device.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS1>(D3D12_FEATURE_D3D12_OPTIONS1);
            var featureDataOptions5 = _device.Get()->CheckFeatureSupport<D3D12_FEATURE_DATA_D3D12_OPTIONS5>(D3D12_FEATURE_D3D12_OPTIONS5);

            SupportsRenderPass = false;
            if (featureDataOptions5.RenderPassesTier > D3D12_RENDER_PASS_TIER_0
                && physicalDevice.VendorId != VendorId.Intel)
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
                    //MaxTextureDimension1D = RequestTexture1DUDimension,
                    //MaxTextureDimension2D = RequestTexture2DUOrVDimension,
                    //MaxTextureDimension3D = RequestTexture3DUVOrWDimension,
                    //MaxTextureDimensionCube = RequestTextureCubeDimension,
                    //MaxTextureArrayLayers = RequestTexture2DArrayAxisDimension,
                    //MaxColorAttachments = SimultaneousRenderTargetCount,
                    //MaxUniformBufferRange = RequestConstantBufferElementCount * 16,
                    //MaxStorageBufferRange = uint.MaxValue,
                    //MinUniformBufferOffsetAlignment = ConstantBufferDataPlacementAlignment,
                    MinStorageBufferOffsetAlignment = 16u,
                    //MaxSamplerAnisotropy = MaxMaxAnisotropy,
                    //MaxViewports = ViewportAndScissorRectObjectCountPerPipeline,
                    //MaxViewportWidth = ViewportBoundsMax,
                    //MaxViewportHeight = ViewportBoundsMax,
                    //MaxTessellationPatchSize = InputAssemblerPatchMaxControlPointCount,
                    //MaxComputeSharedMemorySize = ComputeShaderThreadLocalTempRegisterPool,
                    //MaxComputeWorkGroupCountX = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    //MaxComputeWorkGroupCountY = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    //MaxComputeWorkGroupCountZ = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    //MaxComputeWorkGroupInvocations = ComputeShaderThreadGroupMaxThreadsPerGroup,
                    //MaxComputeWorkGroupSizeX = ComputeShaderThreadGroupMaxX,
                    //MaxComputeWorkGroupSizeY = ComputeShaderThreadGroupMaxY,
                    //MaxComputeWorkGroupSizeZ = ComputeShaderThreadGroupMaxZ,
                }
            };
        }

        public D3D12GraphicsDeviceFactory Factory => ((D3D12PhysicalDevice)PhysicalDevice).Factory;

        public ID3D12Device2* NativeDevice => _device;

        public D3D12Queue GetQueue(CommandQueueType type = CommandQueueType.Graphics) => _queues[(int)type];

        public bool SupportsRenderPass { get; }

        /// <inheritdoc />
        public override GraphicsDeviceCaps Capabilities => _caps;

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D12GraphicsDevice" /> class.
        /// </summary>
        ~D3D12GraphicsDevice() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                for (int i = 0; i < (int)CommandQueueType.Count; i++)
                {
                    _queues[i]?.Dispose();
                }

                // Allocator.
#if ENABLE_D3D12MA
                {
                    D3D12MA_Stats stats;
                    _allocator.Get()->CalculateStats(&stats);

                    if (stats.Total.UsedBytes > 0)
                    {
                        //LOGI("Total device memory leaked: {} bytes.", stats.Total.UsedBytes);
                    }

                    _allocator.Dispose();
                }
#endif

#if DEBUG
                uint refCount = NativeDevice->Release();
                if (refCount > 0)
                {
                    System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    using ComPtr<ID3D12DebugDevice> d3d12DebugDevice = default;
                    if (SUCCEEDED(_device.CopyTo(d3d12DebugDevice.GetAddressOf())))
                    {
                        d3d12DebugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    }
                }
#else
                _device.Dispose();
#endif
            }
        }

        /// <inheritdoc />
        protected override SwapChain CreateSwapChainCore(in SwapChainSurface surface, in SwapChainDescriptor descriptor) => new D3D12SwapChain(this, surface, descriptor);

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new D3D12Texture(this, descriptor);
    }
}
