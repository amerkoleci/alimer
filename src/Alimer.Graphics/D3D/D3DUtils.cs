// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Collections;
using System.Collections.Generic;
using TerraFX.Interop;
using static TerraFX.Interop.DXGI_FORMAT;

namespace Alimer.Graphics
{
    internal static class D3DUtils
    {
        private static readonly FormatMap<PixelFormat, DXGI_FORMAT> s_formatsMap = new FormatMap<PixelFormat, DXGI_FORMAT>
        {
            { PixelFormat.Invalid,        DXGI_FORMAT_UNKNOWN },
            // 8-bit pixel formats
            { PixelFormat.R8Unorm,          DXGI_FORMAT_R8_UNORM },
            { PixelFormat.R8Snorm,          DXGI_FORMAT_R8_SNORM },
            { PixelFormat.R8Uint,           DXGI_FORMAT_R8_UINT },
            { PixelFormat.R8Sint,           DXGI_FORMAT_R8_SINT },
            // 16-bit pixel formats
            { PixelFormat.R16Unorm,         DXGI_FORMAT_R16_UNORM },
            { PixelFormat.R16Snorm,         DXGI_FORMAT_R16_SNORM },
            { PixelFormat.R16Uint,          DXGI_FORMAT_R16_UINT },
            { PixelFormat.R16Sint,          DXGI_FORMAT_R16_SINT },
            { PixelFormat.R16Float,         DXGI_FORMAT_R16_FLOAT },
            { PixelFormat.RG8Unorm,         DXGI_FORMAT_R8G8_UNORM },
            { PixelFormat.RG8Snorm,         DXGI_FORMAT_R8G8_SNORM },
            { PixelFormat.RG8Uint,          DXGI_FORMAT_R8G8_UINT },
            { PixelFormat.RG8Sint,          DXGI_FORMAT_R8G8_SINT },
            // Packed 16-bit pixel formats
            //{ PixelFormat.B5G6R5UNorm,      DXGI_FORMAT_B5G6R5_UNorm },
            //{ PixelFormat.BGRA4UNorm,       DXGI_FORMAT_B4G4R4A4_UNorm },
            // 32-bit pixel formats
            { PixelFormat.R32Uint,          DXGI_FORMAT_R32_UINT },
            { PixelFormat.R32Sint,          DXGI_FORMAT_R32_SINT },
            { PixelFormat.R32Float,         DXGI_FORMAT_R32_FLOAT },
            { PixelFormat.RG16Unorm,        DXGI_FORMAT_R16G16_UNORM },
            { PixelFormat.RG16Snorm,        DXGI_FORMAT_R16G16_SNORM },
            { PixelFormat.RG16Uint,         DXGI_FORMAT_R16G16_UINT },
            { PixelFormat.RG16Sint,         DXGI_FORMAT_R16G16_SINT },
            { PixelFormat.RG16Float,        DXGI_FORMAT_R16G16_FLOAT },
            { PixelFormat.RGBA8Unorm,       DXGI_FORMAT_R8G8B8A8_UNORM },
            { PixelFormat.RGBA8UnormSrgb,   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
            { PixelFormat.RGBA8Snorm,       DXGI_FORMAT_R8G8B8A8_SNORM },
            { PixelFormat.RGBA8Uint,        DXGI_FORMAT_R8G8B8A8_UINT },
            { PixelFormat.RGBA8Sint,        DXGI_FORMAT_R8G8B8A8_SINT },
            { PixelFormat.BGRA8Unorm,       DXGI_FORMAT_B8G8R8A8_UNORM },
            { PixelFormat.BGRA8UnormSrgb,   DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
            // Packed 32-Bit Pixel formats
            { PixelFormat.RGB10A2Unorm,     DXGI_FORMAT_R10G10B10A2_UNORM },
            { PixelFormat.RG11B10Float,     DXGI_FORMAT_R11G11B10_FLOAT },
            // 64-Bit Pixel Formats
            { PixelFormat.RG32Uint,         DXGI_FORMAT_R32G32_UINT },
            { PixelFormat.RG32Sint,         DXGI_FORMAT_R32G32_SINT },
            { PixelFormat.RG32Float,        DXGI_FORMAT_R32G32_FLOAT },
            { PixelFormat.RGBA16Unorm,      DXGI_FORMAT_R16G16B16A16_UNORM },
            { PixelFormat.RGBA16Snorm,      DXGI_FORMAT_R16G16B16A16_SNORM },
            { PixelFormat.RGBA16Uint,       DXGI_FORMAT_R16G16B16A16_UINT },
            { PixelFormat.RGBA16Sint,       DXGI_FORMAT_R16G16B16A16_SINT },
            { PixelFormat.RGBA16Float,      DXGI_FORMAT_R16G16B16A16_FLOAT },
            // 128-Bit Pixel Formats
            { PixelFormat.RGBA32Uint,           DXGI_FORMAT_R32G32B32A32_UINT },
            { PixelFormat.RGBA32Sint,           DXGI_FORMAT_R32G32B32A32_SINT },
            { PixelFormat.RGBA32Float,          DXGI_FORMAT_R32G32B32A32_FLOAT },
            // Depth-stencil
            { PixelFormat.Depth16Unorm,         DXGI_FORMAT_D16_UNORM },
            { PixelFormat.Depth32Float,         DXGI_FORMAT_D32_FLOAT },
            { PixelFormat.Depth24UnormStencil8, DXGI_FORMAT_D24_UNORM_S8_UINT },
            { PixelFormat.Depth32FloatStencil8, DXGI_FORMAT_D32_FLOAT_S8X24_UINT },
            // Compressed BC formats
            { PixelFormat.BC1RGBAUnorm,         DXGI_FORMAT_BC1_UNORM },
            { PixelFormat.BC1RGBAUnormSrgb,     DXGI_FORMAT_BC1_UNORM_SRGB },
            { PixelFormat.BC2RGBAUnorm,         DXGI_FORMAT_BC2_UNORM },
            { PixelFormat.BC2RGBAUnormSrgb,     DXGI_FORMAT_BC2_UNORM_SRGB },
            { PixelFormat.BC3RGBAUnorm,         DXGI_FORMAT_BC3_UNORM },
            { PixelFormat.BC3RGBAUnormSrgb,     DXGI_FORMAT_BC3_UNORM_SRGB },
            { PixelFormat.BC4RUnorm,            DXGI_FORMAT_BC4_UNORM },
            { PixelFormat.BC4RSnorm,            DXGI_FORMAT_BC4_SNORM },
            { PixelFormat.BC5RGUnorm,           DXGI_FORMAT_BC5_UNORM },
            { PixelFormat.BC5RGSnorm,           DXGI_FORMAT_BC5_SNORM },
            { PixelFormat.BC6HRGBSfloat,        DXGI_FORMAT_BC6H_SF16 },
            { PixelFormat.BC6HRGBUfloat,        DXGI_FORMAT_BC6H_UF16 },
            { PixelFormat.BC7RGBAUnorm,         DXGI_FORMAT_BC7_UNORM },
            { PixelFormat.BC7RGBAUnormSrgb,     DXGI_FORMAT_BC7_UNORM_SRGB },
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

        public static DXGI_FORMAT ToDirectXPixelFormat(this PixelFormat format) => s_formatsMap[format];
        public static PixelFormat FromDirectXPixelFormat(this DXGI_FORMAT format) => s_formatsMap[format];

        private class FormatMap<TKey, TValue> : IEnumerable<KeyValuePair<TKey, TValue>>
            where TKey : notnull
            where TValue : notnull
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
