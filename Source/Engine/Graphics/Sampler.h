// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsResource.h"

namespace Alimer
{
    enum class SamplerFilter : uint32_t
    {
        Point,
        Linear
    };

    enum class SamplerAddressMode : uint32_t
    {
        Wrap = 0,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    };

    enum class SamplerBorderColor : uint32_t
    {
        TransparentBlack = 0,
        OpaqueBlack,
        OpaqueWhite,
    };

    struct SamplerDesc
    {
        SamplerFilter minFilter = SamplerFilter::Point;
        SamplerFilter magFilter = SamplerFilter::Point;
        SamplerFilter mipmapFilter = SamplerFilter::Point;
        SamplerAddressMode addressModeU = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeV = SamplerAddressMode::Clamp;
        SamplerAddressMode addressModeW = SamplerAddressMode::Clamp;
        float mipLodBias = 0.0f;
        uint16_t maxAnisotropy = 1;
        CompareFunction compare = CompareFunction::Never;
        float minLod = 0.0f;
        float maxLod = FLT_MAX;
        SamplerBorderColor borderColor = SamplerBorderColor::TransparentBlack;
    };

    class ALIMER_API Sampler : public RefCounted
    {
    public:
        /// Create new sampler.
        [[nodiscard]] static SamplerRef Create(const SamplerDesc& desc);

        [[nodiscard]] virtual u32 GetBindlessIndex() const = 0;

    protected:
        /// Constructor.
        Sampler(const SamplerDesc& desc);

        SamplerFilter minFilter;
        SamplerFilter magFilter;
        SamplerFilter mipmapFilter;
        SamplerAddressMode addressModeU;
        SamplerAddressMode addressModeV;
        SamplerAddressMode addressModeW;
        float mipLodBias;
        uint16_t maxAnisotropy;
        CompareFunction compare;
        float minLod;
        float maxLod;
        SamplerBorderColor borderColor;
    };


    ALIMER_API const char* ToString(SamplerFilter filter);
    ALIMER_API const char* ToString(SamplerAddressMode mode);
    ALIMER_API const char* ToString(SamplerBorderColor borderColor);
}

