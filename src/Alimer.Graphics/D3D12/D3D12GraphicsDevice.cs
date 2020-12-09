// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using TerraFX.Interop;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static Alimer.InteropUtilities;
using static TerraFX.Interop.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.D3D12_FEATURE;
using static TerraFX.Interop.D3D12_RLDO_FLAGS;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DXGI_FEATURE;
using static TerraFX.Interop.DXGI_INFO_QUEUE_MESSAGE_SEVERITY;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D12_RAYTRACING_TIER;

namespace Alimer.Graphics.D3D12
{
    /// <summary>
    /// Direct3D12 graphics device implementation.
    /// </summary>
    internal unsafe class D3D12GraphicsDevice : GraphicsDevice
    {
        private D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        private IDXGIFactory4* _dxgiFactory;
        public readonly bool IsTearingSupported;
        public readonly ID3D12Device* NativeDevice;
        private GraphicsDeviceCaps _capabilities;

        public static bool IsSupported()
        {
            try
            {
                var dxgiFactoryFlags = 0u;
                var iid = IID_IDXGIFactory4;

                using ComPtr<IDXGIFactory4> dxgiFactory = null;
                if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, &iid, (void**)&dxgiFactory)))
                {
                    return false;
                }

                IDXGIAdapter1* adapter = GetAdapter(dxgiFactory, D3D_FEATURE_LEVEL_11_0, false);
                bool supported = adapter != null;

                return true;
            }
            catch
            {
                return false;
            }
        }

        public D3D12GraphicsDevice(GraphicsAdapterType adapterPreference)
            : base()
        {
            uint dxgiFactoryFlags = 0u;
            Guid iid = default;

            if (EnableValidation || EnableGPUBasedValidation)
            {
                iid = IID_ID3D12Debug;
                ID3D12Debug* debugController = null;
                if (SUCCEEDED(D3D12GetDebugInterface(&iid, (void**)&debugController)))
                {
                    // Enable the D3D12 debug layer.
                    debugController->EnableDebugLayer();

                    iid = IID_ID3D12Debug1;
                    ID3D12Debug1* debugController1 = null;
                    if (SUCCEEDED(debugController->QueryInterface(&iid, (void**)&debugController1)))
                    {
                        debugController1->SetEnableGPUBasedValidation(EnableGPUBasedValidation ? TRUE : FALSE);
                        //debugController1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                        debugController1->Release();
                    }

                    debugController->Release();
                }

#if DEBUG
                IDXGIInfoQueue* dxgiInfoQueue;
                iid = IID_IDXGIInfoQueue;
                if (SUCCEEDED(DXGIGetDebugInterface1(0u, &iid, (void**)&dxgiInfoQueue)))
                {
                    dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                    ThrowIfFailed(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE));
                    ThrowIfFailed(dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE));

                    int* hide = stackalloc int[1]
                    {
                        80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                    };

                    DXGI_INFO_QUEUE_FILTER filter = default;
                    filter.DenyList.NumIDs = 1u;
                    filter.DenyList.pIDList = hide;

                    ThrowIfFailed(dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter));
                    dxgiInfoQueue->Release();
                }
#endif
            }

            iid = IID_IDXGIFactory4;
            IDXGIFactory4* dxgiFactory;
            if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, &iid, (void**)&dxgiFactory)))
            {
                throw new NotSupportedException("IDXGIFactory4 creation failed");
            }
            _dxgiFactory = dxgiFactory;

            // Check for tearing support.
            iid = IID_IDXGIFactory5;
            using ComPtr<IDXGIFactory5> dxgiFactory5 = null;
            if (SUCCEEDED(_dxgiFactory->QueryInterface(&iid, (void**)&dxgiFactory5)))
            {
                int allowTearing = 0;
                ThrowIfFailed(dxgiFactory5.Get()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(int)));
                IsTearingSupported = allowTearing == 1;
            }

            // Find adapter now.
            IDXGIAdapter1* adapter = GetAdapter(_dxgiFactory, minFeatureLevel, adapterPreference != GraphicsAdapterType.DiscreteGPU);
            if (adapter == null)
            {
                throw new NotSupportedException("Direct3D12: Cannot find suitable adapter with Direct3D12 support.");
            }

            ID3D12Device* device;
            iid = IID_ID3D12Device;
            ThrowIfFailed(D3D12CreateDevice((IUnknown*)adapter, minFeatureLevel, &iid, (void**)&device));
            NativeDevice = device;

            // Init capabilites.
            InitCapabilites(adapter);

            adapter->Release();
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
                uint refCount = NativeDevice->Release();
                if (refCount > 0)
                {
                    System.Diagnostics.Debug.WriteLine($"Direct3D12: There are {refCount} unreleased references left on the device");

                    Guid iid = IID_ID3D12DebugDevice;
                    using ComPtr<ID3D12DebugDevice> debugDevice = null;
                    if (SUCCEEDED(NativeDevice->QueryInterface(&iid, (void**)&debugDevice)))
                    {
                        debugDevice.Get()->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    }
                }

                //                if (DXGIGetDebugInterface1<IDXGIDebug1>(out IDXGIDebug1? dxgiDebug).Success)
                //                {
                //                    dxgiDebug.ReportLiveObjects(All, ReportLiveObjectFlags.Detail | ReportLiveObjectFlags.IgnoreInternal);
                //                    dxgiDebug.Release();
                //                }
#else
                NativeDevice->Release();
#endif

                _dxgiFactory->Release();
            }
        }

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities => _capabilities;

        private void InitCapabilites(IDXGIAdapter1* adapter)
        {
            DXGI_ADAPTER_DESC1 desc;
            ThrowIfFailed(adapter->GetDesc1(&desc));

            _capabilities.BackendType = BackendType.Direct3D12;
            _capabilities.VendorId = new GPUVendorId(desc.VendorId);
            _capabilities.AdapterId = desc.DeviceId;

            if ((desc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                _capabilities.AdapterType = GraphicsAdapterType.CPU;
            }
            else
            {
                D3D12_FEATURE_DATA_ARCHITECTURE featureDataArchitecture;
                ThrowIfFailed(NativeDevice->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &featureDataArchitecture, SizeOf<D3D12_FEATURE_DATA_ARCHITECTURE>()));
                _capabilities.AdapterType = (featureDataArchitecture.UMA == 1) ? GraphicsAdapterType.IntegratedGPU : GraphicsAdapterType.DiscreteGPU;
            }

            _capabilities.AdapterName = MarshalNullTerminatedStringUtf16(in desc.Description[0], 128).AsString() ?? string.Empty;

            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5;
            ThrowIfFailed(NativeDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, SizeOf<D3D12_FEATURE_DATA_D3D12_OPTIONS5>()));

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
                Raytracing = options5.RaytracingTier > D3D12_RAYTRACING_TIER_NOT_SUPPORTED
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
                MaxComputeWorkGroupSizeY = D3D12_CS_THREAD_GROUP_MAX_X,
                MaxComputeWorkGroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_X,
            };

            //public GPUDeviceLimits Limits;
        }
    }
}
