// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_FEATURE;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static TerraFX.Interop.DirectX.D3D_SHADER_MODEL;
using static TerraFX.Interop.DirectX.D3D_ROOT_SIGNATURE_VERSION;
using static TerraFX.Interop.Windows.E;
using TerraFX.Interop.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe readonly struct D3D12Features
{
    private readonly ID3D12Device* _device;
    // Feature support data structs
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS _options;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS1 _options1;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS2 _options2;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS3 _options3;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS4 _options4;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS5 _options5;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS6 _options6;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS7 _options7;
    private readonly D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT _gpuVASupport;
    private readonly D3D12_FEATURE_DATA_ARCHITECTURE1[] _architecture1;
#if TODO
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS8 _options8;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS9 _options9;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS10 _options10;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS11 _options11;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS12 _options12;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS13 _options13;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS14 _options14;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS15 _options15;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS16 _options16;
    private readonly D3D12_FEATURE_DATA_D3D12_OPTIONS17 _options17;
#endif

    public D3D12Features(ID3D12Device* device)
    {
        _device = device;

        // Initialize static feature support data structures
        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, ref _options).FAILED)
        {
            _options = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, ref _options1).FAILED)
        {
            _options1 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, ref _options2).FAILED)
        {
            _options2 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, ref _options3).FAILED)
        {
            _options3 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, ref _options4).FAILED)
        {
            _options4 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, ref _options5).FAILED)
        {
            _options5 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, ref _options6).FAILED)
        {
            _options6 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, ref _options7).FAILED)
        {
            _options7 = default;
        }

#if TODO
        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS8, ref _options8).FAILED)
        {
            _options8 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, ref _options9).FAILED)
        {
            _options9 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS10, ref _options10).FAILED)
        {
            _options10 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS11, ref _options11).FAILED)
        {
            _options11 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, ref _options12).FAILED)
        {
            _options12 = default;
        }

        if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS13, ref _options13).FAILED)
        {
            _options13 = default;
        } 
#endif

        if (device->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, ref _gpuVASupport).FAILED)
        {
            _gpuVASupport = default;
        }

        // Initialize per-node feature support data structures
        NodeCount = device->GetNodeCount();
        _architecture1 = new D3D12_FEATURE_DATA_ARCHITECTURE1[NodeCount];

        for (uint nodeIndex = 0; nodeIndex < NodeCount; nodeIndex++)
        {
            _architecture1[nodeIndex].NodeIndex = nodeIndex;
            if (device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, ref _architecture1[nodeIndex]).FAILED)
            {
                D3D12_FEATURE_DATA_ARCHITECTURE dataArchLocal = default;
                dataArchLocal.NodeIndex = nodeIndex;
                if (_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &dataArchLocal, (uint)sizeof(D3D12_FEATURE_DATA_ARCHITECTURE)).FAILED)
                {
                    dataArchLocal.TileBasedRenderer = false;
                    dataArchLocal.UMA = false;
                    dataArchLocal.CacheCoherentUMA = false;
                }

                _architecture1[nodeIndex].TileBasedRenderer = dataArchLocal.TileBasedRenderer;
                _architecture1[nodeIndex].UMA = dataArchLocal.UMA;
                _architecture1[nodeIndex].CacheCoherentUMA = dataArchLocal.CacheCoherentUMA;
                _architecture1[nodeIndex].IsolatedMMU = false;
            }
        }

        MaxSupportedFeatureLevel = QueryHighestFeatureLevel();
        RootSignatureHighestVersion = QueryHighestRootSignatureVersion();
        HighestShaderModel = QueryHighestShaderModel();
    }

    public uint NodeCount { get; }
    public bool TileBasedRenderer(uint nodeIndex = 0) => _architecture1[nodeIndex].TileBasedRenderer;
    public bool UMA(uint nodeIndex = 0) => _architecture1[nodeIndex].UMA;
    public bool CacheCoherentUMA(uint nodeIndex = 0) => _architecture1[nodeIndex].CacheCoherentUMA;
    public bool IsolatedMMU(uint nodeIndex = 0) => _architecture1[nodeIndex].IsolatedMMU;
    public D3D_FEATURE_LEVEL MaxSupportedFeatureLevel { get; }
    public D3D_ROOT_SIGNATURE_VERSION RootSignatureHighestVersion { get; }
    public D3D_SHADER_MODEL HighestShaderModel { get; }
    public D3D12_TILED_RESOURCES_TIER TiledResourcesTier => _options.TiledResourcesTier;
    public D3D12_CONSERVATIVE_RASTERIZATION_TIER ConservativeRasterizationTier => _options.ConservativeRasterizationTier;
    public bool DepthBoundsTestSupported => _options2.DepthBoundsTestSupported;
    public bool Native16BitShaderOpsSupported => _options4.Native16BitShaderOpsSupported;

    public bool SRVOnlyTiledResourceTier3 => _options5.SRVOnlyTiledResourceTier3;
    public D3D12_RENDER_PASS_TIER RenderPassesTier => _options5.RenderPassesTier;
    public D3D12_RAYTRACING_TIER RaytracingTier => _options5.RaytracingTier;

    public bool AdditionalShadingRatesSupported => _options6.AdditionalShadingRatesSupported;
    public bool PerPrimitiveShadingRateSupportedWithViewportIndexing => _options6.PerPrimitiveShadingRateSupportedWithViewportIndexing;
    public D3D12_VARIABLE_SHADING_RATE_TIER VariableShadingRateTier => _options6.VariableShadingRateTier;
    public uint ShadingRateImageTileSize => _options6.ShadingRateImageTileSize;
    public bool BackgroundProcessingSupported => _options6.BackgroundProcessingSupported;

    public D3D12_MESH_SHADER_TIER MeshShaderTier => _options7.MeshShaderTier;
    public D3D12_SAMPLER_FEEDBACK_TIER SamplerFeedbackTier => _options7.SamplerFeedbackTier;

    private D3D_FEATURE_LEVEL QueryHighestFeatureLevel()
    {
        // Check against a list of all feature levels present in d3dcommon.h
        // Needs to be updated for future feature levels
        ReadOnlySpan<D3D_FEATURE_LEVEL> featureLevels = stackalloc D3D_FEATURE_LEVEL[]
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_1_0_CORE
        };

        fixed (D3D_FEATURE_LEVEL* pFeatureLevels = featureLevels)
        {
            D3D12_FEATURE_DATA_FEATURE_LEVELS dFeatureLevel = new()
            {
                NumFeatureLevels = (uint)featureLevels.Length,
                pFeatureLevelsRequested = pFeatureLevels
            };

            HRESULT result = _device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &dFeatureLevel, (uint)sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS));
            if (result.SUCCEEDED)
            {
                return dFeatureLevel.MaxSupportedFeatureLevel;
            }

            return D3D_FEATURE_LEVEL_11_0;
        }
    }

    private D3D_ROOT_SIGNATURE_VERSION QueryHighestRootSignatureVersion()
    {
        // Check against a list of all feature levels present in d3dcommon.h
        // Needs to be updated for future feature levels
        ReadOnlySpan<D3D_ROOT_SIGNATURE_VERSION> allRootSignatureVersions = stackalloc D3D_ROOT_SIGNATURE_VERSION[]
        {
            //(D3D_ROOT_SIGNATURE_VERSION)0x3, //D3D_ROOT_SIGNATURE_VERSION_1_2,
            D3D_ROOT_SIGNATURE_VERSION_1_1,
            D3D_ROOT_SIGNATURE_VERSION_1_0,
        };

        for (int i = 0; i < allRootSignatureVersions.Length; i++)
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSignatureData = new()
            {
                HighestVersion = allRootSignatureVersions[i]
            };

            HRESULT result = _device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignatureData, (uint)sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE));
            if (result != E_INVALIDARG)
            {
                if (result.FAILED)
                {
                    rootSignatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
                }

                // If succeeded, the highest version is already written into the member struct
                return rootSignatureData.HighestVersion;
            }
        }

        return D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    private D3D_SHADER_MODEL QueryHighestShaderModel()
    {
        // Check support in descending order
        ReadOnlySpan<D3D_SHADER_MODEL> allModelVersions = stackalloc D3D_SHADER_MODEL[]
        {
            D3D_SHADER_MODEL_6_8,
            D3D_SHADER_MODEL_6_7,
            D3D_SHADER_MODEL_6_6,
            D3D_SHADER_MODEL_6_5,
            D3D_SHADER_MODEL_6_4,
            D3D_SHADER_MODEL_6_3,
            D3D_SHADER_MODEL_6_2,
            D3D_SHADER_MODEL_6_1,
            D3D_SHADER_MODEL_6_0,
            D3D_SHADER_MODEL_5_1
        };
        var numModelVersions = allModelVersions.Length;

        for (int i = 0; i < numModelVersions; i++)
        {
            D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = new()
            {
                HighestShaderModel = allModelVersions[i]
            };
            HRESULT result = _device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, (uint)sizeof(D3D12_FEATURE_DATA_SHADER_MODEL));
            if (result != E_INVALIDARG)
            {
                // Indicates that the version is recognizable by the runtime and stored in the struct
                // Also terminate on unexpected error code
                if (result.FAILED)
                {
                    shaderModel.HighestShaderModel = (D3D_SHADER_MODEL)0;
                }

                return shaderModel.HighestShaderModel;
            }
        }

        // Shader model may not be supported. Continue the rest initializations
        return (D3D_SHADER_MODEL)0;
    }

    // Until TerraFX supports those
#if TODO
    public partial struct D3D12_FEATURE_DATA_D3D12_OPTIONS16
    {
        public BOOL DynamicDepthBiasSupported;
        public BOOL GPUUploadHeapSupported;
    }

    public partial struct D3D12_FEATURE_DATA_D3D12_OPTIONS17
    {
        public BOOL NonNormalizedCoordinateSamplersSupported;
        public BOOL ManualWriteTrackingResourceSupported;
    }

    public partial struct D3D12_FEATURE_DATA_D3D12_OPTIONS18
    {
        public BOOL RenderPassesValid;
    }

    public partial struct D3D12_FEATURE_DATA_D3D12_OPTIONS19
    {
        public BOOL MismatchingOutputDimensionsSupported;
        public uint SupportedSampleCountsWithNoOutputs;
        public BOOL PointSamplingAddressesNeverRoundUp;
        public BOOL RasterizerDesc2Supported;
        public BOOL NarrowQuadrilateralLinesSupported;
        public BOOL AnisoFilterWithPointMipSupported;
        public uint MaxSamplerDescriptorHeapSize;
        public uint MaxSamplerDescriptorHeapSizeWithStaticSamplers;
        public uint MaxViewDescriptorHeapSize;
        public BOOL ComputeOnlyCustomHeapSupported;
    } 
#endif
}
