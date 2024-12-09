// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer.h"

#if defined(ALIMER_GPU_D3D12)
#include <directx/dxgiformat.h>
#else
typedef enum DXGI_FORMAT {
    DXGI_FORMAT_UNKNOWN = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R32G32B32A32_UINT = 3,
    DXGI_FORMAT_R32G32B32A32_SINT = 4,
    DXGI_FORMAT_R32G32B32_TYPELESS = 5,
    DXGI_FORMAT_R32G32B32_FLOAT = 6,
    DXGI_FORMAT_R32G32B32_UINT = 7,
    DXGI_FORMAT_R32G32B32_SINT = 8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM = 11,
    DXGI_FORMAT_R16G16B16A16_UINT = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM = 13,
    DXGI_FORMAT_R16G16B16A16_SINT = 14,
    DXGI_FORMAT_R32G32_TYPELESS = 15,
    DXGI_FORMAT_R32G32_FLOAT = 16,
    DXGI_FORMAT_R32G32_UINT = 17,
    DXGI_FORMAT_R32G32_SINT = 18,
    DXGI_FORMAT_R32G8X24_TYPELESS = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM = 24,
    DXGI_FORMAT_R10G10B10A2_UINT = 25,
    DXGI_FORMAT_R11G11B10_FLOAT = 26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
    DXGI_FORMAT_R8G8B8A8_UINT = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM = 31,
    DXGI_FORMAT_R8G8B8A8_SINT = 32,
    DXGI_FORMAT_R16G16_TYPELESS = 33,
    DXGI_FORMAT_R16G16_FLOAT = 34,
    DXGI_FORMAT_R16G16_UNORM = 35,
    DXGI_FORMAT_R16G16_UINT = 36,
    DXGI_FORMAT_R16G16_SNORM = 37,
    DXGI_FORMAT_R16G16_SINT = 38,
    DXGI_FORMAT_R32_TYPELESS = 39,
    DXGI_FORMAT_D32_FLOAT = 40,
    DXGI_FORMAT_R32_FLOAT = 41,
    DXGI_FORMAT_R32_UINT = 42,
    DXGI_FORMAT_R32_SINT = 43,
    DXGI_FORMAT_R24G8_TYPELESS = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
    DXGI_FORMAT_R8G8_TYPELESS = 48,
    DXGI_FORMAT_R8G8_UNORM = 49,
    DXGI_FORMAT_R8G8_UINT = 50,
    DXGI_FORMAT_R8G8_SNORM = 51,
    DXGI_FORMAT_R8G8_SINT = 52,
    DXGI_FORMAT_R16_TYPELESS = 53,
    DXGI_FORMAT_R16_FLOAT = 54,
    DXGI_FORMAT_D16_UNORM = 55,
    DXGI_FORMAT_R16_UNORM = 56,
    DXGI_FORMAT_R16_UINT = 57,
    DXGI_FORMAT_R16_SNORM = 58,
    DXGI_FORMAT_R16_SINT = 59,
    DXGI_FORMAT_R8_TYPELESS = 60,
    DXGI_FORMAT_R8_UNORM = 61,
    DXGI_FORMAT_R8_UINT = 62,
    DXGI_FORMAT_R8_SNORM = 63,
    DXGI_FORMAT_R8_SINT = 64,
    DXGI_FORMAT_A8_UNORM = 65,
    DXGI_FORMAT_R1_UNORM = 66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
    DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
    DXGI_FORMAT_BC1_TYPELESS = 70,
    DXGI_FORMAT_BC1_UNORM = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB = 72,
    DXGI_FORMAT_BC2_TYPELESS = 73,
    DXGI_FORMAT_BC2_UNORM = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB = 75,
    DXGI_FORMAT_BC3_TYPELESS = 76,
    DXGI_FORMAT_BC3_UNORM = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB = 78,
    DXGI_FORMAT_BC4_TYPELESS = 79,
    DXGI_FORMAT_BC4_UNORM = 80,
    DXGI_FORMAT_BC4_SNORM = 81,
    DXGI_FORMAT_BC5_TYPELESS = 82,
    DXGI_FORMAT_BC5_UNORM = 83,
    DXGI_FORMAT_BC5_SNORM = 84,
    DXGI_FORMAT_B5G6R5_UNORM = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM = 88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
    DXGI_FORMAT_BC6H_TYPELESS = 94,
    DXGI_FORMAT_BC6H_UF16 = 95,
    DXGI_FORMAT_BC6H_SF16 = 96,
    DXGI_FORMAT_BC7_TYPELESS = 97,
    DXGI_FORMAT_BC7_UNORM = 98,
    DXGI_FORMAT_BC7_UNORM_SRGB = 99,
    DXGI_FORMAT_AYUV = 100,
    DXGI_FORMAT_Y410 = 101,
    DXGI_FORMAT_Y416 = 102,
    DXGI_FORMAT_NV12 = 103,
    DXGI_FORMAT_P010 = 104,
    DXGI_FORMAT_P016 = 105,
    DXGI_FORMAT_420_OPAQUE = 106,
    DXGI_FORMAT_YUY2 = 107,
    DXGI_FORMAT_Y210 = 108,
    DXGI_FORMAT_Y216 = 109,
    DXGI_FORMAT_NV11 = 110,
    DXGI_FORMAT_AI44 = 111,
    DXGI_FORMAT_IA44 = 112,
    DXGI_FORMAT_P8 = 113,
    DXGI_FORMAT_A8P8 = 114,
    DXGI_FORMAT_B4G4R4A4_UNORM = 115,

    DXGI_FORMAT_P208 = 130,
    DXGI_FORMAT_V208 = 131,
    DXGI_FORMAT_V408 = 132,


    DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE = 189,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,

    DXGI_FORMAT_A4B4G4R4_UNORM = 191,


    DXGI_FORMAT_FORCE_UINT = 0xffffffff
} DXGI_FORMAT;
#endif

#if defined(ALIMER_GPU_VULKAN)
#	include <vulkan/vulkan_core.h>
#else
typedef enum VkFormat {
    VK_FORMAT_UNDEFINED = 0,
    VK_FORMAT_R4G4_UNORM_PACK8 = 1,
    VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 2,
    VK_FORMAT_B4G4R4A4_UNORM_PACK16 = 3,
    VK_FORMAT_R5G6B5_UNORM_PACK16 = 4,
    VK_FORMAT_B5G6R5_UNORM_PACK16 = 5,
    VK_FORMAT_R5G5B5A1_UNORM_PACK16 = 6,
    VK_FORMAT_B5G5R5A1_UNORM_PACK16 = 7,
    VK_FORMAT_A1R5G5B5_UNORM_PACK16 = 8,
    VK_FORMAT_R8_UNORM = 9,
    VK_FORMAT_R8_SNORM = 10,
    VK_FORMAT_R8_USCALED = 11,
    VK_FORMAT_R8_SSCALED = 12,
    VK_FORMAT_R8_UINT = 13,
    VK_FORMAT_R8_SINT = 14,
    VK_FORMAT_R8_SRGB = 15,
    VK_FORMAT_R8G8_UNORM = 16,
    VK_FORMAT_R8G8_SNORM = 17,
    VK_FORMAT_R8G8_USCALED = 18,
    VK_FORMAT_R8G8_SSCALED = 19,
    VK_FORMAT_R8G8_UINT = 20,
    VK_FORMAT_R8G8_SINT = 21,
    VK_FORMAT_R8G8_SRGB = 22,
    VK_FORMAT_R8G8B8_UNORM = 23,
    VK_FORMAT_R8G8B8_SNORM = 24,
    VK_FORMAT_R8G8B8_USCALED = 25,
    VK_FORMAT_R8G8B8_SSCALED = 26,
    VK_FORMAT_R8G8B8_UINT = 27,
    VK_FORMAT_R8G8B8_SINT = 28,
    VK_FORMAT_R8G8B8_SRGB = 29,
    VK_FORMAT_B8G8R8_UNORM = 30,
    VK_FORMAT_B8G8R8_SNORM = 31,
    VK_FORMAT_B8G8R8_USCALED = 32,
    VK_FORMAT_B8G8R8_SSCALED = 33,
    VK_FORMAT_B8G8R8_UINT = 34,
    VK_FORMAT_B8G8R8_SINT = 35,
    VK_FORMAT_B8G8R8_SRGB = 36,
    VK_FORMAT_R8G8B8A8_UNORM = 37,
    VK_FORMAT_R8G8B8A8_SNORM = 38,
    VK_FORMAT_R8G8B8A8_USCALED = 39,
    VK_FORMAT_R8G8B8A8_SSCALED = 40,
    VK_FORMAT_R8G8B8A8_UINT = 41,
    VK_FORMAT_R8G8B8A8_SINT = 42,
    VK_FORMAT_R8G8B8A8_SRGB = 43,
    VK_FORMAT_B8G8R8A8_UNORM = 44,
    VK_FORMAT_B8G8R8A8_SNORM = 45,
    VK_FORMAT_B8G8R8A8_USCALED = 46,
    VK_FORMAT_B8G8R8A8_SSCALED = 47,
    VK_FORMAT_B8G8R8A8_UINT = 48,
    VK_FORMAT_B8G8R8A8_SINT = 49,
    VK_FORMAT_B8G8R8A8_SRGB = 50,
    VK_FORMAT_A8B8G8R8_UNORM_PACK32 = 51,
    VK_FORMAT_A8B8G8R8_SNORM_PACK32 = 52,
    VK_FORMAT_A8B8G8R8_USCALED_PACK32 = 53,
    VK_FORMAT_A8B8G8R8_SSCALED_PACK32 = 54,
    VK_FORMAT_A8B8G8R8_UINT_PACK32 = 55,
    VK_FORMAT_A8B8G8R8_SINT_PACK32 = 56,
    VK_FORMAT_A8B8G8R8_SRGB_PACK32 = 57,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32 = 58,
    VK_FORMAT_A2R10G10B10_SNORM_PACK32 = 59,
    VK_FORMAT_A2R10G10B10_USCALED_PACK32 = 60,
    VK_FORMAT_A2R10G10B10_SSCALED_PACK32 = 61,
    VK_FORMAT_A2R10G10B10_UINT_PACK32 = 62,
    VK_FORMAT_A2R10G10B10_SINT_PACK32 = 63,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32 = 64,
    VK_FORMAT_A2B10G10R10_SNORM_PACK32 = 65,
    VK_FORMAT_A2B10G10R10_USCALED_PACK32 = 66,
    VK_FORMAT_A2B10G10R10_SSCALED_PACK32 = 67,
    VK_FORMAT_A2B10G10R10_UINT_PACK32 = 68,
    VK_FORMAT_A2B10G10R10_SINT_PACK32 = 69,
    VK_FORMAT_R16_UNORM = 70,
    VK_FORMAT_R16_SNORM = 71,
    VK_FORMAT_R16_USCALED = 72,
    VK_FORMAT_R16_SSCALED = 73,
    VK_FORMAT_R16_UINT = 74,
    VK_FORMAT_R16_SINT = 75,
    VK_FORMAT_R16_SFLOAT = 76,
    VK_FORMAT_R16G16_UNORM = 77,
    VK_FORMAT_R16G16_SNORM = 78,
    VK_FORMAT_R16G16_USCALED = 79,
    VK_FORMAT_R16G16_SSCALED = 80,
    VK_FORMAT_R16G16_UINT = 81,
    VK_FORMAT_R16G16_SINT = 82,
    VK_FORMAT_R16G16_SFLOAT = 83,
    VK_FORMAT_R16G16B16_UNORM = 84,
    VK_FORMAT_R16G16B16_SNORM = 85,
    VK_FORMAT_R16G16B16_USCALED = 86,
    VK_FORMAT_R16G16B16_SSCALED = 87,
    VK_FORMAT_R16G16B16_UINT = 88,
    VK_FORMAT_R16G16B16_SINT = 89,
    VK_FORMAT_R16G16B16_SFLOAT = 90,
    VK_FORMAT_R16G16B16A16_UNORM = 91,
    VK_FORMAT_R16G16B16A16_SNORM = 92,
    VK_FORMAT_R16G16B16A16_USCALED = 93,
    VK_FORMAT_R16G16B16A16_SSCALED = 94,
    VK_FORMAT_R16G16B16A16_UINT = 95,
    VK_FORMAT_R16G16B16A16_SINT = 96,
    VK_FORMAT_R16G16B16A16_SFLOAT = 97,
    VK_FORMAT_R32_UINT = 98,
    VK_FORMAT_R32_SINT = 99,
    VK_FORMAT_R32_SFLOAT = 100,
    VK_FORMAT_R32G32_UINT = 101,
    VK_FORMAT_R32G32_SINT = 102,
    VK_FORMAT_R32G32_SFLOAT = 103,
    VK_FORMAT_R32G32B32_UINT = 104,
    VK_FORMAT_R32G32B32_SINT = 105,
    VK_FORMAT_R32G32B32_SFLOAT = 106,
    VK_FORMAT_R32G32B32A32_UINT = 107,
    VK_FORMAT_R32G32B32A32_SINT = 108,
    VK_FORMAT_R32G32B32A32_SFLOAT = 109,
    VK_FORMAT_R64_UINT = 110,
    VK_FORMAT_R64_SINT = 111,
    VK_FORMAT_R64_SFLOAT = 112,
    VK_FORMAT_R64G64_UINT = 113,
    VK_FORMAT_R64G64_SINT = 114,
    VK_FORMAT_R64G64_SFLOAT = 115,
    VK_FORMAT_R64G64B64_UINT = 116,
    VK_FORMAT_R64G64B64_SINT = 117,
    VK_FORMAT_R64G64B64_SFLOAT = 118,
    VK_FORMAT_R64G64B64A64_UINT = 119,
    VK_FORMAT_R64G64B64A64_SINT = 120,
    VK_FORMAT_R64G64B64A64_SFLOAT = 121,
    VK_FORMAT_B10G11R11_UFLOAT_PACK32 = 122,
    VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 = 123,
    VK_FORMAT_D16_UNORM = 124,
    VK_FORMAT_X8_D24_UNORM_PACK32 = 125,
    VK_FORMAT_D32_SFLOAT = 126,
    VK_FORMAT_S8_UINT = 127,
    VK_FORMAT_D16_UNORM_S8_UINT = 128,
    VK_FORMAT_D24_UNORM_S8_UINT = 129,
    VK_FORMAT_D32_SFLOAT_S8_UINT = 130,
    VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131,
    VK_FORMAT_BC1_RGB_SRGB_BLOCK = 132,
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 133,
    VK_FORMAT_BC1_RGBA_SRGB_BLOCK = 134,
    VK_FORMAT_BC2_UNORM_BLOCK = 135,
    VK_FORMAT_BC2_SRGB_BLOCK = 136,
    VK_FORMAT_BC3_UNORM_BLOCK = 137,
    VK_FORMAT_BC3_SRGB_BLOCK = 138,
    VK_FORMAT_BC4_UNORM_BLOCK = 139,
    VK_FORMAT_BC4_SNORM_BLOCK = 140,
    VK_FORMAT_BC5_UNORM_BLOCK = 141,
    VK_FORMAT_BC5_SNORM_BLOCK = 142,
    VK_FORMAT_BC6H_UFLOAT_BLOCK = 143,
    VK_FORMAT_BC6H_SFLOAT_BLOCK = 144,
    VK_FORMAT_BC7_UNORM_BLOCK = 145,
    VK_FORMAT_BC7_SRGB_BLOCK = 146,
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK = 147,
    VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK = 148,
    VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = 149,
    VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = 150,
    VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = 151,
    VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = 152,
    VK_FORMAT_EAC_R11_UNORM_BLOCK = 153,
    VK_FORMAT_EAC_R11_SNORM_BLOCK = 154,
    VK_FORMAT_EAC_R11G11_UNORM_BLOCK = 155,
    VK_FORMAT_EAC_R11G11_SNORM_BLOCK = 156,
    VK_FORMAT_ASTC_4x4_UNORM_BLOCK = 157,
    VK_FORMAT_ASTC_4x4_SRGB_BLOCK = 158,
    VK_FORMAT_ASTC_5x4_UNORM_BLOCK = 159,
    VK_FORMAT_ASTC_5x4_SRGB_BLOCK = 160,
    VK_FORMAT_ASTC_5x5_UNORM_BLOCK = 161,
    VK_FORMAT_ASTC_5x5_SRGB_BLOCK = 162,
    VK_FORMAT_ASTC_6x5_UNORM_BLOCK = 163,
    VK_FORMAT_ASTC_6x5_SRGB_BLOCK = 164,
    VK_FORMAT_ASTC_6x6_UNORM_BLOCK = 165,
    VK_FORMAT_ASTC_6x6_SRGB_BLOCK = 166,
    VK_FORMAT_ASTC_8x5_UNORM_BLOCK = 167,
    VK_FORMAT_ASTC_8x5_SRGB_BLOCK = 168,
    VK_FORMAT_ASTC_8x6_UNORM_BLOCK = 169,
    VK_FORMAT_ASTC_8x6_SRGB_BLOCK = 170,
    VK_FORMAT_ASTC_8x8_UNORM_BLOCK = 171,
    VK_FORMAT_ASTC_8x8_SRGB_BLOCK = 172,
    VK_FORMAT_ASTC_10x5_UNORM_BLOCK = 173,
    VK_FORMAT_ASTC_10x5_SRGB_BLOCK = 174,
    VK_FORMAT_ASTC_10x6_UNORM_BLOCK = 175,
    VK_FORMAT_ASTC_10x6_SRGB_BLOCK = 176,
    VK_FORMAT_ASTC_10x8_UNORM_BLOCK = 177,
    VK_FORMAT_ASTC_10x8_SRGB_BLOCK = 178,
    VK_FORMAT_ASTC_10x10_UNORM_BLOCK = 179,
    VK_FORMAT_ASTC_10x10_SRGB_BLOCK = 180,
    VK_FORMAT_ASTC_12x10_UNORM_BLOCK = 181,
    VK_FORMAT_ASTC_12x10_SRGB_BLOCK = 182,
    VK_FORMAT_ASTC_12x12_UNORM_BLOCK = 183,
    VK_FORMAT_ASTC_12x12_SRGB_BLOCK = 184,
    VK_FORMAT_G8B8G8R8_422_UNORM = 1000156000,
    VK_FORMAT_B8G8R8G8_422_UNORM = 1000156001,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM = 1000156002,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM = 1000156003,
    VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM = 1000156004,
    VK_FORMAT_G8_B8R8_2PLANE_422_UNORM = 1000156005,
    VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM = 1000156006,
    VK_FORMAT_R10X6_UNORM_PACK16 = 1000156007,
    VK_FORMAT_R10X6G10X6_UNORM_2PACK16 = 1000156008,
    VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
    VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
    VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
    VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
    VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
    VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
    VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
    VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
    VK_FORMAT_R12X4_UNORM_PACK16 = 1000156017,
    VK_FORMAT_R12X4G12X4_UNORM_2PACK16 = 1000156018,
    VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
    VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
    VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
    VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
    VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
    VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
    VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
    VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
    VK_FORMAT_G16B16G16R16_422_UNORM = 1000156027,
    VK_FORMAT_B16G16R16G16_422_UNORM = 1000156028,
    VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM = 1000156029,
    VK_FORMAT_G16_B16R16_2PLANE_420_UNORM = 1000156030,
    VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM = 1000156031,
    VK_FORMAT_G16_B16R16_2PLANE_422_UNORM = 1000156032,
    VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM = 1000156033,
    VK_FORMAT_G8_B8R8_2PLANE_444_UNORM = 1000330000,
    VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 = 1000330001,
    VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 = 1000330002,
    VK_FORMAT_G16_B16R16_2PLANE_444_UNORM = 1000330003,
    VK_FORMAT_A4R4G4B4_UNORM_PACK16 = 1000340000,
    VK_FORMAT_A4B4G4R4_UNORM_PACK16 = 1000340001,
    VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK = 1000066000,
    VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK = 1000066001,
    VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK = 1000066002,
    VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK = 1000066003,
    VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK = 1000066004,
    VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK = 1000066005,
    VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK = 1000066006,
    VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK = 1000066007,
    VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK = 1000066008,
    VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK = 1000066009,
    VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK = 1000066010,
    VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK = 1000066011,
    VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK = 1000066012,
    VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK = 1000066013,
    VK_FORMAT_A1B5G5R5_UNORM_PACK16 = 1000470000,
    VK_FORMAT_A8_UNORM = 1000470001,
    VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
    VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
    VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
    VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
    VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
    VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
    VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
    VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
    VK_FORMAT_R16G16_SFIXED5_NV = 1000464000,
    VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
    VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
    VK_FORMAT_G8B8G8R8_422_UNORM_KHR = VK_FORMAT_G8B8G8R8_422_UNORM,
    VK_FORMAT_B8G8R8G8_422_UNORM_KHR = VK_FORMAT_B8G8R8G8_422_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
    VK_FORMAT_R10X6_UNORM_PACK16_KHR = VK_FORMAT_R10X6_UNORM_PACK16,
    VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
    VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
    VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
    VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
    VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
    VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
    VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
    VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
    VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
    VK_FORMAT_R12X4_UNORM_PACK16_KHR = VK_FORMAT_R12X4_UNORM_PACK16,
    VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
    VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
    VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
    VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
    VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
    VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
    VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
    VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
    VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
    VK_FORMAT_G16B16G16R16_422_UNORM_KHR = VK_FORMAT_G16B16G16R16_422_UNORM,
    VK_FORMAT_B16G16R16G16_422_UNORM_KHR = VK_FORMAT_B16G16R16G16_422_UNORM,
    VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
    VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
    VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
    VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
    VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
    VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
    VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
    VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
    VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT = VK_FORMAT_A4R4G4B4_UNORM_PACK16,
    VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT = VK_FORMAT_A4B4G4R4_UNORM_PACK16,
    // VK_FORMAT_R16G16_S10_5_NV is a deprecated alias
    VK_FORMAT_R16G16_S10_5_NV = VK_FORMAT_R16G16_SFIXED5_NV,
    VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR = VK_FORMAT_A1B5G5R5_UNORM_PACK16,
    VK_FORMAT_A8_UNORM_KHR = VK_FORMAT_A8_UNORM,
    VK_FORMAT_MAX_ENUM = 0x7FFFFFFF
} VkFormat;
#endif

void alimerGetVersion(uint32_t* major, uint32_t* minor, uint32_t* patch)
{
    if (major) *major = ALIMER_VERSION_MAJOR;
    if (minor) *minor = ALIMER_VERSION_MINOR;
    if (patch) *patch = ALIMER_VERSION_PATCH;
}

/* Memory */
// TODO: Add custom memory allocation
// TODO: Add tracy profile (TracyCAlloc)
void* alimerCalloc(size_t count, size_t size)
{
    if (count && size)
    {
        void* block;

        if (count > SIZE_MAX / size)
        {
            alimerLogError(LogCategory_System, "Allocation size overflow");
            return NULL;
        }

        block = malloc(count * size);
        if (block)
        {
            return memset(block, 0, count * size);
        }
        else
        {
            alimerLogFatal(LogCategory_System, "Out of memory");
            return nullptr;
        }
    }

    return nullptr;
}

void* alimerMalloc(size_t size)
{
    return alimerCalloc(1, size);
}

void* alimerRealloc(void* old, size_t size)
{
    void* data = realloc(old, size);
    if (!data) abort();
    //alimerProfileAlloc(data, size);
    return data;
}

void alimerFree(void* data)
{
    //alimerProfileFree(data);
    free(data);
}

Blob* alimerBlobCreate(void* data, size_t size, const char* name)
{
    Blob* blob = ALIMER_ALLOC(Blob);
    blob->ref = 1;
    blob->data = data;
    blob->size = size;
    if (name)
        blob->name = _alimer_strdup(name);
    return blob;
}

void alimerBlobDestroy(Blob* blob)
{
    alimerFree(blob->data);
    alimerFree(blob->name);
    alimerFree(blob);
}

char* _alimer_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = (char*)alimerCalloc(length + 1, 1);
    memcpy(result, source, length + 1);
    return result;
}

/* PixelFormat */
// Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
static const PixelFormatInfo kPixelFormatInfo[] = {
    //        format                   name             bytes blk         kind               red   green   blue  alpha  depth  stencl signed  srgb
    { PixelFormat_Undefined,      "Undefined",          0,   0, 0,  _PixelFormatKind_Force32 },
    // 8-bit formats
    { PixelFormat_R8Unorm,        "R8Unorm",            1,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_R8Snorm,        "R8Snorm",            1,   1, 1, PixelFormatKind_Snorm },
    { PixelFormat_R8Uint,         "R8Uint",             1,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_R8Sint,         "R8USnt",             1,   1, 1, PixelFormatKind_Sint },
    // 16-bit formats
    { PixelFormat_R16Unorm,         "R16Unorm",         2,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_R16Snorm,         "R16Snorm",         2,   1, 1, PixelFormatKind_Snorm },
    { PixelFormat_R16Uint,          "R16Uint",          2,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_R16Sint,          "R16Sint",          2,   1, 1, PixelFormatKind_Sint },
    { PixelFormat_R16Float,         "R16Float",         2,   1, 1, PixelFormatKind_Float },
    { PixelFormat_RG8Unorm,         "RG8Unorm",         2,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_RG8Snorm,         "RG8Snorm",         2,   1, 1, PixelFormatKind_Snorm },
    { PixelFormat_RG8Uint,          "RG8Uint",          2,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_RG8Sint,          "RG8Sint",          2,   1, 1, PixelFormatKind_Sint },
    // Packed 16-Bit formats
    { PixelFormat_B5G6R5Unorm,      "B5G6R5Unorm",      2,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_BGR5A1Unorm,      "BGR5A1Unorm",      2,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_BGRA4Unorm,       "BGRA4Unorm",       2,   1, 1, PixelFormatKind_Unorm },
    // 32-bit formats
    { PixelFormat_R32Uint,          "R32Uint",          4,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_R32Sint,          "R32Sint",          4,   1, 1, PixelFormatKind_Sint },
    { PixelFormat_R32Float,         "R32Float",         4,   1, 1, PixelFormatKind_Float },
    { PixelFormat_RG16Unorm,        "RG16Unorm",        4,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_RG16Snorm,        "RG16Snorm",        4,   1, 1, PixelFormatKind_Snorm },
    { PixelFormat_RG16Uint,         "RG16Uint",         4,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_RG16Sint,         "RG16Sint",         4,   1, 1, PixelFormatKind_Sint },
    { PixelFormat_RG16Float,        "RG16Float",        4,   1, 1, PixelFormatKind_Float },
    { PixelFormat_RGBA8Uint,        "RGBA8Uint",        4,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_RGBA8Sint,        "RGBA8Sint",        4,   1, 1, PixelFormatKind_Sint },
    { PixelFormat_RGBA8Unorm,       "RGBA8Unorm",       4,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_RGBA8UnormSrgb,   "RGBA8UnormSrgb",   4,   1, 1, PixelFormatKind_UnormSrgb  },
    { PixelFormat_RGBA8Snorm,       "RGBA8Snorm",       4,   1, 1, PixelFormatKind_Snorm },
    { PixelFormat_BGRA8Unorm,       "BGRA8Unorm",       4,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_BGRA8UnormSrgb,   "BGRA8UnormSrgb",   4,   1, 1, PixelFormatKind_UnormSrgb },
    // Packed 32-Bit Pixel Formats
    { PixelFormat_RGB10A2Unorm,    "RGB10A2Unorm",      4,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_RGB10A2Uint,     "RGB10A2Uint",       4,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_RG11B10Ufloat,   "RG11B10Ufloat",     4,   1, 1, PixelFormatKind_Float },
    { PixelFormat_RGB9E5Ufloat,    "RGB9E5Ufloat",      4,   1, 1, PixelFormatKind_Float },
    // 64-bit formats
    { PixelFormat_RG32Uint,         "RG32Uint",         8,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_RG32Sint,         "RG32Sint",         8,   1, 1, PixelFormatKind_Sint },
    { PixelFormat_RG32Float,        "RG32Float",        8,   1, 1, PixelFormatKind_Float },
    { PixelFormat_RGBA16Unorm,      "RGBA16Unorm",      8,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_RGBA16Snorm,      "RGBA16Snorm",      8,   1, 1, PixelFormatKind_Snorm },
    { PixelFormat_RGBA16Uint,       "RGBA16Uint",       8,   1, 1, PixelFormatKind_Uint },
    { PixelFormat_RGBA16Sint,       "RGBA16Sint",       8,   1, 1, PixelFormatKind_Sint },
    { PixelFormat_RGBA16Float,      "RGBA16Float",      8,   1, 1, PixelFormatKind_Float },
    // 128-bit formats
    { PixelFormat_RGBA32Uint,       "RGBA32Uint",       16,  1, 1, PixelFormatKind_Uint },
    { PixelFormat_RGBA32Sint,       "RGBA32Sint",       16,  1, 1, PixelFormatKind_Sint },
    { PixelFormat_RGBA32Float,      "RGBA32Float",      16,  1, 1, PixelFormatKind_Float },
    // Depth-stencil formats
    //{ PixelFormat_Stencil8,               "Stencil8",                 4,   1, 1, VGPUFormatKind_Float },
    { PixelFormat_Depth16Unorm,           "Depth16Unorm",             2,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_Depth32Float,           "Depth32Float",             4,   1, 1, PixelFormatKind_Float },
    { PixelFormat_Depth24UnormStencil8,   "Depth24UnormStencil8",     4,   1, 1, PixelFormatKind_Unorm },
    { PixelFormat_Depth32FloatStencil8,   "Depth32FloatStencil8",     8,   1, 1, PixelFormatKind_Float },
    // BC compressed formats
    { PixelFormat_BC1RGBAUnorm,       "BC1RGBAUnorm",         8,   4, 4, PixelFormatKind_Unorm },
    { PixelFormat_BC1RGBAUnormSrgb,   "BC1RGBAUnormSrgb",     8,   4, 4, PixelFormatKind_UnormSrgb  },
    { PixelFormat_BC2RGBAUnorm,       "BC2RGBAUnorm",         16,  4, 4, PixelFormatKind_Unorm },
    { PixelFormat_BC2RGBAUnormSrgb,   "BC2RGBAUnormSrgb",     16,  4, 4, PixelFormatKind_UnormSrgb  },
    { PixelFormat_BC3RGBAUnorm,       "BC3RGBAUnorm",         16,  4, 4, PixelFormatKind_Unorm },
    { PixelFormat_BC3RGBAUnormSrgb,   "BC3RGBAUnormSrgb",     16,  4, 4, PixelFormatKind_UnormSrgb  },
    { PixelFormat_BC4RUnorm,          "BC4RUnorm",            8,   4, 4, PixelFormatKind_Unorm },
    { PixelFormat_BC4RSnorm,          "BC4RSnorm",            8,   4, 4, PixelFormatKind_Snorm },
    { PixelFormat_BC5RGUnorm,         "BC5Unorm",             16,  4, 4, PixelFormatKind_Unorm },
    { PixelFormat_BC5RGSnorm,         "BC5Snorm",             16,  4, 4, PixelFormatKind_Snorm },
    { PixelFormat_BC6HRGBUfloat,      "BC6HRGBUfloat",        16,  4, 4, PixelFormatKind_Float },
    { PixelFormat_BC6HRGBFloat,       "BC6HRGBFloat",         16,  4, 4, PixelFormatKind_Float },
    { PixelFormat_BC7RGBAUnorm,       "BC7RGBAUnorm",         16,  4, 4, PixelFormatKind_Unorm },
    { PixelFormat_BC7RGBAUnormSrgb,   "BC7RGBAUnormSrgb",     16,  4, 4, PixelFormatKind_UnormSrgb },
    // ETC2/EAC compressed formats
    { PixelFormat_ETC2RGBA8Unorm,      "ETC2RGBA8Unorm",       8,   4, 4, PixelFormatKind_Unorm },
    { PixelFormat_ETC2RGBA8UnormSrgb,  "ETC2RGBA8UnormSrgb",   8,   4, 4, PixelFormatKind_UnormSrgb },
    { PixelFormat_ETC2RGB8A1Unorm,     "ETC2RGB8A1Unorm,",     16,   4, 4, PixelFormatKind_Unorm },
    { PixelFormat_ETC2RGB8A1UnormSrgb, "ETC2RGB8A1UnormSrgb",  16,   4, 4, PixelFormatKind_UnormSrgb },
    { PixelFormat_ETC2RGBA8Unorm,      "ETC2RGBA8Unorm",       16,   4, 4, PixelFormatKind_Unorm },
    { PixelFormat_ETC2RGBA8UnormSrgb,  "ETC2RGBA8UnormSrgb",   16,   4, 4, PixelFormatKind_UnormSrgb },
    { PixelFormat_EACR11Unorm,         "EACR11Unorm",          8,    4, 4, PixelFormatKind_Unorm },
    { PixelFormat_EACR11Snorm,         "EACR11Snorm",          8,    4, 4, PixelFormatKind_Snorm },
    { PixelFormat_EACRG11Unorm,        "EACRG11Unorm",         16,   4, 4, PixelFormatKind_Unorm },
    { PixelFormat_EACRG11Snorm,        "EACRG11Snorm",         16,   4, 4, PixelFormatKind_Snorm },
    // ASTC compressed formats
    { PixelFormat_ASTC4x4Unorm,        "ASTC4x4Unorm",         16,   4,   4, PixelFormatKind_Unorm },
    { PixelFormat_ASTC4x4UnormSrgb,    "ASTC4x4UnormSrgb",     16,   4,   4, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC5x4Unorm,        "ASTC5x4Unorm",         16,   5,   4, PixelFormatKind_Unorm },
    { PixelFormat_ASTC5x4UnormSrgb,    "ASTC5x4UnormSrgb",     16,   5,   4, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC5x5Unorm,        "ASTC5x5UnormSrgb",     16,   5,   5, PixelFormatKind_Unorm },
    { PixelFormat_ASTC5x5UnormSrgb,    "ASTC5x5UnormSrgb",     16,   5,   5, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC6x5Unorm,        "ASTC6x5Unorm",         16,   6,   5, PixelFormatKind_Unorm },
    { PixelFormat_ASTC6x5UnormSrgb,    "ASTC6x5UnormSrgb",     16,   6,   5, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC6x6Unorm,        "ASTC6x6Unorm",         16,   6,   6, PixelFormatKind_Unorm },
    { PixelFormat_ASTC6x6UnormSrgb,    "ASTC6x6UnormSrgb",     16,   6,   6, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC8x5Unorm,        "ASTC8x5Unorm",         16,   8,   5, PixelFormatKind_Unorm },
    { PixelFormat_ASTC8x5UnormSrgb,    "ASTC8x5UnormSrgb",     16,   8,   5, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC8x6Unorm,        "ASTC8x6Unorm",         16,   8,   6, PixelFormatKind_Unorm },
    { PixelFormat_ASTC8x6UnormSrgb,    "ASTC8x6UnormSrgb",     16,   8,   6, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC8x8Unorm,        "ASTC8x8Unorm",         16,   8,   8, PixelFormatKind_Unorm },
    { PixelFormat_ASTC8x8UnormSrgb,    "ASTC8x8UnormSrgb",     16,   8,   8, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC10x5Unorm,       "ASTC10x5Unorm",        16,   10,  5, PixelFormatKind_Unorm },
    { PixelFormat_ASTC10x5UnormSrgb,   "ASTC10x5UnormSrgb",    16,   10,  5, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC10x6Unorm,       "ASTC10x6Unorm",        16,   10,  6, PixelFormatKind_Unorm },
    { PixelFormat_ASTC10x6UnormSrgb,   "ASTC10x6UnormSrgb",    16,   10,  6, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC10x8Unorm,       "ASTC10x8Unorm",        16,   10,  8, PixelFormatKind_Unorm },
    { PixelFormat_ASTC10x8UnormSrgb,   "ASTC10x8UnormSrgb",    16,   10,  8, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC10x10Unorm,      "ASTC10x10Unorm",       16,   10,  10, PixelFormatKind_Unorm },
    { PixelFormat_ASTC10x10UnormSrgb,  "ASTC10x10UnormSrgb",   16,   10,  10, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC12x10Unorm,      "ASTC12x10Unorm",       16,   12,  10, PixelFormatKind_Unorm },
    { PixelFormat_ASTC12x10UnormSrgb,  "ASTC12x10UnormSrgb",   16,   12,  10, PixelFormatKind_UnormSrgb },
    { PixelFormat_ASTC12x12Unorm,      "ASTC12x12Unorm",       16,   12,  12, PixelFormatKind_Unorm },
    { PixelFormat_ASTC12x12UnormSrgb,  "ASTC12x12UnormSrgb",   16,   12,  12, PixelFormatKind_UnormSrgb },
    // ASTC HDR compressed formats
    { PixelFormat_ASTC4x4HDR,          "ASTC4x4HDR",           16,   4, 4,    PixelFormatKind_Float },
    { PixelFormat_ASTC5x4HDR,          "ASTC5x4HDR",           16,   5, 4,    PixelFormatKind_Float },
    { PixelFormat_ASTC5x5HDR,          "ASTC5x5HDR",           16,   5, 5,    PixelFormatKind_Float },
    { PixelFormat_ASTC6x5HDR,          "ASTC6x5HDR",           16,   6, 5,    PixelFormatKind_Float },
    { PixelFormat_ASTC6x6HDR,          "ASTC6x6HDR",           16,   6, 6,    PixelFormatKind_Float },
    { PixelFormat_ASTC8x5HDR,          "ASTC8x5HDR",           16,   8, 5,    PixelFormatKind_Float },
    { PixelFormat_ASTC8x6HDR,          "ASTC8x6HDR",           16,   8, 6,    PixelFormatKind_Float },
    { PixelFormat_ASTC8x8HDR,          "ASTC8x8HDR",           16,   8, 8,    PixelFormatKind_Float },
    { PixelFormat_ASTC10x5HDR,         "ASTC10x5HDR",          16,   10, 5,   PixelFormatKind_Float },
    { PixelFormat_ASTC10x6HDR,         "ASTC10x6HDR",          16,   10, 6,   PixelFormatKind_Float },
    { PixelFormat_ASTC10x8HDR,         "ASTC10x8HDR",          16,   10, 8,   PixelFormatKind_Float },
    { PixelFormat_ASTC10x10HDR,        "ASTC10x10HDR",         16,   10, 10,  PixelFormatKind_Float },
    { PixelFormat_ASTC12x10HDR,        "ASTC12x10HDR",         16,   12, 10,  PixelFormatKind_Float },
    { PixelFormat_ASTC12x12HDR,        "ASTC12x12HDR",         16,   12, 12,  PixelFormatKind_Float },
};

static_assert(
    sizeof(kPixelFormatInfo) / sizeof(PixelFormatInfo) == size_t(_PixelFormat_Count),
    "The format info table doesn't have the right number of elements"
    );

static const PixelFormatInfo& GetPixelFormatInfo(PixelFormat format)
{
    if (format >= _PixelFormat_Count)
        return kPixelFormatInfo[0]; // Unknown

    const PixelFormatInfo& info = kPixelFormatInfo[format];
    ALIMER_ASSERT(info.format == format);
    return info;
}

void alimerPixelFormatGetInfo(PixelFormat format, PixelFormatInfo* pInfo)
{
    ALIMER_ASSERT(pInfo);
    ALIMER_ASSERT(kPixelFormatInfo[format].format == format);

    *pInfo = kPixelFormatInfo[format];
}

bool alimerPixelFormatIsDepth(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat_Depth16Unorm:
        case PixelFormat_Depth24UnormStencil8:
        case PixelFormat_Depth32Float:
        case PixelFormat_Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool alimerPixelFormatIsStencil(PixelFormat format)
{
    switch (format)
    {
        //case PixelFormat_Stencil8:
        case PixelFormat_Depth24UnormStencil8:
        case PixelFormat_Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool alimerPixelFormatIsDepthStencil(PixelFormat format)
{
    switch (format)
    {
        //case PixelFormat_Stencil8:
        case PixelFormat_Depth16Unorm:
        case PixelFormat_Depth24UnormStencil8:
        case PixelFormat_Depth32Float:
        case PixelFormat_Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool alimerPixelFormatIsDepthOnly(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat_Depth16Unorm:
        case PixelFormat_Depth32Float:
            return true;
        default:
            return false;
    }
}

bool alimerPixelFormatIsCompressed(PixelFormat format)
{
    const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
    return formatInfo.blockWidth > 1 || formatInfo.blockHeight > 1;
}

bool alimerPixelFormatIsCompressedBC(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat_BC1RGBAUnorm:
        case PixelFormat_BC1RGBAUnormSrgb:
        case PixelFormat_BC2RGBAUnorm:
        case PixelFormat_BC2RGBAUnormSrgb:
        case PixelFormat_BC3RGBAUnorm:
        case PixelFormat_BC3RGBAUnormSrgb:
        case PixelFormat_BC4RUnorm:
        case PixelFormat_BC4RSnorm:
        case PixelFormat_BC5RGUnorm:
        case PixelFormat_BC5RGSnorm:
        case PixelFormat_BC6HRGBUfloat:
        case PixelFormat_BC6HRGBFloat:
        case PixelFormat_BC7RGBAUnorm:
        case PixelFormat_BC7RGBAUnormSrgb:
            return true;
        default:
            return false;
    }
}

bool alimerPixelFormatIsCompressedASTC(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat_ASTC4x4Unorm:
        case PixelFormat_ASTC4x4UnormSrgb:
        case PixelFormat_ASTC5x4Unorm:
        case PixelFormat_ASTC5x4UnormSrgb:
        case PixelFormat_ASTC5x5Unorm:
        case PixelFormat_ASTC5x5UnormSrgb:
        case PixelFormat_ASTC6x5Unorm:
        case PixelFormat_ASTC6x5UnormSrgb:
        case PixelFormat_ASTC6x6Unorm:
        case PixelFormat_ASTC6x6UnormSrgb:
        case PixelFormat_ASTC8x5Unorm:
        case PixelFormat_ASTC8x5UnormSrgb:
        case PixelFormat_ASTC8x6Unorm:
        case PixelFormat_ASTC8x6UnormSrgb:
        case PixelFormat_ASTC8x8Unorm:
        case PixelFormat_ASTC8x8UnormSrgb:
        case PixelFormat_ASTC10x5Unorm:
        case PixelFormat_ASTC10x5UnormSrgb:
        case PixelFormat_ASTC10x6Unorm:
        case PixelFormat_ASTC10x6UnormSrgb:
        case PixelFormat_ASTC10x8Unorm:
        case PixelFormat_ASTC10x8UnormSrgb:
        case PixelFormat_ASTC10x10Unorm:
        case PixelFormat_ASTC10x10UnormSrgb:
        case PixelFormat_ASTC12x10Unorm:
        case PixelFormat_ASTC12x10UnormSrgb:
        case PixelFormat_ASTC12x12Unorm:
            return true;

        case PixelFormat_ASTC4x4HDR:
        case PixelFormat_ASTC5x4HDR:
        case PixelFormat_ASTC5x5HDR:
        case PixelFormat_ASTC6x5HDR:
        case PixelFormat_ASTC6x6HDR:
        case PixelFormat_ASTC8x5HDR:
        case PixelFormat_ASTC8x6HDR:
        case PixelFormat_ASTC8x8HDR:
        case PixelFormat_ASTC10x5HDR:
        case PixelFormat_ASTC10x6HDR:
        case PixelFormat_ASTC10x8HDR:
        case PixelFormat_ASTC10x10HDR:
        case PixelFormat_ASTC12x10HDR:
        case PixelFormat_ASTC12x12HDR:
            return true;

        default:
            return false;
    }
}

const char* alimerPixelFormatGetName(PixelFormat format)
{
    const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
    return formatInfo.name;
}

uint32_t alimerPixelFormatGetBytesPerBlock(PixelFormat format)
{
    const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
    return formatInfo.bytesPerBlock;
}

PixelFormatKind alimerPixelFormatGetKind(PixelFormat format)
{
    const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
    return formatInfo.kind;
}

bool alimerPixelFormatIsInteger(PixelFormat format)
{
    const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
    return formatInfo.kind == PixelFormatKind_Uint || formatInfo.kind == PixelFormatKind_Sint;
}

bool alimerPixelFormatIsSrgb(PixelFormat format)
{
    const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
    return formatInfo.kind == PixelFormatKind_UnormSrgb;
}

PixelFormat alimerPixelFormatSrgbToLinear(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat_RGBA8UnormSrgb:       return PixelFormat_RGBA8Unorm;
        case PixelFormat_BGRA8UnormSrgb:       return PixelFormat_BGRA8Unorm;
            // Bc compressed formats
        case PixelFormat_BC1RGBAUnormSrgb:     return PixelFormat_BC1RGBAUnorm;
        case PixelFormat_BC2RGBAUnormSrgb:     return PixelFormat_BC2RGBAUnorm;
        case PixelFormat_BC3RGBAUnormSrgb:     return PixelFormat_BC3RGBAUnorm;
        case PixelFormat_BC7RGBAUnormSrgb:     return PixelFormat_BC7RGBAUnorm;

            // Etc2/Eac compressed formats
        case PixelFormat_ETC2RGB8UnormSrgb:    return PixelFormat_ETC2RGB8Unorm;
        case PixelFormat_ETC2RGB8A1UnormSrgb:  return PixelFormat_ETC2RGB8A1Unorm;
        case PixelFormat_ETC2RGBA8UnormSrgb:   return PixelFormat_ETC2RGBA8Unorm;

            // Astc compressed formats
        case PixelFormat_ASTC4x4UnormSrgb:     return PixelFormat_ASTC4x4Unorm;
        case PixelFormat_ASTC5x4UnormSrgb:     return PixelFormat_ASTC5x4Unorm;
        case PixelFormat_ASTC5x5UnormSrgb:     return PixelFormat_ASTC5x5Unorm;
        case PixelFormat_ASTC6x5UnormSrgb:     return PixelFormat_ASTC6x5Unorm;
        case PixelFormat_ASTC6x6UnormSrgb:     return PixelFormat_ASTC6x6Unorm;
        case PixelFormat_ASTC8x5UnormSrgb:     return PixelFormat_ASTC8x5Unorm;
        case PixelFormat_ASTC8x6UnormSrgb:     return PixelFormat_ASTC8x6Unorm;
        case PixelFormat_ASTC8x8UnormSrgb:     return PixelFormat_ASTC8x8Unorm;
        case PixelFormat_ASTC10x5UnormSrgb:    return PixelFormat_ASTC10x5Unorm;
        case PixelFormat_ASTC10x6UnormSrgb:    return PixelFormat_ASTC10x6Unorm;
        case PixelFormat_ASTC10x8UnormSrgb:    return PixelFormat_ASTC10x8Unorm;
        case PixelFormat_ASTC10x10UnormSrgb:   return PixelFormat_ASTC10x10Unorm;
        case PixelFormat_ASTC12x10UnormSrgb:   return PixelFormat_ASTC12x10Unorm;
        case PixelFormat_ASTC12x12UnormSrgb:   return PixelFormat_ASTC12x12Unorm;

        default:
            ALIMER_ASSERT(alimerPixelFormatIsSrgb(format) == false);
            return format;
    }
}

PixelFormat alimerPixelFormatLinearToSrgb(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat_RGBA8Unorm:       return PixelFormat_RGBA8UnormSrgb;
        case PixelFormat_BGRA8Unorm:       return PixelFormat_BGRA8UnormSrgb;

        // Bc compressed formats
        case PixelFormat_BC1RGBAUnorm:     return PixelFormat_BC1RGBAUnormSrgb;
        case PixelFormat_BC2RGBAUnorm:     return PixelFormat_BC2RGBAUnormSrgb;
        case PixelFormat_BC3RGBAUnorm:     return PixelFormat_BC3RGBAUnormSrgb;
        case PixelFormat_BC7RGBAUnorm:     return PixelFormat_BC7RGBAUnormSrgb;

        // Etc2/Eac compressed formats
        case PixelFormat_ETC2RGB8Unorm:    return PixelFormat_ETC2RGB8UnormSrgb;
        case PixelFormat_ETC2RGB8A1Unorm:  return PixelFormat_ETC2RGB8A1UnormSrgb;
        case PixelFormat_ETC2RGBA8Unorm:   return PixelFormat_ETC2RGBA8UnormSrgb;

        // Astc compressed formats
        case PixelFormat_ASTC4x4Unorm:     return PixelFormat_ASTC4x4UnormSrgb;
        case PixelFormat_ASTC5x4Unorm:     return PixelFormat_ASTC5x4UnormSrgb;
        case PixelFormat_ASTC5x5Unorm:     return PixelFormat_ASTC5x5UnormSrgb;
        case PixelFormat_ASTC6x5Unorm:     return PixelFormat_ASTC6x5UnormSrgb;
        case PixelFormat_ASTC6x6Unorm:     return PixelFormat_ASTC6x6UnormSrgb;
        case PixelFormat_ASTC8x5Unorm:     return PixelFormat_ASTC8x5UnormSrgb;
        case PixelFormat_ASTC8x6Unorm:     return PixelFormat_ASTC8x6UnormSrgb;
        case PixelFormat_ASTC8x8Unorm:     return PixelFormat_ASTC8x8UnormSrgb;
        case PixelFormat_ASTC10x5Unorm:    return PixelFormat_ASTC10x5UnormSrgb;
        case PixelFormat_ASTC10x6Unorm:    return PixelFormat_ASTC10x6UnormSrgb;
        case PixelFormat_ASTC10x8Unorm:    return PixelFormat_ASTC10x8UnormSrgb;
        case PixelFormat_ASTC10x10Unorm:   return PixelFormat_ASTC10x10UnormSrgb;
        case PixelFormat_ASTC12x10Unorm:   return PixelFormat_ASTC12x10UnormSrgb;
        case PixelFormat_ASTC12x12Unorm:   return PixelFormat_ASTC12x12UnormSrgb;

        default:
            return format;
    }
}

uint32_t alimerPixelFormatToDxgiFormat(PixelFormat format)
{
    switch (format)
    {
        // 8-bit formats
        case PixelFormat_R8Unorm:                  return DXGI_FORMAT_R8_UNORM;
        case PixelFormat_R8Snorm:                  return DXGI_FORMAT_R8_SNORM;
        case PixelFormat_R8Uint:                   return DXGI_FORMAT_R8_UINT;
        case PixelFormat_R8Sint:                   return DXGI_FORMAT_R8_SINT;
            // 16-bit formats
        case PixelFormat_R16Unorm:                 return DXGI_FORMAT_R16_UNORM;
        case PixelFormat_R16Snorm:                 return DXGI_FORMAT_R16_SNORM;
        case PixelFormat_R16Uint:                  return DXGI_FORMAT_R16_UINT;
        case PixelFormat_R16Sint:                  return DXGI_FORMAT_R16_SINT;
        case PixelFormat_R16Float:                 return DXGI_FORMAT_R16_FLOAT;
        case PixelFormat_RG8Unorm:                 return DXGI_FORMAT_R8G8_UNORM;
        case PixelFormat_RG8Snorm:                 return DXGI_FORMAT_R8G8_SNORM;
        case PixelFormat_RG8Uint:                  return DXGI_FORMAT_R8G8_UINT;
        case PixelFormat_RG8Sint:                  return DXGI_FORMAT_R8G8_SINT;
            // Packed 16-Bit Pixel Formats
        case PixelFormat_B5G6R5Unorm:              return DXGI_FORMAT_B5G6R5_UNORM;
        case PixelFormat_BGR5A1Unorm:              return DXGI_FORMAT_B5G5R5A1_UNORM;
        case PixelFormat_BGRA4Unorm:               return DXGI_FORMAT_B4G4R4A4_UNORM;
            // 32-bit formats
        case PixelFormat_R32Uint:                   return DXGI_FORMAT_R32_UINT;
        case PixelFormat_R32Sint:                   return DXGI_FORMAT_R32_SINT;
        case PixelFormat_R32Float:                  return DXGI_FORMAT_R32_FLOAT;
        case PixelFormat_RG16Unorm:                 return DXGI_FORMAT_R16G16_UNORM;
        case PixelFormat_RG16Snorm:                 return DXGI_FORMAT_R16G16_SNORM;
        case PixelFormat_RG16Uint:                  return DXGI_FORMAT_R16G16_UINT;
        case PixelFormat_RG16Sint:                  return DXGI_FORMAT_R16G16_SINT;
        case PixelFormat_RG16Float:                 return DXGI_FORMAT_R16G16_FLOAT;
        case PixelFormat_RGBA8Unorm:                return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat_RGBA8UnormSrgb:            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case PixelFormat_RGBA8Snorm:                return DXGI_FORMAT_R8G8B8A8_SNORM;
        case PixelFormat_RGBA8Uint:                 return DXGI_FORMAT_R8G8B8A8_UINT;
        case PixelFormat_RGBA8Sint:                 return DXGI_FORMAT_R8G8B8A8_SINT;
        case PixelFormat_BGRA8Unorm:                return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat_BGRA8UnormSrgb:            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            // Packed 32-Bit formats
        case PixelFormat_RGB10A2Unorm:              return DXGI_FORMAT_R10G10B10A2_UNORM;
        case PixelFormat_RGB10A2Uint:               return DXGI_FORMAT_R10G10B10A2_UINT;
        case PixelFormat_RG11B10Ufloat:             return DXGI_FORMAT_R11G11B10_FLOAT;
        case PixelFormat_RGB9E5Ufloat:              return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            // 64-Bit formats
        case PixelFormat_RG32Uint:                 return DXGI_FORMAT_R32G32_UINT;
        case PixelFormat_RG32Sint:                 return DXGI_FORMAT_R32G32_SINT;
        case PixelFormat_RG32Float:                return DXGI_FORMAT_R32G32_FLOAT;
        case PixelFormat_RGBA16Unorm:              return DXGI_FORMAT_R16G16B16A16_UNORM;
        case PixelFormat_RGBA16Snorm:              return DXGI_FORMAT_R16G16B16A16_SNORM;
        case PixelFormat_RGBA16Uint:               return DXGI_FORMAT_R16G16B16A16_UINT;
        case PixelFormat_RGBA16Sint:               return DXGI_FORMAT_R16G16B16A16_SINT;
        case PixelFormat_RGBA16Float:              return DXGI_FORMAT_R16G16B16A16_FLOAT;
            // 128-Bit formats
        case PixelFormat_RGBA32Uint:               return DXGI_FORMAT_R32G32B32A32_UINT;
        case PixelFormat_RGBA32Sint:               return DXGI_FORMAT_R32G32B32A32_SINT;
        case PixelFormat_RGBA32Float:              return DXGI_FORMAT_R32G32B32A32_FLOAT;
            // Depth-stencil formats
        //case PixelFormat_Stencil8:                 return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat_Depth16Unorm:              return DXGI_FORMAT_D16_UNORM;
        case PixelFormat_Depth24UnormStencil8:      return DXGI_FORMAT_D24_UNORM_S8_UINT; // UsePackedDepth24UnormStencil8Format ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case PixelFormat_Depth32Float:              return DXGI_FORMAT_D32_FLOAT;
        case PixelFormat_Depth32FloatStencil8:      return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            // Compressed BC formats
        case PixelFormat_BC1RGBAUnorm:             return DXGI_FORMAT_BC1_UNORM;
        case PixelFormat_BC1RGBAUnormSrgb:         return DXGI_FORMAT_BC1_UNORM_SRGB;
        case PixelFormat_BC2RGBAUnorm:             return DXGI_FORMAT_BC2_UNORM;
        case PixelFormat_BC2RGBAUnormSrgb:         return DXGI_FORMAT_BC2_UNORM_SRGB;
        case PixelFormat_BC3RGBAUnorm:             return DXGI_FORMAT_BC3_UNORM;
        case PixelFormat_BC3RGBAUnormSrgb:         return DXGI_FORMAT_BC3_UNORM_SRGB;
        case PixelFormat_BC4RSnorm:                return DXGI_FORMAT_BC4_UNORM;
        case PixelFormat_BC4RUnorm:                return DXGI_FORMAT_BC4_SNORM;
        case PixelFormat_BC5RGUnorm:               return DXGI_FORMAT_BC5_UNORM;
        case PixelFormat_BC5RGSnorm:               return DXGI_FORMAT_BC5_SNORM;
        case PixelFormat_BC6HRGBUfloat:            return DXGI_FORMAT_BC6H_UF16;
        case PixelFormat_BC6HRGBFloat:             return DXGI_FORMAT_BC6H_SF16;
        case PixelFormat_BC7RGBAUnorm:             return DXGI_FORMAT_BC7_UNORM;
        case PixelFormat_BC7RGBAUnormSrgb:         return DXGI_FORMAT_BC7_UNORM_SRGB;

        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

PixelFormat alimerPixelFormatFromDxgiFormat(uint32_t dxgiFormat)
{
    switch (dxgiFormat)
    {
        // 8-bit formats
        case DXGI_FORMAT_R8_UNORM:                  return PixelFormat_R8Unorm;
        case DXGI_FORMAT_R8_SNORM:                  return PixelFormat_R8Snorm;
        case DXGI_FORMAT_R8_UINT:                   return PixelFormat_R8Uint;
        case DXGI_FORMAT_R8_SINT:                   return PixelFormat_R8Sint;
            // 16-bit formats
        case DXGI_FORMAT_R16_UNORM:                 return PixelFormat_R16Unorm;
        case DXGI_FORMAT_R16_SNORM:                 return PixelFormat_R16Snorm;
        case DXGI_FORMAT_R16_UINT:                  return PixelFormat_R16Uint;
        case DXGI_FORMAT_R16_SINT:                  return PixelFormat_R16Sint;
        case DXGI_FORMAT_R16_FLOAT:                 return PixelFormat_R16Float;
        case DXGI_FORMAT_R8G8_UNORM:                return PixelFormat_RG8Unorm;
        case DXGI_FORMAT_R8G8_SNORM:                return PixelFormat_RG8Snorm;
        case DXGI_FORMAT_R8G8_UINT:                 return PixelFormat_RG8Uint;
        case DXGI_FORMAT_R8G8_SINT:                 return PixelFormat_RG8Sint;
            // Packed 16-Bit Pixel Formats
        case DXGI_FORMAT_B5G6R5_UNORM:              return PixelFormat_B5G6R5Unorm;
        case DXGI_FORMAT_B5G5R5A1_UNORM:            return PixelFormat_BGR5A1Unorm;
        case DXGI_FORMAT_B4G4R4A4_UNORM:            return PixelFormat_BGRA4Unorm;
            // 32-bit formats
        case DXGI_FORMAT_R32_UINT:                  return PixelFormat_R32Uint;
        case DXGI_FORMAT_R32_SINT:                  return PixelFormat_R32Sint;
        case DXGI_FORMAT_R32_FLOAT:                 return PixelFormat_R32Float;
        case DXGI_FORMAT_R16G16_UNORM:              return PixelFormat_RG16Unorm;
        case DXGI_FORMAT_R16G16_SNORM:              return PixelFormat_RG16Snorm;
        case DXGI_FORMAT_R16G16_UINT:               return PixelFormat_RG16Uint;
        case DXGI_FORMAT_R16G16_SINT:               return PixelFormat_RG16Sint;
        case DXGI_FORMAT_R16G16_FLOAT:              return PixelFormat_RG16Float;
        case DXGI_FORMAT_R8G8B8A8_UNORM:            return PixelFormat_RGBA8Unorm;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:       return PixelFormat_RGBA8UnormSrgb;
        case DXGI_FORMAT_R8G8B8A8_SNORM:            return PixelFormat_RGBA8Snorm;
        case DXGI_FORMAT_R8G8B8A8_UINT:             return PixelFormat_RGBA8Uint;
        case DXGI_FORMAT_R8G8B8A8_SINT:             return PixelFormat_RGBA8Sint;
        case DXGI_FORMAT_B8G8R8A8_UNORM:            return PixelFormat_BGRA8Unorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:       return PixelFormat_BGRA8UnormSrgb;
            // Packed 32-Bit formats
        case DXGI_FORMAT_R10G10B10A2_UNORM:         return PixelFormat_RGB10A2Unorm;
        case DXGI_FORMAT_R10G10B10A2_UINT:          return PixelFormat_RGB10A2Uint;
        case DXGI_FORMAT_R11G11B10_FLOAT:           return PixelFormat_RG11B10Ufloat;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:        return PixelFormat_RGB9E5Ufloat;
            // 64-Bit formats
        case DXGI_FORMAT_R32G32_UINT:               return PixelFormat_RG32Uint;
        case DXGI_FORMAT_R32G32_SINT:               return PixelFormat_RG32Sint;
        case DXGI_FORMAT_R32G32_FLOAT:              return PixelFormat_RG32Float;
        case DXGI_FORMAT_R16G16B16A16_UNORM:        return PixelFormat_RGBA16Unorm;
        case DXGI_FORMAT_R16G16B16A16_SNORM:        return PixelFormat_RGBA16Snorm;
        case DXGI_FORMAT_R16G16B16A16_UINT:         return PixelFormat_RGBA16Uint;
        case DXGI_FORMAT_R16G16B16A16_SINT:         return PixelFormat_RGBA16Sint;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:        return PixelFormat_RGBA16Float;
            // 128-Bit formats
        case DXGI_FORMAT_R32G32B32A32_UINT:         return PixelFormat_RGBA32Uint;
        case DXGI_FORMAT_R32G32B32A32_SINT:         return PixelFormat_RGBA32Sint;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:        return PixelFormat_RGBA32Float;
            // Depth-stencil formats
        case DXGI_FORMAT_D16_UNORM:			        return PixelFormat_Depth16Unorm;
        case DXGI_FORMAT_D32_FLOAT:			        return PixelFormat_Depth32Float;
            //case DXGI_FORMAT_D24_UNORM_S8_UINT:		return PixelFormat::Stencil8;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:         return PixelFormat_Depth24UnormStencil8;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:      return PixelFormat_Depth32FloatStencil8;
            // Compressed BC formats
        case DXGI_FORMAT_BC1_UNORM:                 return PixelFormat_BC1RGBAUnorm;
        case DXGI_FORMAT_BC1_UNORM_SRGB:            return PixelFormat_BC1RGBAUnormSrgb;
        case DXGI_FORMAT_BC2_UNORM:                 return PixelFormat_BC2RGBAUnorm;
        case DXGI_FORMAT_BC2_UNORM_SRGB:            return PixelFormat_BC2RGBAUnormSrgb;
        case DXGI_FORMAT_BC3_UNORM:                 return PixelFormat_BC3RGBAUnorm;
        case DXGI_FORMAT_BC3_UNORM_SRGB:            return PixelFormat_BC3RGBAUnormSrgb;
        case DXGI_FORMAT_BC4_UNORM:                 return PixelFormat_BC4RSnorm;
        case DXGI_FORMAT_BC4_SNORM:                 return PixelFormat_BC4RUnorm;
        case DXGI_FORMAT_BC5_UNORM:                 return PixelFormat_BC5RGUnorm;
        case DXGI_FORMAT_BC5_SNORM:                 return PixelFormat_BC5RGSnorm;
        case DXGI_FORMAT_BC6H_UF16:                 return PixelFormat_BC6HRGBUfloat;
        case DXGI_FORMAT_BC6H_SF16:                 return PixelFormat_BC6HRGBFloat;
        case DXGI_FORMAT_BC7_UNORM:                 return PixelFormat_BC7RGBAUnorm;
        case DXGI_FORMAT_BC7_UNORM_SRGB:            return PixelFormat_BC7RGBAUnormSrgb;

        default:
            return PixelFormat_Undefined;
    }
}

uint32_t alimerPixelFormatToVkFormat(PixelFormat format)
{
    switch (format)
    {
        // 8-bit formats
        case PixelFormat_R8Unorm:                    return VK_FORMAT_R8_UNORM;
        case PixelFormat_R8Snorm:                    return VK_FORMAT_R8_SNORM;
        case PixelFormat_R8Uint:                     return VK_FORMAT_R8_UINT;
        case PixelFormat_R8Sint:                     return VK_FORMAT_R8_SINT;
            // 16-bit formats
        case PixelFormat_R16Uint:                    return VK_FORMAT_R16_UINT;
        case PixelFormat_R16Sint:                    return VK_FORMAT_R16_SINT;
        case PixelFormat_R16Unorm:                   return VK_FORMAT_R16_UNORM;
        case PixelFormat_R16Snorm:                   return VK_FORMAT_R16_SNORM;
        case PixelFormat_R16Float:                   return VK_FORMAT_R16_SFLOAT;
        case PixelFormat_RG8Unorm:                   return VK_FORMAT_R8G8_UNORM;
        case PixelFormat_RG8Snorm:                   return VK_FORMAT_R8G8_SNORM;
        case PixelFormat_RG8Uint:                    return VK_FORMAT_R8G8_UINT;
        case PixelFormat_RG8Sint:                    return VK_FORMAT_R8G8_SINT;
            // Packed 16-Bit Pixel Formats
        case PixelFormat_B5G6R5Unorm:                return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case PixelFormat_BGR5A1Unorm:                return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case PixelFormat_BGRA4Unorm:                 return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
            // 32-bit formats
        case PixelFormat_R32Uint:                    return VK_FORMAT_R32_UINT;
        case PixelFormat_R32Sint:                    return VK_FORMAT_R32_SINT;
        case PixelFormat_R32Float:                   return VK_FORMAT_R32_SFLOAT;
        case PixelFormat_RG16Uint:                   return VK_FORMAT_R16G16_UINT;
        case PixelFormat_RG16Sint:                   return VK_FORMAT_R16G16_SINT;
        case PixelFormat_RG16Unorm:                  return VK_FORMAT_R16G16_UNORM;
        case PixelFormat_RG16Snorm:                  return VK_FORMAT_R16G16_SNORM;
        case PixelFormat_RG16Float:                  return VK_FORMAT_R16G16_SFLOAT;
        case PixelFormat_RGBA8Unorm:                 return VK_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat_RGBA8UnormSrgb:             return VK_FORMAT_R8G8B8A8_SRGB;
        case PixelFormat_RGBA8Snorm:                 return VK_FORMAT_R8G8B8A8_SNORM;
        case PixelFormat_RGBA8Uint:                  return VK_FORMAT_R8G8B8A8_UINT;
        case PixelFormat_RGBA8Sint:                  return VK_FORMAT_R8G8B8A8_SINT;
        case PixelFormat_BGRA8Unorm:                 return VK_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat_BGRA8UnormSrgb:             return VK_FORMAT_B8G8R8A8_SRGB;
            // Packed 32-Bit formats
        case PixelFormat_RGB10A2Unorm:               return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case PixelFormat_RGB10A2Uint:                return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case PixelFormat_RG11B10Ufloat:               return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PixelFormat_RGB9E5Ufloat:                return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            // 64-Bit formats
        case PixelFormat_RG32Uint:                   return VK_FORMAT_R32G32_UINT;
        case PixelFormat_RG32Sint:                   return VK_FORMAT_R32G32_SINT;
        case PixelFormat_RG32Float:                  return VK_FORMAT_R32G32_SFLOAT;
        case PixelFormat_RGBA16Uint:                 return VK_FORMAT_R16G16B16A16_UINT;
        case PixelFormat_RGBA16Sint:                 return VK_FORMAT_R16G16B16A16_SINT;
        case PixelFormat_RGBA16Unorm:                return VK_FORMAT_R16G16B16A16_UNORM;
        case PixelFormat_RGBA16Snorm:                return VK_FORMAT_R16G16B16A16_SNORM;
        case PixelFormat_RGBA16Float:                return VK_FORMAT_R16G16B16A16_SFLOAT;
            // 128-Bit formats
        case PixelFormat_RGBA32Uint:                 return VK_FORMAT_R32G32B32A32_UINT;
        case PixelFormat_RGBA32Sint:                 return VK_FORMAT_R32G32B32A32_SINT;
        case PixelFormat_RGBA32Float:                return VK_FORMAT_R32G32B32A32_SFLOAT;
            // Depth-stencil formats
        //case PixelFormat_Stencil8:                 return VK_FORMAT_S8_UINT;
        case PixelFormat_Depth16Unorm:             return VK_FORMAT_D16_UNORM;
        case PixelFormat_Depth24UnormStencil8:     return VK_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat_Depth32Float:             return VK_FORMAT_D32_SFLOAT;
        case PixelFormat_Depth32FloatStencil8:     return VK_FORMAT_D32_SFLOAT_S8_UINT;

            // Compressed BC formats
        case PixelFormat_BC1RGBAUnorm:                 return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case PixelFormat_BC1RGBAUnormSrgb:             return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case PixelFormat_BC2RGBAUnorm:                 return VK_FORMAT_BC2_UNORM_BLOCK;
        case PixelFormat_BC2RGBAUnormSrgb:             return VK_FORMAT_BC2_SRGB_BLOCK;
        case PixelFormat_BC3RGBAUnorm:                 return VK_FORMAT_BC3_UNORM_BLOCK;
        case PixelFormat_BC3RGBAUnormSrgb:             return VK_FORMAT_BC3_SRGB_BLOCK;
        case PixelFormat_BC4RUnorm:                    return VK_FORMAT_BC4_UNORM_BLOCK;
        case PixelFormat_BC4RSnorm:                    return VK_FORMAT_BC4_SNORM_BLOCK;
        case PixelFormat_BC5RGUnorm:                   return VK_FORMAT_BC5_UNORM_BLOCK;
        case PixelFormat_BC5RGSnorm:                   return VK_FORMAT_BC5_SNORM_BLOCK;
        case PixelFormat_BC6HRGBUfloat:                return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case PixelFormat_BC6HRGBFloat:                 return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case PixelFormat_BC7RGBAUnorm:                 return VK_FORMAT_BC7_UNORM_BLOCK;
        case PixelFormat_BC7RGBAUnormSrgb:             return VK_FORMAT_BC7_SRGB_BLOCK;
            // EAC/ETC compressed formats
        case PixelFormat_ETC2RGB8Unorm:              return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case PixelFormat_ETC2RGB8UnormSrgb:          return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case PixelFormat_ETC2RGB8A1Unorm:            return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case PixelFormat_ETC2RGB8A1UnormSrgb:        return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case PixelFormat_ETC2RGBA8Unorm:             return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case PixelFormat_ETC2RGBA8UnormSrgb:         return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case PixelFormat_EACR11Unorm:                return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case PixelFormat_EACR11Snorm:                return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case PixelFormat_EACRG11Unorm:               return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case PixelFormat_EACRG11Snorm:               return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
            // ASTC compressed formats
        case PixelFormat_ASTC4x4Unorm:               return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case PixelFormat_ASTC4x4UnormSrgb:           return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case PixelFormat_ASTC5x4Unorm:               return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case PixelFormat_ASTC5x4UnormSrgb:           return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case PixelFormat_ASTC5x5Unorm:               return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case PixelFormat_ASTC5x5UnormSrgb:           return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case PixelFormat_ASTC6x5Unorm:               return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case PixelFormat_ASTC6x5UnormSrgb:           return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case PixelFormat_ASTC6x6Unorm:               return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case PixelFormat_ASTC6x6UnormSrgb:           return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case PixelFormat_ASTC8x5Unorm:               return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case PixelFormat_ASTC8x5UnormSrgb:           return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case PixelFormat_ASTC8x6Unorm:               return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case PixelFormat_ASTC8x6UnormSrgb:           return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case PixelFormat_ASTC8x8Unorm:               return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case PixelFormat_ASTC8x8UnormSrgb:           return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case PixelFormat_ASTC10x5Unorm:              return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case PixelFormat_ASTC10x5UnormSrgb:          return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case PixelFormat_ASTC10x6Unorm:              return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case PixelFormat_ASTC10x6UnormSrgb:          return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case PixelFormat_ASTC10x8Unorm:              return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case PixelFormat_ASTC10x8UnormSrgb:          return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case PixelFormat_ASTC10x10Unorm:             return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case PixelFormat_ASTC10x10UnormSrgb:         return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case PixelFormat_ASTC12x10Unorm:             return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case PixelFormat_ASTC12x10UnormSrgb:         return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case PixelFormat_ASTC12x12Unorm:             return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case PixelFormat_ASTC12x12UnormSrgb:         return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
            // ASTC HDR compressed formats
        case PixelFormat_ASTC4x4HDR:           return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
        case PixelFormat_ASTC5x4HDR:           return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK;
        case PixelFormat_ASTC5x5HDR:           return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK;
        case PixelFormat_ASTC6x5HDR:           return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK;
        case PixelFormat_ASTC6x6HDR:           return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK;
        case PixelFormat_ASTC8x5HDR:           return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK;
        case PixelFormat_ASTC8x6HDR:           return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK;
        case PixelFormat_ASTC8x8HDR:           return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK;
        case PixelFormat_ASTC10x5HDR:          return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK;
        case PixelFormat_ASTC10x6HDR:          return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK;
        case PixelFormat_ASTC10x8HDR:          return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;
        case PixelFormat_ASTC10x10HDR:         return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK;
        case PixelFormat_ASTC12x10HDR:         return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK;
        case PixelFormat_ASTC12x12HDR:         return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK;

            //case PixelFormat::R8BG8Biplanar420Unorm:      return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

        default:
            return VK_FORMAT_UNDEFINED;
    }
}

PixelFormat alimerPixelFormatFromVkFormat(uint32_t vkFormat)
{
    switch (vkFormat)
    {
        // 8-bit formats
        case VK_FORMAT_R8_UNORM:                        return PixelFormat_R8Unorm;
        case VK_FORMAT_R8_SNORM:                        return PixelFormat_R8Snorm;
        case VK_FORMAT_R8_UINT:                         return PixelFormat_R8Uint;
        case VK_FORMAT_R8_SINT:                         return PixelFormat_R8Sint;
            // 16-bit formats
        case VK_FORMAT_R16_UNORM:                       return PixelFormat_R16Unorm;
        case VK_FORMAT_R16_SNORM:                       return PixelFormat_R16Snorm;
        case VK_FORMAT_R16_UINT:                        return PixelFormat_R16Uint;
        case VK_FORMAT_R16_SINT:                        return PixelFormat_R16Sint;
        case VK_FORMAT_R16_SFLOAT:                      return PixelFormat_R16Float;
        case VK_FORMAT_R8G8_UNORM:                      return PixelFormat_RG8Unorm;
        case VK_FORMAT_R8G8_SNORM:                      return PixelFormat_RG8Snorm;
        case VK_FORMAT_R8G8_UINT:                       return PixelFormat_RG8Uint;
        case VK_FORMAT_R8G8_SINT:                       return PixelFormat_RG8Sint;
            // Packed 16-Bit Pixel Formats
        case VK_FORMAT_B5G6R5_UNORM_PACK16:             return PixelFormat_B5G6R5Unorm;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:           return PixelFormat_BGR5A1Unorm;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:           return PixelFormat_BGRA4Unorm;
            // 32-bit formats
        case VK_FORMAT_R32_UINT:                        return PixelFormat_R32Uint;
        case VK_FORMAT_R32_SINT:                        return PixelFormat_R32Sint;
        case VK_FORMAT_R32_SFLOAT:                      return PixelFormat_R32Float;
        case VK_FORMAT_R16G16_UNORM:                    return PixelFormat_RG16Unorm;
        case VK_FORMAT_R16G16_SNORM:                    return PixelFormat_RG16Snorm;
        case VK_FORMAT_R16G16_UINT:                     return PixelFormat_RG16Uint;
        case VK_FORMAT_R16G16_SINT:                     return PixelFormat_RG16Sint;
        case VK_FORMAT_R16G16_SFLOAT:                   return PixelFormat_RG16Float;
        case VK_FORMAT_R8G8B8A8_UNORM:                  return PixelFormat_RGBA8Unorm;
        case VK_FORMAT_R8G8B8A8_SRGB:                   return PixelFormat_RGBA8UnormSrgb;
        case VK_FORMAT_R8G8B8A8_SNORM:                  return PixelFormat_RGBA8Snorm;
        case VK_FORMAT_R8G8B8A8_UINT:                   return PixelFormat_RGBA8Uint;
        case VK_FORMAT_R8G8B8A8_SINT:                   return PixelFormat_RGBA8Sint;
        case VK_FORMAT_B8G8R8A8_UNORM:                  return PixelFormat_BGRA8Unorm;
        case VK_FORMAT_B8G8R8A8_SRGB:                   return PixelFormat_BGRA8UnormSrgb;
            // Packed 32-Bit formats
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:        return PixelFormat_RGB10A2Unorm;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:         return PixelFormat_RGB10A2Uint;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:         return PixelFormat_RG11B10Ufloat;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:          return PixelFormat_RGB9E5Ufloat;
            // 64-Bit formats
        case VK_FORMAT_R32G32_UINT:                     return PixelFormat_RG32Uint;
        case VK_FORMAT_R32G32_SINT:                     return PixelFormat_RG32Sint;
        case VK_FORMAT_R32G32_SFLOAT:                   return PixelFormat_RG32Float;
        case VK_FORMAT_R16G16B16A16_UNORM:              return PixelFormat_RGBA16Unorm;
        case VK_FORMAT_R16G16B16A16_SNORM:              return PixelFormat_RGBA16Snorm;
        case VK_FORMAT_R16G16B16A16_UINT:               return PixelFormat_RGBA16Uint;
        case VK_FORMAT_R16G16B16A16_SINT:               return PixelFormat_RGBA16Sint;
        case VK_FORMAT_R16G16B16A16_SFLOAT:             return PixelFormat_RGBA16Float;
            // 128-Bit formats
        case VK_FORMAT_R32G32B32A32_UINT:               return PixelFormat_RGBA32Uint;
        case VK_FORMAT_R32G32B32A32_SINT:               return PixelFormat_RGBA32Sint;
        case VK_FORMAT_R32G32B32A32_SFLOAT:             return PixelFormat_RGBA32Float;
            // Depth-stencil formats
        case VK_FORMAT_D16_UNORM:			            return PixelFormat_Depth16Unorm;
        case VK_FORMAT_D32_SFLOAT:			            return PixelFormat_Depth32Float;
        case VK_FORMAT_D24_UNORM_S8_UINT:               return PixelFormat_Depth24UnormStencil8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:              return PixelFormat_Depth32FloatStencil8;
            // Compressed BC formats
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:            return PixelFormat_BC1RGBAUnorm;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:             return PixelFormat_BC1RGBAUnormSrgb;
        case VK_FORMAT_BC2_UNORM_BLOCK:                 return PixelFormat_BC2RGBAUnorm;
        case VK_FORMAT_BC2_SRGB_BLOCK:                  return PixelFormat_BC2RGBAUnormSrgb;
        case VK_FORMAT_BC3_UNORM_BLOCK:                 return PixelFormat_BC3RGBAUnorm;
        case VK_FORMAT_BC3_SRGB_BLOCK:                  return PixelFormat_BC3RGBAUnormSrgb;
        case VK_FORMAT_BC4_UNORM_BLOCK:                 return PixelFormat_BC4RSnorm;
        case VK_FORMAT_BC4_SNORM_BLOCK:                 return PixelFormat_BC4RUnorm;
        case VK_FORMAT_BC5_UNORM_BLOCK:                 return PixelFormat_BC5RGUnorm;
        case VK_FORMAT_BC5_SNORM_BLOCK:                 return PixelFormat_BC5RGSnorm;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:               return PixelFormat_BC6HRGBUfloat;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:               return PixelFormat_BC6HRGBFloat;
        case VK_FORMAT_BC7_UNORM_BLOCK:                 return PixelFormat_BC7RGBAUnorm;
        case VK_FORMAT_BC7_SRGB_BLOCK:                  return PixelFormat_BC7RGBAUnormSrgb;
            // EAC/ETC compressed formats
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:         return PixelFormat_ETC2RGB8Unorm;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:          return PixelFormat_ETC2RGB8UnormSrgb;
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:       return PixelFormat_ETC2RGB8A1Unorm;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:        return PixelFormat_ETC2RGB8A1UnormSrgb;
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:       return PixelFormat_ETC2RGBA8Unorm;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:        return PixelFormat_ETC2RGBA8UnormSrgb;
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:             return PixelFormat_EACR11Unorm;
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:             return PixelFormat_EACR11Snorm;
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:          return PixelFormat_EACRG11Unorm;
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:          return PixelFormat_EACRG11Snorm;
            // ASTC compressed formats
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:            return PixelFormat_ASTC4x4Unorm;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:             return PixelFormat_ASTC4x4UnormSrgb;
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:            return PixelFormat_ASTC5x4Unorm;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:             return PixelFormat_ASTC5x4UnormSrgb;
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:            return PixelFormat_ASTC5x5Unorm;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:             return PixelFormat_ASTC5x5UnormSrgb;
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:            return PixelFormat_ASTC6x5Unorm;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:             return PixelFormat_ASTC6x5UnormSrgb;
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:            return PixelFormat_ASTC6x6Unorm;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:             return PixelFormat_ASTC6x6UnormSrgb;
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:            return PixelFormat_ASTC8x5Unorm;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:             return PixelFormat_ASTC8x5UnormSrgb;
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:            return PixelFormat_ASTC8x6Unorm;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:             return PixelFormat_ASTC8x6UnormSrgb;
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:            return PixelFormat_ASTC8x8Unorm;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:             return PixelFormat_ASTC8x8UnormSrgb;
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:           return PixelFormat_ASTC10x5Unorm;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:            return PixelFormat_ASTC10x5UnormSrgb;
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:           return PixelFormat_ASTC10x6Unorm;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:            return PixelFormat_ASTC10x6UnormSrgb;
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:           return PixelFormat_ASTC10x8Unorm;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:            return PixelFormat_ASTC10x8UnormSrgb;
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:          return PixelFormat_ASTC10x10Unorm;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:           return PixelFormat_ASTC10x10UnormSrgb;
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:          return PixelFormat_ASTC12x10Unorm;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:           return PixelFormat_ASTC12x10UnormSrgb;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:          return PixelFormat_ASTC12x12Unorm;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:           return PixelFormat_ASTC12x12UnormSrgb;
            // ASTC HDR compressed formats
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:           return PixelFormat_ASTC4x4HDR;
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:           return PixelFormat_ASTC5x4HDR;
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:           return PixelFormat_ASTC5x5HDR;
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:           return PixelFormat_ASTC6x5HDR;
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:           return PixelFormat_ASTC6x6HDR;
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:           return PixelFormat_ASTC8x5HDR;
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:           return PixelFormat_ASTC8x6HDR;
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:           return PixelFormat_ASTC8x8HDR;
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:          return PixelFormat_ASTC10x5HDR;
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:          return PixelFormat_ASTC10x6HDR;
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:          return PixelFormat_ASTC10x8HDR;
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:         return PixelFormat_ASTC10x10HDR;
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:         return PixelFormat_ASTC12x10HDR;
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:         return PixelFormat_ASTC12x12HDR;

            //case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:      return PixelFormat::R8BG8Biplanar420Unorm;

        default:
            return PixelFormat_Undefined;
    }
}

#if defined(_WIN32)
/// Returns a wide string version of the specified UTF-8 string
WCHAR* Win32_CreateWideStringFromUTF8(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!count)
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string from UTF-8");
        return nullptr;
    }

    target = ALIMER_ALLOCN(WCHAR, count);

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string from UTF-8");
        alimerFree(target);
        return nullptr;
    }

    return target;
}

/// Returns a UTF-8 string version of the specified wide string
char* Win32_CreateUTF8FromWideString(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!size)
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string to UTF-8");
        return nullptr;
    }

    target = ALIMER_ALLOCN(char, size);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string to UTF-8");
        alimerFree(target);
        return nullptr;
    }

    return target;
}

#endif
