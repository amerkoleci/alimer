// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/GraphicsDefs.h"
#include <array>

namespace Alimer
{
    const char* GetVendorName(uint32_t vendorId)
    {
        switch (vendorId)
        {
        case KnownVendorId_AMD:
            return "AMD";
        case KnownVendorId_ImgTec:
            return "IMAGINATION";
        case KnownVendorId_Nvidia:
            return "Nvidia";
        case KnownVendorId_ARM:
            return "ARM";
        case KnownVendorId_Qualcomm:
            return "Qualcom";
        case KnownVendorId_Intel:
            return "Intel";
        default:
            return "Unknown";
        }
    }

    GPUVendorId VendorIdToAdapterVendor(uint32_t vendorId)
    {
        switch (vendorId)
        {
        case KnownVendorId_Nvidia:
            return GPUVendorId::NVIDIA;
        case KnownVendorId_AMD:
            return GPUVendorId::AMD;
        case KnownVendorId_Intel:
            return GPUVendorId::INTEL;
        case KnownVendorId_ARM:
            return GPUVendorId::ARM;
        case KnownVendorId_Qualcomm:
            return GPUVendorId::QUALCOMM;
        case KnownVendorId_ImgTec:
            return GPUVendorId::IMGTECH;
        case KnownVendorId_Microsoft:
            return GPUVendorId::MSFT;
        case KnownVendorId_Apple:
            return GPUVendorId::APPLE;
        case KnownVendorId_Mesa:
            return GPUVendorId::MESA;
        case KnownVendorId_BROADCOM:
            return GPUVendorId::BROADCOM;
        
        default:
            return GPUVendorId::Unknown;
        }
    }
    const char* ToString(GPUAdapterType type)
    {
        switch (type)
        {
        case GPUAdapterType::Software:      return "Software";
        case GPUAdapterType::Integrated:    return "Integrated";
        case GPUAdapterType::Discrete:      return "Discrete";
        default:
            return "<Unknown>";
        }
    }

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

    static constexpr std::array<VertexFormatInfo, 31> sVertexFormatTable = { {
           
            {VertexFormat::Undefined, 0, 0, 0, VertexFormatBaseType::Float},
            {VertexFormat::Uint8x2, 2, 2, 1, VertexFormatBaseType::Uint},
            {VertexFormat::Uint8x4, 4, 4, 1, VertexFormatBaseType::Uint},
            {VertexFormat::Sint8x2, 2, 2, 1, VertexFormatBaseType::Sint},
            {VertexFormat::Sint8x4, 4, 4, 1, VertexFormatBaseType::Sint},
            {VertexFormat::Unorm8x2, 2, 2, 1, VertexFormatBaseType::Float},
            {VertexFormat::Unorm8x4, 4, 4, 1, VertexFormatBaseType::Float},
            {VertexFormat::Snorm8x2, 2, 2, 1, VertexFormatBaseType::Float},
            {VertexFormat::Snorm8x4, 4, 4, 1, VertexFormatBaseType::Float},
            {VertexFormat::Uint16x2, 4, 2, 2, VertexFormatBaseType::Uint},
            {VertexFormat::Uint16x4, 8, 4, 2, VertexFormatBaseType::Uint},
            {VertexFormat::Sint16x2, 4, 2, 2, VertexFormatBaseType::Sint},
            {VertexFormat::Sint16x4, 8, 4, 2, VertexFormatBaseType::Sint},
            {VertexFormat::Unorm16x2, 4, 2, 2, VertexFormatBaseType::Float},
            {VertexFormat::Unorm16x4, 8, 4, 2, VertexFormatBaseType::Float},
            {VertexFormat::Snorm16x2, 4, 2, 2, VertexFormatBaseType::Float},
            {VertexFormat::Snorm16x4, 8, 4, 2, VertexFormatBaseType::Float},
            {VertexFormat::Float16x2, 4, 2, 2, VertexFormatBaseType::Float},
            {VertexFormat::Float16x4, 8, 4, 2, VertexFormatBaseType::Float},
            {VertexFormat::Float32, 4, 1, 4, VertexFormatBaseType::Float},
            {VertexFormat::Float32x2, 8, 2, 4, VertexFormatBaseType::Float},
            {VertexFormat::Float32x3, 12, 3, 4, VertexFormatBaseType::Float},
            {VertexFormat::Float32x4, 16, 4, 4, VertexFormatBaseType::Float},
            {VertexFormat::Uint32, 4, 1, 4, VertexFormatBaseType::Uint},
            {VertexFormat::Uint32x2, 8, 2, 4, VertexFormatBaseType::Uint},
            {VertexFormat::Uint32x3, 12, 3, 4, VertexFormatBaseType::Uint},
            {VertexFormat::Uint32x4, 16, 4, 4, VertexFormatBaseType::Uint},
            {VertexFormat::Sint32, 4, 1, 4, VertexFormatBaseType::Sint},
            {VertexFormat::Sint32x2, 8, 2, 4, VertexFormatBaseType::Sint},
            {VertexFormat::Sint32x3, 12, 3, 4, VertexFormatBaseType::Sint},
            {VertexFormat::Sint32x4, 16, 4, 4, VertexFormatBaseType::Sint},
            //
        }
    };

    const VertexFormatInfo& GetVertexFormatInfo(VertexFormat format)
    {
        ALIMER_ASSERT(format != VertexFormat::Undefined);
        ALIMER_ASSERT(static_cast<uint32_t>(format) < sVertexFormatTable.size());
        ALIMER_ASSERT(sVertexFormatTable[static_cast<uint32_t>(format)].format == format);
        return sVertexFormatTable[static_cast<uint32_t>(format)];
    }
}
