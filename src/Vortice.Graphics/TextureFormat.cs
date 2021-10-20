// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    /// <summary>
    /// Defines texture format.
    /// </summary>
    public enum TextureFormat
    {
        Invalid = 0,
        // 8-bit pixel formats
        R8UNorm,
        R8SNorm,
        R8UInt,
        R8SInt,
        // 16-bit pixel formats
        R16UNorm,
        R16SNorm,
        R16UInt,
        R16SInt,
        R16Float,
        RG8UNorm,
        RG8SNorm,
        RG8UInt,
        RG8SInt,
        // 32-bit pixel formats
        R32UInt,
        R32SInt,
        R32Float,
        RG16UNorm,
        RG16SNorm,
        RG16UInt,
        RG16SInt,
        RG16Float,
        RGBA8UNorm,
        RGBA8UNormSrgb,
        RGBA8SNorm,
        RGBA8UInt,
        RGBA8SInt,
        BGRA8UNorm,
        BGRA8UNormSrgb,
        // Packed 32-Bit Pixel formats
        RGB10A2UNorm,
        RG11B10Float,
        RGB9E5Float,
        // 64-Bit Pixel Formats
        RG32UInt,
        RG32SInt,
        RG32Float,
        RGBA16UNorm,
        RGBA16SNorm,
        RGBA16UInt,
        RGBA16SInt,
        RGBA16Float,
        // 128-Bit Pixel Formats
        RGBA32UInt,
        RGBA32SInt,
        RGBA32Float,
        // Depth-stencil formats
        Depth16UNorm,
        Depth32Float,
        Depth24UNormStencil8,
        Depth32FloatStencil8,
        // Compressed BC formats
        BC1RGBAUNorm,
        BC1RGBAUNormSrgb,
        BC2RGBAUNorm,
        BC2RGBAUNormSrgb,
        BC3RGBAUNorm,
        BC3RGBAUNormSrgb,
        BC4RUNorm,
        BC4RSNorm,
        BC5RGUNorm,
        BC5RGSNorm,
        BC6HRGBUFloat,
        BC6HRGBFloat,
        BC7RGBAUNorm,
        BC7RGBAUNormSrgb,

        Count
    }
}
