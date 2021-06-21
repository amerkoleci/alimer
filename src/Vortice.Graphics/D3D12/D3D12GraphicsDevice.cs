// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D12;
using Vortice.Direct3D12.Debug;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using InfoQueueFilter = Vortice.Direct3D12.Debug.InfoQueueFilter;
using InfoQueueFilterDescription = Vortice.Direct3D12.Debug.InfoQueueFilterDescription;

namespace Vortice.Graphics.D3D12
{
    public sealed unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        private readonly GraphicsDeviceCaps _caps;

        internal D3D12GraphicsDevice(ID3D12Device2 device, IDXGIAdapter1 adapter)
        {
            NativeDevice = device;
            Adapter = adapter;

            // Configure debug device (if active).
            using (ID3D12InfoQueue? infoQueue = NativeDevice.QueryInterfaceOrNull<ID3D12InfoQueue>())
            {
                if (infoQueue != null)
                {
#if DEBUG
                    infoQueue.SetBreakOnSeverity(MessageSeverity.Corruption, true);
                    infoQueue.SetBreakOnSeverity(MessageSeverity.Error, true);
#endif
                    MessageId[] hide = new[]
                    {
                        MessageId.ClearRenderTargetViewMismatchingClearValue,
                        MessageId.ClearDepthStencilViewMismatchingClearValue,

                        MessageId.MapInvalidNullRange,
                        MessageId.UnmapInvalidNullRange,
                        MessageId.ExecuteCommandListsWrongSwapChainBufferReference
                    };

                    InfoQueueFilter filter = new()
                    {
                        DenyList = new InfoQueueFilterDescription()
                        {
                            Ids = hide
                        }
                    };

                    infoQueue.AddStorageFilterEntries(filter);

                    // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                    infoQueue.SetBreakOnID(MessageId.DeviceRemovalProcessAtFault, true);
                }
            }

            AdapterDescription1 adapterDesc = adapter.Description1;

            // Init capabilites.
            GPUAdapterType adapterType;
            if ((adapterDesc.Flags & AdapterFlags.Software) != AdapterFlags.None)
            {
                adapterType = GPUAdapterType.CPU;
            }
            else
            {
                FeatureDataArchitecture featureDataArchitecture = NativeDevice.Architecture;
                adapterType = (featureDataArchitecture.Uma) ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = featureDataArchitecture.CacheCoherentUMA;
            }

            FeatureDataD3D12Options1 dataOptions1 = NativeDevice.Options1;
            FeatureDataD3D12Options5 dataOptions5 = NativeDevice.Options5;

            SupportsRenderPass = false;
            if (dataOptions5.RenderPassesTier > RenderPassTier.Tier0
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
                AdapterName = adapterDesc.Description,
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
                    Raytracing = dataOptions5.RaytracingTier >= RaytracingTier.Tier1_0
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

        public ID3D12Device2 NativeDevice { get; }
        public IDXGIAdapter1 Adapter { get; }

        /// <summary>
        /// Gets whether or not the current device has a cache coherent UMA architecture.
        /// </summary>
        public bool IsCacheCoherentUMA { get; }

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
        }

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new D3D12Texture(this, descriptor);
    }
}
