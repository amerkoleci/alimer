// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"

#if defined(ALIMER_RHI_METAL)
#include "Core/Log.h"
#include "Core/Vector.h"
#include "Core/UnorderedMap.h"
#include "Core/Hash.h"
#include "RHI/RHI.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <array>
#include <mutex>
#include <deque>
#include <memory>
#include <sstream>

namespace Alimer
{
    namespace
    {
        constexpr MTL::VertexFormat ToVkVertexFormat(VertexAttributeFormat format)
        {
            switch (format)
            {
                case VertexAttributeFormat::Uint8:               return MTL::VertexFormatUChar;
                case VertexAttributeFormat::Uint8x2:             return MTL::VertexFormatUChar2;
                case VertexAttributeFormat::Uint8x4:             return MTL::VertexFormatUChar4;
                case VertexAttributeFormat::Sint8:               return MTL::VertexFormatChar;
                case VertexAttributeFormat::Sint8x2:             return MTL::VertexFormatChar;
                case VertexAttributeFormat::Sint8x4:             return MTL::VertexFormatChar4;
                case VertexAttributeFormat::Unorm8:              return MTL::VertexFormatUCharNormalized;
                case VertexAttributeFormat::Unorm8x2:            return MTL::VertexFormatUChar2Normalized;
                case VertexAttributeFormat::Unorm8x4:            return MTL::VertexFormatUChar4Normalized;
                case VertexAttributeFormat::Snorm8:              return MTL::VertexFormatCharNormalized;
                case VertexAttributeFormat::Snorm8x2:            return MTL::VertexFormatChar2Normalized;
                case VertexAttributeFormat::Snorm8x4:            return MTL::VertexFormatChar4Normalized;

                case VertexAttributeFormat::Uint16:              return MTL::VertexFormatUShort;
                case VertexAttributeFormat::Uint16x2:            return MTL::VertexFormatUShort2;
                case VertexAttributeFormat::Uint16x4:            return MTL::VertexFormatUShort4;
                case VertexAttributeFormat::Sint16:              return MTL::VertexFormatShort;
                case VertexAttributeFormat::Sint16x2:            return MTL::VertexFormatShort2;
                case VertexAttributeFormat::Sint16x4:            return MTL::VertexFormatShort4;
                case VertexAttributeFormat::Unorm16:             return MTL::VertexFormatUShortNormalized;
                case VertexAttributeFormat::Unorm16x2:           return MTL::VertexFormatUShort2Normalized;
                case VertexAttributeFormat::Unorm16x4:           return MTL::VertexFormatUShort4Normalized;
                case VertexAttributeFormat::Snorm16:             return MTL::VertexFormatShortNormalized;
                case VertexAttributeFormat::Snorm16x2:           return MTL::VertexFormatShort2Normalized;
                case VertexAttributeFormat::Snorm16x4:           return MTL::VertexFormatShort4Normalized;
                case VertexAttributeFormat::Float16:             return MTL::VertexFormatHalf;
                case VertexAttributeFormat::Float16x2:           return MTL::VertexFormatHalf2;
                case VertexAttributeFormat::Float16x4:           return MTL::VertexFormatHalf4;

                case VertexAttributeFormat::Float32:             return MTL::VertexFormatFloat;
                case VertexAttributeFormat::Float32x2:           return MTL::VertexFormatFloat2;
                case VertexAttributeFormat::Float32x3:           return MTL::VertexFormatFloat3;
                case VertexAttributeFormat::Float32x4:           return MTL::VertexFormatFloat4;

                case VertexAttributeFormat::Uint32:              return MTL::VertexFormatUInt;
                case VertexAttributeFormat::Uint32x2:            return MTL::VertexFormatUInt2;
                case VertexAttributeFormat::Uint32x3:            return MTL::VertexFormatUInt3;
                case VertexAttributeFormat::Uint32x4:            return MTL::VertexFormatUInt4;

                case VertexAttributeFormat::Sint32:              return MTL::VertexFormatInt;
                case VertexAttributeFormat::Sint32x2:            return MTL::VertexFormatInt2;
                case VertexAttributeFormat::Sint32x3:            return MTL::VertexFormatInt3;
                case VertexAttributeFormat::Sint32x4:            return MTL::VertexFormatInt4;

                case VertexAttributeFormat::Unorm10_10_10_2:     return MTL::VertexFormatUInt1010102Normalized;
                case VertexAttributeFormat::Unorm8x4BGRA:        return MTL::VertexFormatUChar4Normalized_BGRA;
                    //case VertexAttributeFormat::RG11B10Float:            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                    //case VertexAttributeFormat::RGB9E5Float:             return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

                default:
                    return MTL::VertexFormatInvalid;
            }
        }

    }

    bool Metal_IsSupported()
    {
        return false;
    }

    RHIFactoryRef Metal_CreateFactory(const RHIFactoryDesc& desc)
    {
        // TODO:
        return nullptr;
    }
}
#endif /* defined(ALIMER_RHI_METAL) */
