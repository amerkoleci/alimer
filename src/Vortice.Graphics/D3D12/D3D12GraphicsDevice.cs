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
    internal unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        private readonly D3D12Queue[] _queues = new D3D12Queue[(int)CommandQueueType.Count];

        private readonly GraphicsDeviceCaps _caps;

        internal D3D12GraphicsDevice(D3D12PhysicalDevice physicalDevice, string? name = null)
            : base(GraphicsBackend.Direct3D12, physicalDevice)
        {
            NativeDevice = D3D12CreateDevice<ID3D12Device2>(physicalDevice.Adapter, FeatureLevel.Level_12_0);

            // Configure debug device (if active).
            if (physicalDevice.Factory.ValidationMode != ValidationMode.Disabled)
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

            if (!string.IsNullOrEmpty(name))
            {
                NativeDevice.Name = name;
            }

            // Create queues
            _queues[(int)CommandQueueType.Graphics] = new D3D12Queue(this, CommandQueueType.Graphics);
            _queues[(int)CommandQueueType.Compute] = new D3D12Queue(this, CommandQueueType.Compute);

            // Init capabilites.
            FeatureDataD3D12Options1 featureDataOptions1 = NativeDevice.Options1;
            FeatureDataD3D12Options5 featureDataOptions5 = NativeDevice.Options5;

            SupportsRenderPass = false;
            if (featureDataOptions5.RenderPassesTier > RenderPassTier.Tier0
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

        public D3D12GraphicsDeviceFactory Factory => ((D3D12PhysicalDevice)PhysicalDevice).Factory;

        public ID3D12Device2 NativeDevice { get; }

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

#if DEBUG
                uint refCount = NativeDevice.Release();
                if (refCount > 0)
                {
                    Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    ID3D12DebugDevice? d3d12DebugDevice = NativeDevice.QueryInterfaceOrNull<ID3D12DebugDevice>();
                    if (d3d12DebugDevice != null)
                    {
                        d3d12DebugDevice!.ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                        d3d12DebugDevice.Dispose();
                    }
                }
#else
                NativeDevice.Dispose();
#endif
            }
        }

        /// <inheritdoc />
        protected override SwapChain CreateSwapChainCore(in SwapChainSurface surface, in SwapChainDescriptor descriptor) => new D3D12SwapChain(this, surface, descriptor);

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new D3D12Texture(this, descriptor);
    }
}
