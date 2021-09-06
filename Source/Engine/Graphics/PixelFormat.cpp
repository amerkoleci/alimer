// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/PixelFormat.h"

namespace Alimer
{
	const PixelFormatInfo kFormatDesc[] = {
        //        format                    name                bytes blk         kind               red   green   blue  alpha  depth  stencl signed  srgb
        { PixelFormat::Undefined,           "Undefined",        0,   0, PixelFormatKind::Integer,      false, false, false, false, false, false, false, false },
        { PixelFormat::R8UNorm,             "R8UNorm",          1,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { PixelFormat::R8SNorm,             "R8SNorm",          1,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { PixelFormat::R8UInt,              "R8UInt",           1,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { PixelFormat::R8SInt,              "R8SInt",           1,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { PixelFormat::R16UNorm,            "R16UNorm",         2,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { PixelFormat::R16SNorm,            "R16SNorm",         2,   1, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { PixelFormat::R16UInt,             "R16UInt",          2,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { PixelFormat::R16SInt,             "R16SInt",          2,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { PixelFormat::R16Float,            "R16Float",         2,   1, PixelFormatKind::Float,        true,  false, false, false, false, false, true,  false },
        { PixelFormat::RG8UNorm,            "RG8UNorm",         2,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG8SNorm,            "RG8SNorm",         2,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG8UInt,             "RG8UInt",          2,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG8SInt,             "RG8SInt",          2,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { PixelFormat::BGRA4UNorm,          "BGRA4UNorm",       2,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::B5G6R5UNorm,         "B5G6R5UNorm",      2,   1, PixelFormatKind::Normalized,   true,  true,  true,  false, false, false, false, false },
        { PixelFormat::B5G5R5A1UNorm,       "B5G5R5A1UNorm",    2,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::R32UInt,             "R32UInt",          4,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, false, false },
        { PixelFormat::R32SInt,             "R32SInt",          4,   1, PixelFormatKind::Integer,      true,  false, false, false, false, false, true,  false },
        { PixelFormat::R32Float,            "R32Float",         4,   1, PixelFormatKind::Float,        true,  false, false, false, false, false, true,  false },
        { PixelFormat::RG16UNorm,           "RG16UNorm",        4,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG16SNorm,           "RG16SNorm",        4,   1, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG16UInt,            "RG16UInt",         4,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG16SInt,            "RG16SInt",         4,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { PixelFormat::RG16Float,           "RG16Float",        4,   1, PixelFormatKind::Float,        true,  true,  false, false, false, false, true,  false },
        { PixelFormat::RGBA8UNorm,          "RGBA8UNorm",       4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA8UNormSrgb,      "RGBA8UNormSrgb",   4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { PixelFormat::RGBA8SNorm,          "RGBA8SNorm",       4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA8UInt,           "RGBA8UInt",        4,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA8SInt,           "RGBA8SInt",        4,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { PixelFormat::BGRA8UNorm,          "BGRA8UNorm",       4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::BGRA8UNormSrgb,      "BGRA8UNormSrgb",   4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGB10A2UNorm,        "RGB10A2UNorm",     4,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RG11B10Float,        "RG11B10Float",     4,   1, PixelFormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { PixelFormat::RGB9E5Float,         "RGB9E5Float",      4,   1, PixelFormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { PixelFormat::RG32UInt,            "RG32UInt",         8,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, false, false },
        { PixelFormat::RG32SInt,            "RG32SInt",         8,   1, PixelFormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
        { PixelFormat::RG32Float,           "RG32Float",        8,   1, PixelFormatKind::Float,        true,  true,  false, false, false, false, true,  false },
        { PixelFormat::RGBA16UNorm,         "RGBA16UNorm",      8,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA16SNorm,         "RGBA16SNorm",      8,   1, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA16UInt,          "RGBA16UInt",       8,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA16SInt,          "RGBA16SInt",       8,   1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { PixelFormat::RGBA16Float,         "RGBA16Float",      8,   1, PixelFormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
        //{ PixelFormat::RGB32_UINT,        "RGB32_UINT",        12,  1, PixelFormatKind::Integer,      true,  true,  true,  false, false, false, false, false },
        //{ PixelFormat::RGB32_SINT,        "RGB32_SINT",        12,  1, PixelFormatKind::Integer,      true,  true,  true,  false, false, false, true,  false },
        //{ PixelFormat::RGB32_FLOAT,       "RGB32_FLOAT",       12,  1, PixelFormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
        { PixelFormat::RGBA32UInt,          "RGBA32UInt",       16,  1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::RGBA32SInt,          "RGBA32SInt",       16,  1, PixelFormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
        { PixelFormat::RGBA32Float,         "RGBA32Float",      16,  1, PixelFormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
        { PixelFormat::Depth16UNorm,        "Depth16UNorm",     2,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
        { PixelFormat::Depth32Float,        "Depth32Float",     4,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
        { PixelFormat::Depth24UNormStencil8, "Depth24UNormStencil8",    4,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
        { PixelFormat::Depth32FloatStencil8, "Depth32FloatStencil8",    8,   1, PixelFormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
        //{ PixelFormat::X24G8_UINT,        "X24G8_UINT",        4,   1, PixelFormatKind::Integer,      false, false, false, false, false, true,  false, false },
        //{ PixelFormat::X32G8_UINT,        "X32G8_UINT",        8,   1, PixelFormatKind::Integer,      false, false, false, false, false, true,  false, false },
        { PixelFormat::BC1UNorm,            "BC1UNorm",         8,   4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::BC1UNormSrgb,        "BC1UNormSrgb",     8,   4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { PixelFormat::BC2UNorm,            "BC2UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::BC2UNormSrgb,        "BC2UNormSrgb",     16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { PixelFormat::BC3UNorm,            "BC3UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::BC3UNormSrgb,        "BC3UNormSrgb",     16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        { PixelFormat::BC4UNorm,            "BC4UNorm",         8,   4, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { PixelFormat::BC4SNorm,            "BC4SNorm",         8,   4, PixelFormatKind::Normalized,   true,  false, false, false, false, false, false, false },
        { PixelFormat::BC5UNorm,            "BC5UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { PixelFormat::BC5SNorm,            "BC5SNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
        { PixelFormat::BC6HUFloat,          "BC6HUFloat",       16,  4, PixelFormatKind::Float,        true,  true,  true,  false, false, false, false, false },
        { PixelFormat::BC6HSFloat,          "BC6HSFloat",       16,  4, PixelFormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
        { PixelFormat::BC7UNorm,            "BC7UNorm",         16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
        { PixelFormat::BC7UNormSrgb,        "BC7UNormSrgb",     16,  4, PixelFormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
	};

    const PixelFormatInfo& GetFormatInfo(PixelFormat format)
    {
        static_assert(sizeof(kFormatDesc) / sizeof(PixelFormatInfo) == size_t(PixelFormat::Count),
            "The format info table doesn't have the right number of elements");

        if (uint32_t(format) >= uint32_t(PixelFormat::Count))
            return kFormatDesc[0]; // UNKNOWN

        const PixelFormatInfo& info = kFormatDesc[uint32_t(format)];
        ALIMER_ASSERT(info.format == format);
        return info;
    }
}
