// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Sampler.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    Sampler::Sampler(const SamplerDesc& desc)
        : minFilter(desc.minFilter)
        , magFilter(desc.magFilter)
        , mipmapFilter(desc.mipmapFilter)
        , addressModeU(desc.addressModeU)
        , addressModeV(desc.addressModeV)
        , addressModeW(desc.addressModeW)
        , mipLodBias(desc.mipLodBias)
        , maxAnisotropy(desc.maxAnisotropy)
        , compare(desc.compare)
        , minLod(desc.minLod)
        , maxLod(desc.maxLod)
        , borderColor(desc.borderColor)
    {
    }

    SamplerRef Sampler::Create(const SamplerDesc& desc)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return gGraphics().CreateSampler(desc);
    }

    const char* ToString(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::Point:  return "Point";
            case SamplerFilter::Linear: return "Linear";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

    const char* ToString(SamplerAddressMode mode)
    {
        switch (mode)
        {
            case SamplerAddressMode::Wrap:          return "Wrap";
            case SamplerAddressMode::Mirror:        return "Mirror";
            case SamplerAddressMode::Clamp:         return "Clamp";
            case SamplerAddressMode::Border:        return "Border";
            case SamplerAddressMode::MirrorOnce:    return "MirrorOnce";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

    const char* ToString(SamplerBorderColor borderColor)
    {
        switch (borderColor)
        {
            case SamplerBorderColor::TransparentBlack:  return "TransparentBlack";
            case SamplerBorderColor::OpaqueBlack:       return "OpaqueBlack";
            case SamplerBorderColor::OpaqueWhite:       return "OpaqueWhite";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

}
