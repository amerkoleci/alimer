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
        constexpr MTL::VertexFormat ToVkVertexFormat(VertexFormat format)
        {
            switch (format)
            {
                case VertexFormat::UByte:               return MTL::VertexFormatUChar;
                case VertexFormat::UByte2:              return MTL::VertexFormatUChar2;
                case VertexFormat::UByte4:              return MTL::VertexFormatUChar4;
                case VertexFormat::Byte:                return MTL::VertexFormatChar;
                case VertexFormat::Byte2:               return MTL::VertexFormatChar;
                case VertexFormat::Byte4:               return MTL::VertexFormatChar4;
                case VertexFormat::UByteNormalized:     return MTL::VertexFormatUCharNormalized;
                case VertexFormat::UByte2Normalized:    return MTL::VertexFormatUChar2Normalized;
                case VertexFormat::UByte4Normalized:    return MTL::VertexFormatUChar4Normalized;
                case VertexFormat::ByteNormalized:      return MTL::VertexFormatCharNormalized;
                case VertexFormat::Byte2Normalized:     return MTL::VertexFormatChar2Normalized;
                case VertexFormat::Byte4Normalized:     return MTL::VertexFormatChar4Normalized;

                case VertexFormat::UShort:              return MTL::VertexFormatUShort;
                case VertexFormat::UShort2:             return MTL::VertexFormatUShort2;
                case VertexFormat::UShort4:             return MTL::VertexFormatUShort4;
                case VertexFormat::Short:               return MTL::VertexFormatShort;
                case VertexFormat::Short2:              return MTL::VertexFormatShort2;
                case VertexFormat::Short4:              return MTL::VertexFormatShort4;
                case VertexFormat::UShortNormalized:    return MTL::VertexFormatUShortNormalized;
                case VertexFormat::UShort2Normalized:   return MTL::VertexFormatUShort2Normalized;
                case VertexFormat::UShort4Normalized:   return MTL::VertexFormatUShort4Normalized;
                case VertexFormat::ShortNormalized:     return MTL::VertexFormatShortNormalized;
                case VertexFormat::Short2Normalized:    return MTL::VertexFormatShort2Normalized;
                case VertexFormat::Short4Normalized:    return MTL::VertexFormatShort4Normalized;
                case VertexFormat::Half:                return MTL::VertexFormatHalf;
                case VertexFormat::Half2:               return MTL::VertexFormatHalf2;
                case VertexFormat::Half4:               return MTL::VertexFormatHalf4;

                case VertexFormat::Float:               return MTL::VertexFormatFloat;
                case VertexFormat::Float2:              return MTL::VertexFormatFloat2;
                case VertexFormat::Float3:              return MTL::VertexFormatFloat3;
                case VertexFormat::Float4:              return MTL::VertexFormatFloat4;

                case VertexFormat::UInt:                return MTL::VertexFormatUInt;
                case VertexFormat::UInt2:               return MTL::VertexFormatUInt2;
                case VertexFormat::UInt3:               return MTL::VertexFormatUInt3;
                case VertexFormat::UInt4:               return MTL::VertexFormatUInt4;

                case VertexFormat::Int:                 return MTL::VertexFormatInt;
                case VertexFormat::Int2:                return MTL::VertexFormatInt2;
                case VertexFormat::Int3:                return MTL::VertexFormatInt3;
                case VertexFormat::Int4:                return MTL::VertexFormatInt4;

                case VertexFormat::UInt1010102Normalized:   return MTL::VertexFormatUInt1010102Normalized;
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
