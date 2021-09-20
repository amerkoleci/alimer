// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Vortice.Graphics.Utilities;
using System;
using Vortice.Direct3D12;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;
using SharpGen.Runtime;
using Vortice.Direct3D;
using Vortice.Direct3D12.Debug;

namespace Vortice.Graphics.D3D12
{
    public sealed unsafe class GraphicsDeviceD3D12 : GraphicsDevice
    {
        private ID3D12Device2 _nativeDevice;

        private readonly QueueD3D12[] _queues = new QueueD3D12[(int)CommandQueueType.Count];

        private readonly GraphicsDeviceCaps _caps;

        internal GraphicsDeviceD3D12(ID3D12Device2 device, IDXGIAdapter1 adapter)
        {
            Adapter = adapter;
            _nativeDevice = device;

            // Configure debug device (if active).
            if (ValidationMode != ValidationMode.Disabled)
            {
                ID3D12InfoQueue? d3d12InfoQueue = _nativeDevice.QueryInterfaceOrNull<ID3D12InfoQueue>();
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
                        MessageId.ExecuteCommandListsWrongSwapChainBufferReference
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

            // Create queues
            _queues[(int)CommandQueueType.Graphics] = new QueueD3D12(this, CommandQueueType.Graphics);
            _queues[(int)CommandQueueType.Compute] = new QueueD3D12(this, CommandQueueType.Compute);

            AdapterDescription1 adapterDesc = Adapter.Description1;

            // Init capabilites.
            GPUAdapterType adapterType;
            if ((adapterDesc.Flags & AdapterFlags.Software) != 0)
            {
                adapterType = GPUAdapterType.CPU;
            }
            else
            {
                FeatureDataArchitecture1 featureDataArchitecture = _nativeDevice.Architecture1;

                adapterType = featureDataArchitecture.Uma ? GPUAdapterType.IntegratedGPU : GPUAdapterType.DiscreteGPU;
                IsCacheCoherentUMA = featureDataArchitecture.CacheCoherentUMA;
            }

            FeatureDataD3D12Options1 featureDataOptions1 = _nativeDevice.Options1;
            FeatureDataD3D12Options5 featureDataOptions5 = _nativeDevice.Options5;

            SupportsRenderPass = false;
            if (featureDataOptions5.RenderPassesTier > RenderPassTier.Tier0
                && (VendorId)adapterDesc.VendorId != VendorId.Intel)
            {
                SupportsRenderPass = true;
            }

            _caps = new GraphicsDeviceCaps()
            {
                BackendType = GraphicsBackend.Direct3D12,
                VendorId = (VendorId)adapterDesc.VendorId,
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
                    MinUniformBufferOffsetAlignment = ConstantBufferDataPlacementAlignment,
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

        public IDXGIAdapter1 Adapter { get; }

        public ID3D12Device2 NativeDevice => _nativeDevice;

        public QueueD3D12 GetQueue(CommandQueueType type = CommandQueueType.Graphics) => _queues[(int)type];

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
                for (int i = 0; i < (int)CommandQueueType.Count; i++)
                {
                    _queues[i]?.Dispose();
                }

#if DEBUG
                uint refCount = _nativeDevice.Release();
                if (refCount > 0)
                {
                    Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    ID3D12DebugDevice? d3d12DebugDevice = _nativeDevice.QueryInterfaceOrNull<ID3D12DebugDevice>();
                    if (d3d12DebugDevice != null)
                    {
                        d3d12DebugDevice!.ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                        d3d12DebugDevice.Dispose();
                    }
                }
#else
                _nativeDevice.Dispose();
#endif

                Adapter.Dispose();
            }
        }

        /// <inheritdoc />
        protected override SwapChain CreateSwapChainCore(in SwapChainSurface surface, in SwapChainDescriptor descriptor) => new SwapChainD3D12(this, surface, descriptor);

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new TextureD3D12(this, descriptor);
    }
}
