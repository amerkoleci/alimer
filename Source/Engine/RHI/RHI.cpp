// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "RHI.h"
#include "Window.h"
#include "Core/Log.h"

namespace Alimer::rhi
{
    const char* ToString(CompareFunction func)
    {
        switch (func)
        {
            case CompareFunction::Never:        return "Never";
            case CompareFunction::Less:         return "Less";
            case CompareFunction::Equal:        return "Equal";
            case CompareFunction::LessEqual:    return "LessEqual";
            case CompareFunction::Greater:      return "Greater";
            case CompareFunction::NotEqual:     return "NotEqual";
            case CompareFunction::GreaterEqual: return "GreaterEqual";
            case CompareFunction::Always:       return "Always";
            default:
                ALIMER_UNREACHABLE();
                return "<Unknown>";
        }
    }

    bool StencilTestEnabled(const DepthStencilState* depthStencil)
    {
        return depthStencil->backFace.compare != CompareFunction::Always ||
            depthStencil->backFace.failOp != StencilOperation::Keep ||
            depthStencil->backFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->backFace.passOp != StencilOperation::Keep ||
            depthStencil->frontFace.compare != CompareFunction::Always ||
            depthStencil->frontFace.failOp != StencilOperation::Keep ||
            depthStencil->frontFace.depthFailOp != StencilOperation::Keep ||
            depthStencil->frontFace.passOp != StencilOperation::Keep;
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
