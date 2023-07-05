
// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D;
using Win32.Graphics.Direct3D12;
using D3D12Feature = Win32.Graphics.Direct3D12.Feature;

namespace Alimer.Graphics.D3D12;

internal unsafe readonly struct D3D12Features
{
    private readonly ID3D12Device* _device;
    // Feature support data structs
    private readonly FeatureDataD3D12Options _options;
    private readonly FeatureDataD3D12Options1 _options1;
    private readonly FeatureDataD3D12Options2 _options2;
    private readonly FeatureDataD3D12Options3 _options3;
    private readonly FeatureDataD3D12Options4 _options4;
    private readonly FeatureDataD3D12Options5 _options5;
    private readonly FeatureDataD3D12Options6 _options6;
    private readonly FeatureDataD3D12Options7 _options7;
    private readonly FeatureDataGpuVirtualAddressSupport _gpuVASupport;
    private readonly FeatureDataArchitecture1[] _architecture1;
    private readonly FeatureLevel _maxSupportedFeatureLevel;
    private readonly ShaderModel _highestShaderModel;

    public D3D12Features(ID3D12Device* device)
    {
        _device = device;

        // Initialize static feature support data structures
        if (device->CheckFeatureSupport(D3D12Feature.Options, ref _options).Failure)
        {
            _options = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options1, ref _options1).Failure)
        {
            _options1 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options2, ref _options2).Failure)
        {
            _options2 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options3, ref _options3).Failure)
        {
            _options3 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options4, ref _options4).Failure)
        {
            _options4 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options5, ref _options5).Failure)
        {
            _options5 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options6, ref _options6).Failure)
        {
            _options6 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.Options7, ref _options7).Failure)
        {
            _options7 = default;
        }

        if (device->CheckFeatureSupport(D3D12Feature.GpuVirtualAddressSupport, ref _gpuVASupport).Failure)
        {
            _gpuVASupport = default;
        }

        // Initialize per-node feature support data structures
        NodeCount = device->GetNodeCount();
        //m_dProtectedResourceSessionSupport.resize(uNodeCount);
        _architecture1 = new FeatureDataArchitecture1[NodeCount];

        for (uint nodeIndex = 0; nodeIndex < NodeCount; nodeIndex++)
        {
        }

        _maxSupportedFeatureLevel = QueryHighestFeatureLevel();
        _highestShaderModel = QueryHighestShaderModel();
    }

    public uint NodeCount { get; }
    public bool UMA(uint nodeIndex = 0) => _architecture1[nodeIndex].UMA;
    public FeatureLevel MaxSupportedFeatureLevel => _maxSupportedFeatureLevel;
    public ShaderModel HighestShaderModel => _highestShaderModel;
    public TiledResourcesTier TiledResourcesTier => _options.TiledResourcesTier;
    public bool DepthBoundsTestSupported => _options2.DepthBoundsTestSupported;
    public bool Native16BitShaderOpsSupported => _options4.Native16BitShaderOpsSupported;

    public bool SRVOnlyTiledResourceTier3 => _options5.SRVOnlyTiledResourceTier3;
    public RenderPassTier RenderPassesTier => _options5.RenderPassesTier;
    public RaytracingTier RaytracingTier => _options5.RaytracingTier;

    public bool AdditionalShadingRatesSupported => _options6.AdditionalShadingRatesSupported;
    public bool PerPrimitiveShadingRateSupportedWithViewportIndexing => _options6.PerPrimitiveShadingRateSupportedWithViewportIndexing;
    public VariableShadingRateTier VariableShadingRateTier => _options6.VariableShadingRateTier;
    public uint ShadingRateImageTileSize => _options6.ShadingRateImageTileSize;
    public bool BackgroundProcessingSupported => _options6.BackgroundProcessingSupported;

    public MeshShaderTier MeshShaderTier => _options7.MeshShaderTier;
    public SamplerFeedbackTier SamplerFeedbackTier => _options7.SamplerFeedbackTier;

    private FeatureLevel QueryHighestFeatureLevel()
    {
        // Check against a list of all feature levels present in d3dcommon.h
        // Needs to be updated for future feature levels
        ReadOnlySpan<FeatureLevel> featureLevels = stackalloc FeatureLevel[]
        {
            FeatureLevel.Level_12_2,
            FeatureLevel.Level_12_1,
            FeatureLevel.Level_12_0,
            FeatureLevel.Level_11_1,
            FeatureLevel.Level_11_0,
            FeatureLevel.Level_10_1,
            FeatureLevel.Level_10_0,
            FeatureLevel.Level_9_3,
            FeatureLevel.Level_9_2,
            FeatureLevel.Level_9_1,
            FeatureLevel.Level_1_0_Core
        };

        fixed (FeatureLevel* pFeatureLevels = featureLevels)
        {
            FeatureDataFeatureLevels dFeatureLevel = new()
            {
                NumFeatureLevels = (uint)featureLevels.Length,
                pFeatureLevelsRequested = pFeatureLevels
            };

            HResult result = _device->CheckFeatureSupport(D3D12Feature.FeatureLevels, &dFeatureLevel, sizeof(FeatureDataFeatureLevels));
            if (result.Success)
            {
                return dFeatureLevel.MaxSupportedFeatureLevel;
            }
            else
            {
                return FeatureLevel.Level_11_0;
            }
        }
    }


    private ShaderModel QueryHighestShaderModel()
    {
        // Check support in descending order
        HResult result;

        ReadOnlySpan<ShaderModel> allModelVersions = stackalloc ShaderModel[]
        {
            ShaderModel.SM_6_8,
            ShaderModel.SM_6_7,
            ShaderModel.SM_6_6,
            ShaderModel.SM_6_5,
            ShaderModel.SM_6_4,
            ShaderModel.SM_6_3,
            ShaderModel.SM_6_2,
            ShaderModel.SM_6_1,
            ShaderModel.SM_6_0,
            ShaderModel.SM_5_1
        };
        var numModelVersions = allModelVersions.Length;

        for (var i = 0; i < numModelVersions; i++)
        {
            FeatureDataShaderModel shaderModel = new()
            {
                HighestShaderModel = allModelVersions[i]
            };
            result = _device->CheckFeatureSupport(D3D12Feature.ShaderModel, &shaderModel, sizeof(FeatureDataShaderModel));
            if (result != HResult.InvalidArg)
            {
                // Indicates that the version is recognizable by the runtime and stored in the struct
                // Also terminate on unexpected error code
                if (result.Failure)
                {
                    shaderModel.HighestShaderModel = (ShaderModel)0;
                }

                return shaderModel.HighestShaderModel;
            }
        }

        // Shader model may not be supported. Continue the rest initializations
        return (ShaderModel)0;
    }
}
