// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.Direct3D11.Debug;
using static Vortice.Direct3D11.D3D11;

namespace Vortice.Graphics.D3D11
{
    internal unsafe class D3D11GraphicsDevice : GraphicsDevice
    {
        private static readonly FeatureLevel[] s_featureLevels = new[]
        {
            FeatureLevel.Level_11_1,
            FeatureLevel.Level_11_0,
            FeatureLevel.Level_10_1,
            FeatureLevel.Level_10_0
        };

        private FeatureLevel _featureLevel;
        private readonly GraphicsDeviceCaps _caps;

        internal D3D11GraphicsDevice(D3D11PhysicalDevice physicalDevice, string? name = null)
            : base(GraphicsBackend.Direct3D11, physicalDevice)
        {
            {
                DeviceCreationFlags creationFlags = DeviceCreationFlags.BgraSupport;

                if (physicalDevice.Factory.ValidationMode != ValidationMode.Disabled && SdkLayersAvailable())
                {
                    creationFlags |= DeviceCreationFlags.Debug;
                }

                D3D11CreateDevice(physicalDevice.Adapter, DriverType.Unknown, creationFlags, s_featureLevels,
                    out ID3D11Device? tempDevice,
                    out _featureLevel,
                    out ID3D11DeviceContext? tempContext).CheckError();

                // Configure debug device (if active).
                if (physicalDevice.Factory.ValidationMode != ValidationMode.Disabled)
                {
                    ID3D11Debug? d3d11Debug = tempDevice.QueryInterfaceOrNull<ID3D11Debug>();
                    if (d3d11Debug != null)
                    {
                        ID3D11InfoQueue? d3dInfoQueue = d3d11Debug!.QueryInterfaceOrNull<ID3D11InfoQueue>();
                        if (d3dInfoQueue != null)
                        {
                            d3dInfoQueue.SetBreakOnSeverity(MessageSeverity.Corruption, true);
                            d3dInfoQueue.SetBreakOnSeverity(MessageSeverity.Error, true);

                            MessageId[] hide = new MessageId[]
                            {
                                MessageId.SetPrivateDataChangingParams,
                            };

                            InfoQueueFilter filter = new()
                            {
                                DenyList = new InfoQueueFilterDescription()
                                {
                                    Ids = hide
                                }
                            };

                            d3dInfoQueue.AddStorageFilterEntries(filter);

                            // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                            d3dInfoQueue.SetBreakOnID(MessageId.DeviceRemovalProcessAtFault, true);
                            d3dInfoQueue.Dispose();
                        }

                        d3d11Debug!.Dispose();
                    }
                }

                NativeDevice = tempDevice.QueryInterface<ID3D11Device1>();
                ImmediateContext = tempContext.QueryInterface<ID3D11DeviceContext1>();
                tempDevice.Dispose();
                tempContext.Dispose();
            }

            if (!string.IsNullOrEmpty(name))
            {
                IntPtr namePtr = Marshal.StringToHGlobalAnsi(name);
                NativeDevice.SetPrivateData(CommonGuid.DebugObjectName, name!.Length, namePtr);
                Marshal.FreeHGlobal(namePtr);
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
                    Raytracing = false
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
                    //MinStorageBufferOffsetAlignment = 16u,
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

        public D3D11GraphicsDeviceFactory Factory => ((D3D11PhysicalDevice)PhysicalDevice).Factory;

        public ID3D11Device1 NativeDevice { get; }
        public ID3D11DeviceContext1 ImmediateContext { get; }
        public FeatureLevel FeatureLevel => _featureLevel;

        /// <inheritdoc />
        public override GraphicsDeviceCaps Capabilities => _caps;

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D11GraphicsDevice" /> class.
        /// </summary>
        ~D3D11GraphicsDevice() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                ImmediateContext.Dispose();

#if DEBUG
                uint refCount = NativeDevice.Release();
                if (refCount > 0)
                {
                    Debug.WriteLine($"Direct3D11: There are {refCount} unreleased references left on the device");

                    ID3D11Debug? d3d11Debug = NativeDevice.QueryInterfaceOrNull<ID3D11Debug>();
                    if (d3d11Debug != null)
                    {
                        d3d11Debug!.ReportLiveDeviceObjects(ReportLiveDeviceObjectFlags.Detail | ReportLiveDeviceObjectFlags.IgnoreInternal);
                        d3d11Debug.Dispose();
                    }
                }
#else
                _nativeDevice.Dispose();
#endif
            }
        }

        /// <inheritdoc />
        protected override SwapChain CreateSwapChainCore(in SwapChainSurface surface, in SwapChainDescriptor descriptor) => new D3D11SwapChain(this, surface, descriptor);

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new D3D11Texture(this, descriptor);
    }
}
