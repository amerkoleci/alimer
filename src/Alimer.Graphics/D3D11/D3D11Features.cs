// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32.Graphics.Direct3D11;
using D3D11Feature = Win32.Graphics.Direct3D11.Feature;

namespace Alimer.Graphics.D3D11;

internal unsafe class D3D11Features
{
    // Feature support data structs
    private readonly FeatureDataArchitectureInfo _architectureInfo;
    private readonly FeatureDataD3D11Options _options;
    private readonly FeatureDataD3D11Options1 _options1;
    private readonly FeatureDataD3D11Options2 _options2;
    private readonly FeatureDataD3D11Options3 _options3;
    private readonly FeatureDataD3D11Options4 _options4;
    private readonly FeatureDataD3D11Options5 _options5;

    public D3D11Features(ID3D11Device* device)
    {
        // Initialize static feature support data structures
        if (device->CheckFeatureSupport(D3D11Feature.ArchitectureInfo, ref _architectureInfo).Failure)
        {
            _architectureInfo = default;
        }

        if (device->CheckFeatureSupport(D3D11Feature.Options, ref _options).Failure)
        {
            _options = default;
        }

        if (device->CheckFeatureSupport(D3D11Feature.Options1, ref _options1).Failure)
        {
            _options1 = default;
        }

        if (device->CheckFeatureSupport(D3D11Feature.Options2, ref _options2).Failure)
        {
            _options2 = default;
        }

        if (device->CheckFeatureSupport(D3D11Feature.Options3, ref _options3).Failure)
        {
            _options3 = default;
        }

        if (device->CheckFeatureSupport(D3D11Feature.Options4, ref _options4).Failure)
        {
            _options4 = default;
        }

        if (device->CheckFeatureSupport(D3D11Feature.Options5, ref _options5).Failure)
        {
            _options5 = default;
        }
    }

    public bool UnifiedMemoryArchitecture => _options2.UnifiedMemoryArchitecture;
}
