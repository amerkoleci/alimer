// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice.Graphics
{
    /// <summary>
    /// Defines pixel format.
    /// </summary>
    public enum PixelFormat
    {
        Invalid = 0,
        // 8-bit pixel formats
        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,
        // 16-bit pixel formats
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,
        R16Float,
        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,
        // 32-bit pixel formats
        R32Uint,
        R32Sint,
        R32Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,
        RG16Float,
        RGBA8Unorm,
        RGBA8UnormSrgb,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        BGRA8Unorm,
        BGRA8UnormSrgb,
        // Packed 32-Bit Pixel formats
        RGB10A2Unorm,
        RG11B10Float,
        // 64-Bit Pixel Formats
        RG32Uint,
        RG32Sint,
        RG32Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        // 128-Bit Pixel Formats
        RGBA32Uint,
        RGBA32Sint,
        RGBA32Float,
        // Depth-stencil formats
        Depth16Unorm,
        Depth32Float,
        Depth24UnormStencil8,
        Depth32FloatStencil8,
        // Compressed formats
        // Compressed BC formats
        BC1RGBAUnorm,
        BC1RGBAUnormSrgb,
        BC2RGBAUnorm,
        BC2RGBAUnormSrgb,
        BC3RGBAUnorm,
        BC3RGBAUnormSrgb,
        BC4RUnorm,
        BC4RSnorm,
        BC5RGUnorm,
        BC5RGSnorm,
        BC6HRGBUfloat,
        BC6HRGBSfloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        Count
    }
}
