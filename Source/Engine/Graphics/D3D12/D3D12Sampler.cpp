// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12Sampler.h"
#include "D3D12Graphics.h"

namespace Alimer
{
    namespace
    {
        [[nodiscard]] constexpr D3D12_FILTER_TYPE ToD3D12FilterType(SamplerFilter value)
        {
            switch (value)
            {
                case SamplerFilter::Nearest:
                    return D3D12_FILTER_TYPE_POINT;
                case SamplerFilter::Linear:
                    return D3D12_FILTER_TYPE_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_FILTER_TYPE_POINT;
            }
        }

        [[nodiscard]] constexpr D3D12_TEXTURE_ADDRESS_MODE ToD3D12AddressMode(SamplerAddressMode mode)
        {
            switch (mode) {
                case SamplerAddressMode::Wrap:
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case SamplerAddressMode::Mirror:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case SamplerAddressMode::Clamp:
                    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case SamplerAddressMode::Border:
                    return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                case SamplerAddressMode::MirrorOnce:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
        }
    }

	D3D12Sampler::D3D12Sampler(D3D12Graphics& device, const SamplerCreateInfo* info)
		: Sampler()
		, device{ device }
	{
        const D3D12_FILTER_TYPE minFilter = ToD3D12FilterType(info->minFilter);
        const D3D12_FILTER_TYPE magFilter = ToD3D12FilterType(info->magFilter);
        const D3D12_FILTER_TYPE mipFilter = ToD3D12FilterType(info->mipFilter);

        D3D12_FILTER_REDUCTION_TYPE reduction = info->compareFunction == CompareFunction::Never
            ? D3D12_FILTER_REDUCTION_TYPE_STANDARD
            : D3D12_FILTER_REDUCTION_TYPE_COMPARISON;

        D3D12_SAMPLER_DESC desc{};

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        if (info->maxAnisotropy > 1)
        {
            desc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
        }
        else
        {
            desc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, reduction);
        }

        desc.AddressU = ToD3D12AddressMode(info->addressModeU);
        desc.AddressV = ToD3D12AddressMode(info->addressModeV);
        desc.AddressW = ToD3D12AddressMode(info->addressModeW);
        desc.MipLODBias = 0.f;
        desc.MaxAnisotropy = Min<UINT>(info->maxAnisotropy, 16u);
        if (info->compareFunction != CompareFunction::Never)
        {
            desc.ComparisonFunc = ToD3D12ComparisonFunc(info->compareFunction);
        }
        else
        {
            desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }

        desc.MinLOD = info->lodMinClamp;
        desc.MaxLOD = info->lodMaxClamp;
	}

	D3D12Sampler::~D3D12Sampler()
	{
		Destroy();
	}

	void D3D12Sampler::Destroy()
	{
	}
}
