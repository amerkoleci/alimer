// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Define a dxgi format.
/// </summary>
/// <remarks>
/// Equivalent of the <a href="https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format">DXGI_FORMAT</a> enumeration.
/// </remarks>
public enum DxgiFormat : uint
{
    /// <summary>The format is not known.</summary>
	/// <unmanaged>DXGI_FORMAT_UNKNOWN</unmanaged>
	Unknown = 0u,
    /// <summary>A four-component, 128-bit typeless format that supports 32 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32A32_TYPELESS</unmanaged>
    R32G32B32A32Typeless = 1u,
    /// <summary>A four-component, 128-bit floating-point format that supports 32 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32A32_FLOAT</unmanaged>
    R32G32B32A32Float = 2u,
    /// <summary>A four-component, 128-bit unsigned-integer format that supports 32 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32A32_UINT</unmanaged>
    R32G32B32A32Uint = 3u,
    /// <summary>A four-component, 128-bit signed-integer format that supports 32 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32A32_SINT</unmanaged>
    R32G32B32A32Sint = 4u,
    /// <summary>A three-component, 96-bit typeless format that supports 32 bits per color channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32_TYPELESS</unmanaged>
    R32G32B32Typeless = 5u,
    /// <summary>A three-component, 96-bit floating-point format that supports 32 bits per color channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32_FLOAT</unmanaged>
    R32G32B32Float = 6u,
    /// <summary>A three-component, 96-bit unsigned-integer format that supports 32 bits per color channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32_UINT</unmanaged>
    R32G32B32Uint = 7u,
    /// <summary>A three-component, 96-bit signed-integer format that supports 32 bits per color channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32B32_SINT</unmanaged>
    R32G32B32Sint = 8u,
    /// <summary>A four-component, 64-bit typeless format that supports 16 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16B16A16_TYPELESS</unmanaged>
    R16G16B16A16Typeless = 9u,
    /// <summary>A four-component, 64-bit floating-point format that supports 16 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16B16A16_FLOAT</unmanaged>
    R16G16B16A16Float = 10u,
    /// <summary>A four-component, 64-bit unsigned-normalized-integer format that supports 16 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16B16A16_UNORM</unmanaged>
    R16G16B16A16Unorm = 11u,
    /// <summary>A four-component, 64-bit unsigned-integer format that supports 16 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16B16A16_UINT</unmanaged>
    R16G16B16A16Uint = 12u,
    /// <summary>A four-component, 64-bit signed-normalized-integer format that supports 16 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16B16A16_SNORM</unmanaged>
    R16G16B16A16Snorm = 13u,
    /// <summary>A four-component, 64-bit signed-integer format that supports 16 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16B16A16_SINT</unmanaged>
    R16G16B16A16Sint = 14u,
    /// <summary>A two-component, 64-bit typeless format that supports 32 bits for the red channel and 32 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32_TYPELESS</unmanaged>
    R32G32Typeless = 15u,
    /// <summary>A two-component, 64-bit floating-point format that supports 32 bits for the red channel and 32 bits for the green channel.<sup>5,8</sup></summary>
    /// <unmanaged>DXGI_FORMAT_R32G32_FLOAT</unmanaged>
    R32G32Float = 16u,
    /// <summary>A two-component, 64-bit unsigned-integer format that supports 32 bits for the red channel and 32 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32_UINT</unmanaged>
    R32G32Uint = 17u,
    /// <summary>A two-component, 64-bit signed-integer format that supports 32 bits for the red channel and 32 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G32_SINT</unmanaged>
    R32G32Sint = 18u,
    /// <summary>A two-component, 64-bit typeless format that supports 32 bits for the red channel, 8 bits for the green channel, and 24 bits are unused.</summary>
    /// <unmanaged>DXGI_FORMAT_R32G8X24_TYPELESS</unmanaged>
    R32G8X24Typeless = 19u,
    /// <summary>A 32-bit floating-point component, and two unsigned-integer components (with an additional 32 bits). This format supports 32-bit depth, 8-bit stencil, and 24 bits are unused.⁵</summary>
    /// <unmanaged>DXGI_FORMAT_D32_FLOAT_S8X24_UINT</unmanaged>
    D32FloatS8X24Uint = 20u,
    /// <summary>A 32-bit floating-point component, and two typeless components (with an additional 32 bits). This format supports 32-bit red channel, 8 bits are unused, and 24 bits are unused.⁵</summary>
    /// <unmanaged>DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS</unmanaged>
    R32FloatX8X24Typeless = 21u,
    /// <summary>A 32-bit typeless component, and two unsigned-integer components (with an additional 32 bits). This format has 32 bits unused, 8 bits for green channel, and 24 bits are unused.</summary>
    /// <unmanaged>DXGI_FORMAT_X32_TYPELESS_G8X24_UINT</unmanaged>
    X32TypelessG8X24Uint = 22u,
    /// <summary>A four-component, 32-bit typeless format that supports 10 bits for each color and 2 bits for alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R10G10B10A2_TYPELESS</unmanaged>
    R10G10B10A2Typeless = 23u,
    /// <summary>A four-component, 32-bit unsigned-normalized-integer format that supports 10 bits for each color and 2 bits for alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R10G10B10A2_UNORM</unmanaged>
    R10G10B10A2Unorm = 24u,
    /// <summary>A four-component, 32-bit unsigned-integer format that supports 10 bits for each color and 2 bits for alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R10G10B10A2_UINT</unmanaged>
    R10G10B10A2Uint = 25u,
    /// <summary>Three partial-precision floating-point numbers encoded into a single 32-bit value (a variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased (15) exponent).</summary>
    /// <unmanaged>DXGI_FORMAT_R11G11B10_FLOAT</unmanaged>
    R11G11B10Float = 26u,
    /// <summary>A four-component, 32-bit typeless format that supports 8 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8B8A8_TYPELESS</unmanaged>
    R8G8B8A8Typeless = 27u,
    /// <summary>A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8B8A8_UNORM</unmanaged>
    R8G8B8A8Unorm = 28u,
    /// <summary>A four-component, 32-bit unsigned-normalized integer sRGB format that supports 8 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8B8A8_UNORM_SRGB</unmanaged>
    R8G8B8A8UnormSrgb = 29u,
    /// <summary>A four-component, 32-bit unsigned-integer format that supports 8 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8B8A8_UINT</unmanaged>
    R8G8B8A8Uint = 30u,
    /// <summary>A four-component, 32-bit signed-normalized-integer format that supports 8 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8B8A8_SNORM</unmanaged>
    R8G8B8A8Snorm = 31u,
    /// <summary>A four-component, 32-bit signed-integer format that supports 8 bits per channel including alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8B8A8_SINT</unmanaged>
    R8G8B8A8Sint = 32u,
    /// <summary>A two-component, 32-bit typeless format that supports 16 bits for the red channel and 16 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16_TYPELESS</unmanaged>
    R16G16Typeless = 33u,
    /// <summary>A two-component, 32-bit floating-point format that supports 16 bits for the red channel and 16 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16_FLOAT</unmanaged>
    R16G16Float = 34u,
    /// <summary>A two-component, 32-bit unsigned-normalized-integer format that supports 16 bits each for the green and red channels.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16_UNORM</unmanaged>
    R16G16Unorm = 35u,
    /// <summary>A two-component, 32-bit unsigned-integer format that supports 16 bits for the red channel and 16 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16_UINT</unmanaged>
    R16G16Uint = 36u,
    /// <summary>A two-component, 32-bit signed-normalized-integer format that supports 16 bits for the red channel and 16 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16_SNORM</unmanaged>
    R16G16Snorm = 37u,
    /// <summary>A two-component, 32-bit signed-integer format that supports 16 bits for the red channel and 16 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16G16_SINT</unmanaged>
    R16G16Sint = 38u,
    /// <summary>A single-component, 32-bit typeless format that supports 32 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32_TYPELESS</unmanaged>
    R32Typeless = 39u,
    /// <summary>A single-component, 32-bit floating-point format that supports 32 bits for depth.</summary>
    /// <unmanaged>DXGI_FORMAT_D32_FLOAT</unmanaged>
    D32Float = 40u,
    /// <summary>A single-component, 32-bit floating-point format that supports 32 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32_FLOAT</unmanaged>
    R32Float = 41u,
    /// <summary>A single-component, 32-bit unsigned-integer format that supports 32 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32_UINT</unmanaged>
    R32Uint = 42u,
    /// <summary>A single-component, 32-bit signed-integer format that supports 32 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R32_SINT</unmanaged>
    R32Sint = 43u,
    /// <summary>A two-component, 32-bit typeless format that supports 24 bits for the red channel and 8 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R24G8_TYPELESS</unmanaged>
    R24G8Typeless = 44u,
    /// <summary>A 32-bit z-buffer format that supports 24 bits for depth and 8 bits for stencil.</summary>
    /// <unmanaged>DXGI_FORMAT_D24_UNORM_S8_UINT</unmanaged>
    D24UnormS8Uint = 45u,
    /// <summary>A 32-bit format, that contains a 24 bit, single-component, unsigned-normalized integer, with an additional typeless 8 bits. This format has 24 bits red channel and 8 bits unused.</summary>
    /// <unmanaged>DXGI_FORMAT_R24_UNORM_X8_TYPELESS</unmanaged>
    R24UnormX8Typeless = 46u,
    /// <summary>A 32-bit format, that contains a 24 bit, single-component, typeless format,  with an additional 8 bit unsigned integer component. This format has 24 bits unused and 8 bits green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_X24_TYPELESS_G8_UINT</unmanaged>
    X24TypelessG8Uint = 47u,
    /// <summary>A two-component, 16-bit typeless format that supports 8 bits for the red channel and 8 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8_TYPELESS</unmanaged>
    R8G8Typeless = 48u,
    /// <summary>A two-component, 16-bit unsigned-normalized-integer format that supports 8 bits for the red channel and 8 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8_UNORM</unmanaged>
    R8G8Unorm = 49u,
    /// <summary>A two-component, 16-bit unsigned-integer format that supports 8 bits for the red channel and 8 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8_UINT</unmanaged>
    R8G8Uint = 50u,
    /// <summary>A two-component, 16-bit signed-normalized-integer format that supports 8 bits for the red channel and 8 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8_SNORM</unmanaged>
    R8G8Snorm = 51u,
    /// <summary>A two-component, 16-bit signed-integer format that supports 8 bits for the red channel and 8 bits for the green channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8_SINT</unmanaged>
    R8G8Sint = 52u,
    /// <summary>A single-component, 16-bit typeless format that supports 16 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16_TYPELESS</unmanaged>
    R16Typeless = 53u,
    /// <summary>A single-component, 16-bit floating-point format that supports 16 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16_FLOAT</unmanaged>
    R16Float = 54u,
    /// <summary>A single-component, 16-bit unsigned-normalized-integer format that supports 16 bits for depth.</summary>
    /// <unmanaged>DXGI_FORMAT_D16_UNORM</unmanaged>
    D16Unorm = 55u,
    /// <summary>A single-component, 16-bit unsigned-normalized-integer format that supports 16 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16_UNORM</unmanaged>
    R16Unorm = 56u,
    /// <summary>A single-component, 16-bit unsigned-integer format that supports 16 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16_UINT</unmanaged>
    R16Uint = 57u,
    /// <summary>A single-component, 16-bit signed-normalized-integer format that supports 16 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16_SNORM</unmanaged>
    R16Snorm = 58u,
    /// <summary>A single-component, 16-bit signed-integer format that supports 16 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R16_SINT</unmanaged>
    R16Sint = 59u,
    /// <summary>A single-component, 8-bit typeless format that supports 8 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8_TYPELESS</unmanaged>
    R8Typeless = 60u,
    /// <summary>A single-component, 8-bit unsigned-normalized-integer format that supports 8 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8_UNORM</unmanaged>
    R8Unorm = 61u,
    /// <summary>A single-component, 8-bit unsigned-integer format that supports 8 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8_UINT</unmanaged>
    R8Uint = 62u,
    /// <summary>A single-component, 8-bit signed-normalized-integer format that supports 8 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8_SNORM</unmanaged>
    R8Snorm = 63u,
    /// <summary>A single-component, 8-bit signed-integer format that supports 8 bits for the red channel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8_SINT</unmanaged>
    R8Sint = 64u,
    /// <summary>A single-component, 8-bit unsigned-normalized-integer format for alpha only.</summary>
    /// <unmanaged>DXGI_FORMAT_A8_UNORM</unmanaged>
    A8Unorm = 65u,
    /// <summary>A single-component, 1-bit unsigned-normalized integer format that supports 1 bit for the red channel. ².</summary>
    /// <unmanaged>DXGI_FORMAT_R1_UNORM</unmanaged>
    R1Unorm = 66u,
    /// <summary>Three partial-precision floating-point numbers encoded into a single 32-bit value all sharing the same 5-bit exponent (variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased (15) exponent).</summary>
    /// <unmanaged>DXGI_FORMAT_R9G9B9E5_SHAREDEXP</unmanaged>
    R9G9B9E5SharedExp = 67u,
    /// <summary>A four-component, 32-bit unsigned-normalized-integer format. This packed RGB format is analogous to the UYVY format. Each 32-bit block describes a pair of pixels: (R8, G8, B8) and (R8, G8, B8) where the R8/B8 values are repeated, and the G8 values are unique to each pixel.</summary>
    /// <unmanaged>DXGI_FORMAT_R8G8_B8G8_UNORM</unmanaged>
    R8G8B8G8Unorm = 68u,
    /// <summary>A four-component, 32-bit unsigned-normalized-integer format. This packed RGB format is analogous to the YUY2 format. Each 32-bit block describes a pair of pixels: (R8, G8, B8) and (R8, G8, B8) where the R8/B8 values are repeated, and the G8 values are unique to each pixel.</summary>
    /// <unmanaged>DXGI_FORMAT_G8R8_G8B8_UNORM</unmanaged>
    G8R8G8B8Unorm = 69u,
    /// <summary>Four-component typeless block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC1_TYPELESS</unmanaged>
    BC1Typeless = 70u,
    /// <summary>Four-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC1_UNORM</unmanaged>
    BC1Unorm = 71u,
    /// <summary>Four-component block-compression format for sRGB data. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC1_UNORM_SRGB</unmanaged>
    BC1UnormSrgb = 72u,
    /// <summary>Four-component typeless block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC2_TYPELESS</unmanaged>
    BC2Typeless = 73u,
    /// <summary>Four-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC2_UNORM</unmanaged>
    BC2Unorm = 74u,
    /// <summary>Four-component block-compression format for sRGB data. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC2_UNORM_SRGB</unmanaged>
    BC2UnormSrgb = 75u,
    /// <summary>Four-component typeless block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC3_TYPELESS</unmanaged>
    BC3Typeless = 76u,
    /// <summary>Four-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC3_UNORM</unmanaged>
    BC3Unorm = 77u,
    /// <summary>Four-component block-compression format for sRGB data. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC3_UNORM_SRGB</unmanaged>
    BC3UnormSrgb = 78u,
    /// <summary>One-component typeless block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC4_TYPELESS</unmanaged>
    BC4Typeless = 79u,
    /// <summary>One-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC4_UNORM</unmanaged>
    BC4Unorm = 80u,
    /// <summary>One-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC4_SNORM</unmanaged>
    BC4Snorm = 81u,
    /// <summary>Two-component typeless block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC5_TYPELESS</unmanaged>
    BC5Typeless = 82u,
    /// <summary>Two-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC5_UNORM</unmanaged>
    BC5Unorm = 83u,
    /// <summary>Two-component block-compression format. For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC5_SNORM</unmanaged>
    BC5Snorm = 84u,
    /// <summary>A three-component, 16-bit unsigned-normalized-integer format that supports 5 bits for blue, 6 bits for green, and 5 bits for red.</summary>
    /// <unmanaged>DXGI_FORMAT_B5G6R5_UNORM</unmanaged>
    B5G6R5Unorm = 85u,
    /// <summary>A four-component, 16-bit unsigned-normalized-integer format that supports 5 bits for each color channel and 1-bit alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_B5G5R5A1_UNORM</unmanaged>
    B5G5R5A1Unorm = 86u,
    /// <summary>A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8-bit alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_B8G8R8A8_UNORM</unmanaged>
    B8G8R8A8Unorm = 87u,
    /// <summary>A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8 bits unused.</summary>
    /// <unmanaged>DXGI_FORMAT_B8G8R8X8_UNORM</unmanaged>
    B8G8R8X8Unorm = 88u,
    /// <summary>A four-component, 32-bit 2.8-biased fixed-point format that supports 10 bits for each color channel and 2-bit alpha.</summary>
    /// <unmanaged>DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM</unmanaged>
    R10G10B10XRBiasA2Unorm = 89u,
    /// <summary>A four-component, 32-bit typeless format that supports 8 bits for each channel including alpha. ⁴</summary>
    /// <unmanaged>DXGI_FORMAT_B8G8R8A8_TYPELESS</unmanaged>
    B8G8R8A8Typeless = 90u,
    /// <summary>A four-component, 32-bit unsigned-normalized standard RGB format that supports 8 bits for each channel including alpha. ⁴</summary>
    /// <unmanaged>DXGI_FORMAT_B8G8R8A8_UNORM_SRGB</unmanaged>
    B8G8R8A8UnormSrgb = 91u,
    /// <summary>A four-component, 32-bit typeless format that supports 8 bits for each color channel, and 8 bits are unused. ⁴</summary>
    /// <unmanaged>DXGI_FORMAT_B8G8R8X8_TYPELESS</unmanaged>
    B8G8R8X8Typeless = 92u,
    /// <summary>A four-component, 32-bit unsigned-normalized standard RGB format that supports 8 bits for each color channel, and 8 bits are unused. ⁴</summary>
    /// <unmanaged>DXGI_FORMAT_B8G8R8X8_UNORM_SRGB</unmanaged>
    B8G8R8X8UnormSrgb = 93u,
    /// <summary>A typeless block-compression format. ⁴ For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC6H_TYPELESS</unmanaged>
    BC6HTypeless = 94u,
    /// <summary>A block-compression format. ⁴ For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.⁵</summary>
    /// <unmanaged>DXGI_FORMAT_BC6H_UF16</unmanaged>
    BC6HUF16 = 95u,
    /// <summary>A block-compression format. ⁴ For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.⁵</summary>
    /// <unmanaged>DXGI_FORMAT_BC6H_SF16</unmanaged>
    BC6HSF16 = 96u,
    /// <summary>A typeless block-compression format. ⁴ For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC7_TYPELESS</unmanaged>
    BC7Typeless = 97u,
    /// <summary>A block-compression format. ⁴ For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC7_UNORM</unmanaged>
    BC7Unorm = 98u,
    /// <summary>A block-compression format. ⁴ For information about block-compression formats, see <a href="https://docs.microsoft.com/windows/desktop/direct3d11/texture-block-compression-in-direct3d-11">Texture Block Compression in Direct3D 11</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_BC7_UNORM_SRGB</unmanaged>
    BC7UnormSrgb = 99u,
    /// <summary>Most common YUV 4:4:4 video resource format. Valid view formats for this video resource format are DXGI_FORMAT_R8G8B8A8_UNORM and DXGI_FORMAT_R8G8B8A8_UINT. For UAVs, an additional valid view format is DXGI_FORMAT_R32_UINT. By using DXGI_FORMAT_R32_UINT for UAVs, you can both read and write as opposed to just write for DXGI_FORMAT_R8G8B8A8_UNORM and DXGI_FORMAT_R8G8B8A8_UINT. Supported view types are SRV, RTV, and UAV. One view provides a straightforward mapping of the entire surface. The mapping to the view channel is V-&gt;R8, 
    /// U-&gt;G8, 
    /// Y-&gt;B8, 
    /// and A-&gt;A8.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_AYUV</unmanaged>
    AYUV = 100u,
    /// <summary>10-bit per channel packed YUV 4:4:4 video resource format. Valid view formats for this video resource format are DXGI_FORMAT_R10G10B10A2_UNORM and DXGI_FORMAT_R10G10B10A2_UINT. For UAVs, an additional valid view format is DXGI_FORMAT_R32_UINT. By using DXGI_FORMAT_R32_UINT for UAVs, you can both read and write as opposed to just write for DXGI_FORMAT_R10G10B10A2_UNORM and DXGI_FORMAT_R10G10B10A2_UINT. Supported view types are SRV and UAV. One view provides a straightforward mapping of the entire surface. The mapping to the view channel is U-&gt;R10,
    /// Y-&gt;G10,
    /// V-&gt;B10,
    /// and A-&gt;A2.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>.</summary>
    /// <unmanaged>DXGI_FORMAT_Y410</unmanaged>
    Y410 = 101u,
    /// <summary>16-bit per channel packed YUV 4:4:4 video resource format. Valid view formats for this video resource format are DXGI_FORMAT_R16G16B16A16_UNORM and DXGI_FORMAT_R16G16B16A16_UINT. Supported view types are SRV and UAV. One view provides a straightforward mapping of the entire surface. The mapping to the view channel is U-&gt;R16,
    /// Y-&gt;G16,
    /// V-&gt;B16,
    /// and A-&gt;A16.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>./summary>
    /// <unmanaged>DXGI_FORMAT_Y416</unmanaged>
    Y416 = 102u,
    /// <summary>Most common YUV 4:2:0 video resource format. Valid luminance data view formats for this video resource format are DXGI_FORMAT_R8_UNORM and DXGI_FORMAT_R8_UINT. Valid chrominance data view formats (width and height are each 1/2 of luminance view) for this video resource format are DXGI_FORMAT_R8G8_UNORM and DXGI_FORMAT_R8G8_UINT. Supported view types are SRV, RTV, and UAV. For luminance data view, the mapping to the view channel is Y-&gt;R8. For chrominance data view, the mapping to the view channel is U-&gt;R8 and
    /// V-&gt;G8.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width and height must be even. Direct3D 11 staging resources and initData parameters for this format use (rowPitch * (height + (height / 2))) bytes. The first (SysMemPitch * height) bytes are the Y plane, the remaining (SysMemPitch * (height / 2)) bytes are the UV plane.
    ///
    /// An app using the YUY 4:2:0 formats  must map the luma (Y) plane separately from the chroma (UV) planes. Developers do this by calling <a href="https://docs.microsoft.com/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createshaderresourceview">ID3D12Device::CreateShaderResourceView</a> twice for the same texture and passing in 1-channel and 2-channel formats. Passing in a 1-channel format compatible with the Y plane maps only the Y plane. Passing in a 2-channel format compatible with the UV planes (together) maps only the U and V planes as a single resource view.</summary>
    /// <unmanaged>DXGI_FORMAT_NV12</unmanaged>
    NV12 = 103u,
    /// <summary>10-bit per channel planar YUV 4:2:0 video resource format. Valid luminance data view formats for this video resource format are DXGI_FORMAT_R16_UNORM and DXGI_FORMAT_R16_UINT. The runtime does not enforce whether the lowest 6 bits are 0 (given that this video resource format is a 10-bit format that uses 16 bits). If required, application shader code would have to enforce this manually.  From the runtime's point of view, DXGI_FORMAT_P010 is no different than DXGI_FORMAT_P016. Valid chrominance data view formats (width and height are each 1/2 of luminance view) for this video resource format are DXGI_FORMAT_R16G16_UNORM and DXGI_FORMAT_R16G16_UINT. For UAVs, an additional valid chrominance data view format is DXGI_FORMAT_R32_UINT. By using DXGI_FORMAT_R32_UINT for UAVs, you can both read and write as opposed to just write for DXGI_FORMAT_R16G16_UNORM and DXGI_FORMAT_R16G16_UINT. Supported view types are SRV, RTV, and UAV. For luminance data view, the mapping to the view channel is Y-&gt;R16. For chrominance data view, the mapping to the view channel is U-&gt;R16 and
    /// V-&gt;G16.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width and height must be even. Direct3D 11 staging resources and initData parameters for this format use (rowPitch * (height + (height / 2))) bytes. The first (SysMemPitch * height) bytes are the Y plane, the remaining (SysMemPitch * (height / 2)) bytes are the UV plane.
    ///
    /// An app using the YUY 4:2:0 formats  must map the luma (Y) plane separately from the chroma (UV) planes. Developers do this by calling <a href="https://docs.microsoft.com/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createshaderresourceview">ID3D12Device::CreateShaderResourceView</a> twice for the same texture and passing in 1-channel and 2-channel formats. Passing in a 1-channel format compatible with the Y plane maps only the Y plane. Passing in a 2-channel format compatible with the UV planes (together) maps only the U and V planes as a single resource view.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_P010</unmanaged>
    P010 = 104u,
    /// <summary>16-bit per channel planar YUV 4:2:0 video resource format. Valid luminance data view formats for this video resource format are DXGI_FORMAT_R16_UNORM and DXGI_FORMAT_R16_UINT. Valid chrominance data view formats (width and height are each 1/2 of luminance view) for this video resource format are DXGI_FORMAT_R16G16_UNORM and DXGI_FORMAT_R16G16_UINT. For UAVs, an additional valid chrominance data view format is DXGI_FORMAT_R32_UINT. By using DXGI_FORMAT_R32_UINT for UAVs, you can both read and write as opposed to just write for DXGI_FORMAT_R16G16_UNORM and DXGI_FORMAT_R16G16_UINT. Supported view types are SRV, RTV, and UAV. For luminance data view, the mapping to the view channel is Y-&gt;R16. For chrominance data view, the mapping to the view channel is U-&gt;R16 and
    /// V-&gt;G16.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width and height must be even. Direct3D 11 staging resources and initData parameters for this format use (rowPitch * (height + (height / 2))) bytes. The first (SysMemPitch * height) bytes are the Y plane, the remaining (SysMemPitch * (height / 2)) bytes are the UV plane.
    ///
    /// An app using the YUY 4:2:0 formats  must map the luma (Y) plane separately from the chroma (UV) planes. Developers do this by calling <a href="https://docs.microsoft.com/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createshaderresourceview">ID3D12Device::CreateShaderResourceView</a> twice for the same texture and passing in 1-channel and 2-channel formats. Passing in a 1-channel format compatible with the Y plane maps only the Y plane. Passing in a 2-channel format compatible with the UV planes (together) maps only the U and V planes as a single resource view.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_P016</unmanaged>
    P016 = 105u,
    /// <summary>8-bit per channel planar YUV 4:2:0 video resource format. This format is subsampled where each pixel has its own Y value, but each 2x2 pixel block shares a single U and V value. The runtime requires that the width and height of all resources that are created with this format are multiples of 2. The runtime also requires that the left, right, top, and bottom members of any RECT that are used for this format are multiples of 2. This format differs from DXGI_FORMAT_NV12 in that the layout of the data within the resource is completely opaque to applications. Applications cannot use the CPU to map the resource and then access the data within the resource. You cannot use shaders with this format. Because of this behavior, legacy hardware that supports a non-NV12 4:2:0 layout (for example, YV12, and so on) can be used. Also, new hardware that has a 4:2:0 implementation better than NV12 can be used when the application does not need the data to be in a standard layout. 
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width and height must be even. Direct3D 11 staging resources and initData parameters for this format use (rowPitch * (height + (height / 2))) bytes. 
    ///
    /// An app using the YUY 4:2:0 formats  must map the luma (Y) plane separately from the chroma (UV) planes. Developers do this by calling <a href="https://docs.microsoft.com/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createshaderresourceview">ID3D12Device::CreateShaderResourceView</a> twice for the same texture and passing in 1-channel and 2-channel formats. Passing in a 1-channel format compatible with the Y plane maps only the Y plane. Passing in a 2-channel format compatible with the UV planes (together) maps only the U and V planes as a single resource view.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_420_OPAQUE</unmanaged>
    Opaque420 = 106u,
    /// <summary>Most common YUV 4:2:2 video resource format. Valid view formats for this video resource format are DXGI_FORMAT_R8G8B8A8_UNORM and DXGI_FORMAT_R8G8B8A8_UINT. For UAVs, an additional valid view format is DXGI_FORMAT_R32_UINT. By using DXGI_FORMAT_R32_UINT for UAVs, you can both read and write as opposed to just write for DXGI_FORMAT_R8G8B8A8_UNORM and DXGI_FORMAT_R8G8B8A8_UINT. Supported view types are SRV and UAV. One view provides a straightforward mapping of the entire surface. The mapping to the view channel is Y0-&gt;R8, 
    /// U0-&gt;G8, 
    /// Y1-&gt;B8, 
    /// and V0-&gt;A8.
    ///
    /// A unique valid view format for this video resource format is DXGI_FORMAT_R8G8_B8G8_UNORM. With this view format, the width of the view appears to be twice what the DXGI_FORMAT_R8G8B8A8_UNORM or DXGI_FORMAT_R8G8B8A8_UINT view would be when hardware reconstructs RGBA automatically on read and before filtering.  This Direct3D hardware behavior is legacy and is likely not useful any more. With this view format, the mapping to the view channel is Y0-&gt;R8, 
    /// U0-&gt;
    /// G8[0], 
    /// Y1-&gt;B8, 
    /// and V0-&gt;
    /// G8[1].
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width must be even.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_YUY2</unmanaged>
    YUY2 = 107u,
    /// <summary>10-bit per channel packed YUV 4:2:2 video resource format. Valid view formats for this video resource format are DXGI_FORMAT_R16G16B16A16_UNORM and DXGI_FORMAT_R16G16B16A16_UINT. The runtime does not enforce whether the lowest 6 bits are 0 (given that this video resource format is a 10-bit format that uses 16 bits). If required, application shader code would have to enforce this manually.  From the runtime's point of view, DXGI_FORMAT_Y210 is no different than DXGI_FORMAT_Y216. Supported view types are SRV and UAV. One view provides a straightforward mapping of the entire surface. The mapping to the view channel is Y0-&gt;R16,
    /// U-&gt;G16,
    /// Y1-&gt;B16,
    /// and V-&gt;A16.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width must be even.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_Y210</unmanaged>
    Y210 = 108u,
    /// <summary>16-bit per channel packed YUV 4:2:2 video resource format. Valid view formats for this video resource format are DXGI_FORMAT_R16G16B16A16_UNORM and DXGI_FORMAT_R16G16B16A16_UINT. Supported view types are SRV and UAV. One view provides a straightforward mapping of the entire surface. The mapping to the view channel is Y0-&gt;R16,
    /// U-&gt;G16,
    /// Y1-&gt;B16,
    /// and V-&gt;A16.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width must be even.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_Y216</unmanaged>
    Y216 = 109u,
    /// <summary>Most common planar YUV 4:1:1 video resource format. Valid luminance data view formats for this video resource format are DXGI_FORMAT_R8_UNORM and DXGI_FORMAT_R8_UINT. Valid chrominance data view formats (width and height are each 1/4 of luminance view) for this video resource format are DXGI_FORMAT_R8G8_UNORM and DXGI_FORMAT_R8G8_UINT. Supported view types are SRV, RTV, and UAV. For luminance data view, the mapping to the view channel is Y-&gt;R8. For chrominance data view, the mapping to the view channel is U-&gt;R8 and
    /// V-&gt;G8.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// Width must be a multiple of 4. Direct3D11 staging resources and initData parameters for this format use (rowPitch * height * 2) bytes. The first (SysMemPitch * height) bytes are the Y plane, the next ((SysMemPitch / 2) * height) bytes are the UV plane, and the remainder is padding. 
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_NV11</unmanaged>
    NV11 = 110u,
    /// <summary>4-bit palletized YUV format that is commonly used for DVD subpicture.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_AI44</unmanaged>
    AI44 = 111u,
    /// <summary>4-bit palletized YUV format that is commonly used for DVD subpicture.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_IA44</unmanaged>
    IA44 = 112u,
    /// <summary>8-bit palletized format that is used for palletized RGB data when the processor processes ISDB-T data and for palletized YUV data when the processor processes BluRay data.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_P8</unmanaged>
    P8 = 113u,
    /// <summary>8-bit palletized format with 8 bits of alpha that is used for palletized YUV data when the processor processes BluRay data.
    ///
    /// For more info about YUV formats for video rendering, see <a href="https://docs.microsoft.com/windows/desktop/medfound/recommended-8-bit-yuv-formats-for-video-rendering">Recommended 8-Bit YUV Formats for Video Rendering</a>. 
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_A8P8</unmanaged>
    A8P8 = 114u,
    /// <summary>A four-component, 16-bit unsigned-normalized integer format that supports 4 bits for each channel including alpha.
    ///
    /// <b>Direct3D 11.1:  </b>This value is not supported until Windows 8.</summary>
    /// <unmanaged>DXGI_FORMAT_B4G4R4A4_UNORM</unmanaged>
    B4G4R4A4Unorm = 115u,
    /// <summary>A video format; an 8-bit version of a hybrid planar 4:2:2 format.</summary>
    /// <unmanaged>DXGI_FORMAT_P208</unmanaged>
    P208 = 130u,
    /// <summary>An 8 bit YCbCrA 4:4 rendering format.</summary>
    /// <unmanaged>DXGI_FORMAT_V208</unmanaged>
    V208 = 131u,
    /// <summary>An 8 bit YCbCrA 4:4:4:4 rendering format.</summary>
    /// <unmanaged>DXGI_FORMAT_V408</unmanaged>
    V408 = 132u,
    /// <unmanaged>DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE</unmanaged>
    SamplerFeedbackMinMipOpaque = 189u,
    /// <unmanaged>DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE</unmanaged>
    SamplerFeedbackMipRegionUsedOpaque = 190u,
	/// <unmanaged>DXGI_FORMAT_A4B4G4R4_UNORM</unmanaged>
	A4B4G4R4Unorm = 191,
    /// <unmanaged>DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT</unmanaged>
    Xbox_R10G10B10_7E3_A2Float = 116u,
    /// <unmanaged>DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT</unmanaged>
    Xbox_R10G10B10_6E4_A2Float = 117u,
    /// <unmanaged>DXGI_FORMAT_D16_UNORM_S8_UINT</unmanaged>
    Xbox_D16Unorm_S8Uint = 118u,
    /// <unmanaged>DXGI_FORMAT_R16_UNORM_X8_TYPELESS</unmanaged>
    Xbox_R16Unorm_X8Typeless = 119u,
    /// <unmanaged>DXGI_FORMAT_X16_TYPELESS_G8_UINT</unmanaged>
    Xbox_X16Typeless_G8Uint = 120u,
    /// <unmanaged>DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM</unmanaged>
    Xbox_R10G10B10Snorm_A2Unorm = 189u,
    /// <unmanaged>DXGI_FORMAT_R4G4_UNORM</unmanaged>
    Xbox_R4G4Unorm = 190u,
}
