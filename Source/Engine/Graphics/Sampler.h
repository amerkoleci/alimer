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

    struct SamplerCreateInfo
    {
        const char* label = nullptr;
        SamplerFilter minFilter = SamplerFilter::Nearest;
        SamplerFilter magFilter = SamplerFilter::Nearest;
        SamplerFilter mipFilter = SamplerFilter::Nearest;
        SamplerAddressMode addressModeU = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeV = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeW = SamplerAddressMode::Clamp;
        uint16_t maxAnisotropy = 1;
        CompareFunction compareFunction = CompareFunction::Never;
        SamplerBorderColor borderColor = SamplerBorderColor::TransparentBlack;
        float lodMinClamp = 0.0f;
        float lodMaxClamp = FLT_MAX;
    };

	class ALIMER_API Sampler : public GPUObjectOld, public RefCounted
	{
	public:
		static SamplerRef Create(const SamplerCreateInfo& info);

	protected:
		/// Constructor.
		Sampler();

        [[nodiscard]] const GPUSampler* GetHandle() const { return &handle; }

    private:
        GPUSampler handle;
	};
}
