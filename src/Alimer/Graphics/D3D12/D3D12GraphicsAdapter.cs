// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Utilities.MarshalUtilities;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.DirectX.D3D_SHADER_MODEL;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_CONSERVATIVE_RASTERIZATION_TIER;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.D3D12_FORMAT_SUPPORT1;
using static TerraFX.Interop.DirectX.D3D12_MESH_SHADER_TIER;
using static TerraFX.Interop.DirectX.D3D12_RAYTRACING_TIER;
using static TerraFX.Interop.DirectX.D3D12_TILED_RESOURCES_TIER;
using static TerraFX.Interop.DirectX.D3D12_VARIABLE_SHADING_RATE_TIER;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BINDING_TIER;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12GraphicsAdapter : GraphicsAdapter, IDisposable
{
    private readonly ComPtr<IDXGIAdapter1> _handle;
    private readonly bool _featureBGRA8UnormStorage;

    public D3D12GraphicsAdapter(
        D3D12GraphicsManager manager,
        ComPtr<IDXGIAdapter1> handle,
        ID3D12Device* device)
        : base(manager)
    {
        _handle = handle.Move();

        DXGI_ADAPTER_DESC1 adapterDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&adapterDesc));

        // Init features
        Features = new D3D12Features(device);

        uint MaxNonSamplerDescriptors = 0;
        uint MaxSamplerDescriptors = 0;
        D3D12_FEATURE_DATA_D3D12_OPTIONS19 options19 = default;
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, ref options19)))
        {
            if (Features.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
            }
            else if (Features.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_2)
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
            else
            {
                MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
            }
            MaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        }
        else
        {
            MaxNonSamplerDescriptors = options19.MaxViewDescriptorHeapSize;
            MaxSamplerDescriptors = options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers;
        }

        // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
        string driverDescription = string.Empty;
        LARGE_INTEGER umdVersion;
        if (_handle.Get()->CheckInterfaceSupport(__uuidof<IDXGIDevice>(), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
        {
            driverDescription = "D3D12 driver version ";

            long encodedVersion = umdVersion.QuadPart;
            for (int i = 0; i < 4; ++i)
            {
                ushort driverVersion = (ushort)((encodedVersion >> (48 - 16 * i)) & 0xFFFF);
                driverDescription += $"{driverVersion}.";
            }
        }

        DeviceName = GetUtf16Span(in adapterDesc.Description[0], 128).GetString() ?? string.Empty;
        VendorId = adapterDesc.VendorId;
        DeviceId = adapterDesc.DeviceId;

        // Detect adapter type.
        Type = GraphicsAdapterType.Other;
        if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
        {
            Type = GraphicsAdapterType.Cpu;
        }
        else
        {
            Type = Features.UMA() ? GraphicsAdapterType.IntegratedGpu : GraphicsAdapterType.DiscreteGpu;
        }

        D3D12_FEATURE_DATA_FORMAT_SUPPORT bgra8unormFormatInfo = default;
        bgra8unormFormatInfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &bgra8unormFormatInfo, (uint)sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
        if (hr.SUCCEEDED &&
            (bgra8unormFormatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0)
        {
            _featureBGRA8UnormStorage = true;
        }
    }

    public D3D12GraphicsManager DxManager => (D3D12GraphicsManager)base.Manager;

    public IDXGIAdapter1* Handle => _handle;
    public D3D12Features Features { get; }

    /// <inheritdoc />
    public override string DeviceName { get; }

    /// <inheritdoc />
    public override uint VendorId { get; }

    /// <inheritdoc />
    public override uint DeviceId { get; }

    /// <inheritdoc />
    public override GraphicsAdapterType Type { get; }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            // Always supported features
            case Feature.Depth32FloatStencil8:
            case Feature.TimestampQuery:
            case Feature.PipelineStatisticsQuery:
            case Feature.TextureCompressionBC:
            case Feature.IndirectFirstInstance:
            case Feature.SamplerClampToBorder:
            case Feature.SamplerMirrorClampToEdge:
            case Feature.DepthResolveMinMax:
            case Feature.StencilResolveMinMax:
            case Feature.Predication:
                return true;

            // Always unsupported features
            case Feature.TextureCompressionETC2:
            case Feature.TextureCompressionASTC:
            case Feature.TextureCompressionASTC_HDR:
                return false;

            case Feature.ShaderFloat16:
                //const bool supportsDP4a = d3dFeatures.HighestShaderModel() >= D3D_SHADER_MODEL_6_4;
                return Features.HighestShaderModel >= D3D_SHADER_MODEL_6_2 && Features.Native16BitShaderOpsSupported;

            case Feature.RG11B10UfloatRenderable:
                return true;

            case Feature.BGRA8UnormStorage:
                return _featureBGRA8UnormStorage;

            case Feature.DepthBoundsTest:
                return Features.DepthBoundsTestSupported;

            case Feature.SamplerMinMax:
                if (Features.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_2)
                {
                    // Tier 2 for tiled resources
                    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/tiled-resources-texture-sampling-features
                }

                return (Features.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_1);

            case Feature.ConservativeRasterization:
                return Features.ConservativeRasterizationTier != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;

            case Feature.CacheCoherentUMA:
                return Features.CacheCoherentUMA();

            case Feature.DescriptorIndexing:
                return true;

            case Feature.VariableRateShading:
                return (Features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1);

            case Feature.VariableRateShadingTier2:
                return (Features.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2);

            case Feature.RayTracing:
                return (Features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

            case Feature.RayTracingTier2:
                return (Features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1);

            case Feature.MeshShader:
                return (Features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1);

            default:
                return false;
        }
    }

    /// <inheritdoc />
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format)
    {
        // TODO:
        PixelFormatSupport result = PixelFormatSupport.None;
        return result;
    }

#if TODO
    /// <inheritdoc />
    public override bool QueryVertexFormatSupport(VertexFormat format)
    {
        // TODO:
        return false;
    } 
#endif

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new D3D12GraphicsDevice(this, description);

    public void Dispose()
    {
        _handle.Dispose();
    }
}
