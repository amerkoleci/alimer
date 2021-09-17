// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GPUResource.h"

namespace Alimer
{
    enum class SamplerFilter : uint32_t
    {
        Nearest,
        Linear
    };

    enum class SamplerAddressMode : uint32_t
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    };

    enum class SamplerBorderColor : uint32_t
    {
        TransparentBlack,
        OpaqueBlack,
        OpaqueWhite,
    };

    struct SamplerDesc
    {
        const char* label = nullptr;
        SamplerFilter magFilter = SamplerFilter::Nearest;
        SamplerFilter minFilter = SamplerFilter::Nearest;
        SamplerFilter mipFilter = SamplerFilter::Nearest;
        SamplerAddressMode addressModeU = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeV = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeW = SamplerAddressMode::Clamp;
        float    mipLodBias = 0.0f;
        uint16_t maxAnisotropy = 1;
        CompareFunction compareFunction = CompareFunction::Undefined;
        float minLod = 0.0f;
        float maxLod = FLT_MAX;
        SamplerBorderColor borderColor = SamplerBorderColor::TransparentBlack;
    };

	class ALIMER_API Sampler : public RefCounted
	{
	public:
        [[nodiscard]] static SamplerRef Create(const SamplerDesc& desc);

	protected:
		/// Constructor.
		Sampler();

	};
}
