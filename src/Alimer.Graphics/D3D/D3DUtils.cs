// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Collections;
using System.Collections.Generic;
using Vortice.DXGI;

namespace Alimer.Graphics
{
    internal static class D3DUtils
    {
        private static readonly FormatMap<PixelFormat, Format> _formatsMap = new FormatMap<PixelFormat, Format>
        {
            { PixelFormat.Invalid,        Format.Unknown },
            // 8-bit pixel formats
            { PixelFormat.R8Unorm,          Format.R8_UNorm },
            { PixelFormat.R8Snorm,          Format.R8_UNorm },
            { PixelFormat.R8Uint,           Format.R8_UNorm },
            { PixelFormat.R8Sint,           Format.R8_UNorm },
            // 16-bit pixel formats
            { PixelFormat.R16Unorm,         Format.R16_UNorm },
            { PixelFormat.R16Snorm,         Format.R16_SNorm },
            { PixelFormat.R16Uint,          Format.R16_UInt },
            { PixelFormat.R16Sint,          Format.R16_SInt },
            { PixelFormat.R16Float,         Format.R16_Float },
            { PixelFormat.RG8Unorm,         Format.R8G8_UNorm },
            { PixelFormat.RG8Snorm,         Format.R8G8_SNorm },
            { PixelFormat.RG8Uint,          Format.R8G8_UInt },
            { PixelFormat.RG8Sint,          Format.R8G8_SInt },
            // Packed 16-bit pixel formats
            //{ PixelFormat.B5G6R5UNorm,      Format.B5G6R5_UNorm },
            //{ PixelFormat.BGRA4UNorm,       Format.B4G4R4A4_UNorm },
            // 32-bit pixel formats
            { PixelFormat.R32Uint,          Format.R32_UInt },
            { PixelFormat.R32Sint,          Format.R32_SInt },
            { PixelFormat.R32Float,         Format.R32_Float },
            { PixelFormat.RG16Unorm,        Format.R16G16_UNorm },
            { PixelFormat.RG16Snorm,        Format.R16G16_SNorm },
            { PixelFormat.RG16Uint,         Format.R16G16_UInt },
            { PixelFormat.RG16Sint,         Format.R16G16_SInt },
            { PixelFormat.RG16Float,        Format.R16G16_Float },
            { PixelFormat.RGBA8Unorm,       Format.R8G8B8A8_UNorm },
            { PixelFormat.RGBA8UnormSrgb,   Format.R8G8B8A8_UNorm_SRgb },
            { PixelFormat.RGBA8Snorm,       Format.R8G8B8A8_SNorm },
            { PixelFormat.RGBA8Uint,        Format.R8G8B8A8_UInt },
            { PixelFormat.RGBA8Sint,        Format.R8G8B8A8_SInt },
            { PixelFormat.BGRA8Unorm,       Format.B8G8R8A8_UNorm },
            { PixelFormat.BGRA8UnormSrgb,   Format.B8G8R8A8_UNorm_SRgb },
            // Packed 32-Bit Pixel formats
            { PixelFormat.RGB10A2Unorm,     Format.R10G10B10A2_UNorm },
            { PixelFormat.RG11B10Float,     Format.R11G11B10_Float },
            // 64-Bit Pixel Formats
            { PixelFormat.RG32Uint,         Format.R32G32_UInt },
            { PixelFormat.RG32Sint,         Format.R32G32_SInt },
            { PixelFormat.RG32Float,        Format.R32G32_Float },
            { PixelFormat.RGBA16Unorm,      Format.R16G16B16A16_UNorm },
            { PixelFormat.RGBA16Snorm,      Format.R16G16B16A16_SNorm },
            { PixelFormat.RGBA16Uint,       Format.R16G16B16A16_UInt },
            { PixelFormat.RGBA16Sint,       Format.R16G16B16A16_SInt },
            { PixelFormat.RGBA16Float,      Format.R16G16B16A16_Float },
            // 128-Bit Pixel Formats
            { PixelFormat.RGBA32Uint,       Format.R32G32B32A32_UInt },
            { PixelFormat.RGBA32Sint,       Format.R32G32B32A32_SInt },
            { PixelFormat.RGBA32Float,      Format.R32G32B32A32_Float },
            // Depth-stencil
            { PixelFormat.Depth16Unorm,     Format.D16_UNorm },
            { PixelFormat.Depth32Float,     Format.D32_Float },
            { PixelFormat.Depth24UnormStencil8, Format.D24_UNorm_S8_UInt },
            //{ PixelFormat.Depth32FloatStencil8, Format.D32_Float_S8X24_UInt },
            // Compressed BC formats
            { PixelFormat.BC1RGBAUnorm,     Format.BC1_UNorm },
            { PixelFormat.BC1RGBAUnormSrgb, Format.BC1_UNorm_SRgb },
            { PixelFormat.BC2RGBAUnorm,     Format.BC2_UNorm },
            { PixelFormat.BC2RGBAUnormSrgb, Format.BC2_UNorm_SRgb },
            { PixelFormat.BC3RGBAUnorm,     Format.BC3_UNorm },
            { PixelFormat.BC3RGBAUnormSrgb, Format.BC3_UNorm_SRgb },
            { PixelFormat.BC4RUnorm,     Format.BC4_UNorm },
            { PixelFormat.BC4RSnorm,     Format.BC4_SNorm },
            { PixelFormat.BC5RGUnorm,     Format.BC5_UNorm },
            { PixelFormat.BC5RGSnorm,     Format.BC5_SNorm },
            { PixelFormat.BC6HRGBSfloat,      Format.BC6H_Sf16 },
            { PixelFormat.BC6HRGBUfloat,      Format.BC6H_Uf16 },
            { PixelFormat.BC7RGBAUnorm,     Format.BC7_UNorm },
            { PixelFormat.BC7RGBAUnormSrgb, Format.BC7_UNorm_SRgb },
            // Compressed PVRTC Pixel Formats
            //{ PixelFormat.PVRTC_RGB2,   Format.Unknown },
            //{ PixelFormat.PVRTC_RGBA2,  Format.Unknown },
            //{ PixelFormat.PVRTC_RGB4,   Format.Unknown },
            //{ PixelFormat.PVRTC_RGBA4,  Format.Unknown },
            // Compressed ETC Pixel Formats
            //{ PixelFormat.ETC2_RGB8,    Format.Unknown },
            //{ PixelFormat.ETC2_RGB8A1,  Format.Unknown },
            // Compressed ASTC Pixel Formats
            //{ PixelFormat.ASTC4x4,      Format.Unknown },
            //{ PixelFormat.ASTC5x5,      Format.Unknown },
            //{ PixelFormat.ASTC6x6,      Format.Unknown },
            //{ PixelFormat.ASTC8x5,      Format.Unknown },
            //{ PixelFormat.ASTC8x6,      Format.Unknown },
            //{ PixelFormat.ASTC8x8,      Format.Unknown },
            //{ PixelFormat.ASTC10x10,    Format.Unknown },
            //{ PixelFormat.ASTC12x12,    Format.Unknown },
        };

        public static Format ToDirectXPixelFormat(this PixelFormat format) => _formatsMap[format];
        public static PixelFormat FromDirectXPixelFormat(this Format format) => _formatsMap[format];

        private class FormatMap<TKey, TValue> : IEnumerable<KeyValuePair<TKey, TValue>>
        {
            private readonly Dictionary<TKey, TValue> _forward = new Dictionary<TKey, TValue>();
            private readonly Dictionary<TValue, TKey> _reverse = new Dictionary<TValue, TKey>();
            public void Add(TKey key, TValue value)
            {
                _forward.Add(key, value);
                if (!_reverse.ContainsKey(value))
                {
                    _reverse.Add(value, key);
                }
            }
            public TValue this[TKey key] => _forward[key];
            public TKey this[TValue value] => _reverse[value];
            IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
            public IEnumerator<KeyValuePair<TKey, TValue>> GetEnumerator()
            {
                return _forward.GetEnumerator();
            }
        }
    }
}
