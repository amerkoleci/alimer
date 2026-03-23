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
                case VertexFormat::Uint8:               return MTL::VertexFormatUChar;
                case VertexFormat::Uint8x2:             return MTL::VertexFormatUChar2;
                case VertexFormat::Uint8x4:             return MTL::VertexFormatUChar4;
                case VertexFormat::Sint8:               return MTL::VertexFormatChar;
                case VertexFormat::Sint8x2:             return MTL::VertexFormatChar;
                case VertexFormat::Sint8x4:             return MTL::VertexFormatChar4;
                case VertexFormat::Unorm8:              return MTL::VertexFormatUCharNormalized;
                case VertexFormat::Unorm8x2:            return MTL::VertexFormatUChar2Normalized;
                case VertexFormat::Unorm8x4:            return MTL::VertexFormatUChar4Normalized;
                case VertexFormat::Snorm8:              return MTL::VertexFormatCharNormalized;
                case VertexFormat::Snorm8x2:            return MTL::VertexFormatChar2Normalized;
                case VertexFormat::Snorm8x4:            return MTL::VertexFormatChar4Normalized;

                case VertexFormat::Uint16:              return MTL::VertexFormatUShort;
                case VertexFormat::Uint16x2:            return MTL::VertexFormatUShort2;
                case VertexFormat::Uint16x4:            return MTL::VertexFormatUShort4;
                case VertexFormat::Sint16:              return MTL::VertexFormatShort;
                case VertexFormat::Sint16x2:            return MTL::VertexFormatShort2;
                case VertexFormat::Sint16x4:            return MTL::VertexFormatShort4;
                case VertexFormat::Unorm16:             return MTL::VertexFormatUShortNormalized;
                case VertexFormat::Unorm16x2:           return MTL::VertexFormatUShort2Normalized;
                case VertexFormat::Unorm16x4:           return MTL::VertexFormatUShort4Normalized;
                case VertexFormat::Snorm16:             return MTL::VertexFormatShortNormalized;
                case VertexFormat::Snorm16x2:           return MTL::VertexFormatShort2Normalized;
                case VertexFormat::Snorm16x4:           return MTL::VertexFormatShort4Normalized;
                case VertexFormat::Float16:             return MTL::VertexFormatHalf;
                case VertexFormat::Float16x2:           return MTL::VertexFormatHalf2;
                case VertexFormat::Float16x4:           return MTL::VertexFormatHalf4;

                case VertexFormat::Float32:             return MTL::VertexFormatFloat;
                case VertexFormat::Float32x2:           return MTL::VertexFormatFloat2;
                case VertexFormat::Float32x3:           return MTL::VertexFormatFloat3;
                case VertexFormat::Float32x4:           return MTL::VertexFormatFloat4;

                case VertexFormat::Uint32:              return MTL::VertexFormatUInt;
                case VertexFormat::Uint32x2:            return MTL::VertexFormatUInt2;
                case VertexFormat::Uint32x3:            return MTL::VertexFormatUInt3;
                case VertexFormat::Uint32x4:            return MTL::VertexFormatUInt4;

                case VertexFormat::Sint32:              return MTL::VertexFormatInt;
                case VertexFormat::Sint32x2:            return MTL::VertexFormatInt2;
                case VertexFormat::Sint32x3:            return MTL::VertexFormatInt3;
                case VertexFormat::Sint32x4:            return MTL::VertexFormatInt4;

                case VertexFormat::Unorm10_10_10_2:     return MTL::VertexFormatUInt1010102Normalized;
                case VertexFormat::Unorm8x4BGRA:        return MTL::VertexFormatUChar4Normalized_BGRA;
                    //case VertexFormat::RG11B10Float:            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                    //case VertexFormat::RGB9E5Float:             return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

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
