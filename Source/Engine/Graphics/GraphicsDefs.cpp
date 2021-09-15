// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/GraphicsDefs.h"

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
}
