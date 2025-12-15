// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/PixelFormat.h"

#if defined(_WIN32)
#   if defined(ALIMER_RHI_D3D12)
#       include <directx/dxgiformat.h>
#   else
#       include <dxgiformat.h>
#   endif
#endif

#if defined(ALIMER_RHI_VULKAN)
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
    VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
    VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
    VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
    VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
    VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
    VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
    VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
    VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
    VK_FORMAT_R16G16_S10_5_NV = 1000464000,
    VK_FORMAT_MAX_ENUM = 0x7FFFFFFF
} VkFormat;
#endif

#if !defined(DXGI_FORMAT_DEFINED)
typedef enum DXGI_FORMAT
{
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


    DXGI_FORMAT_FORCE_UINT = 0xffffffff
} DXGI_FORMAT;
#endif

namespace Alimer
{
    // Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
    const PixelFormatInfo kFormatDesc[] = {
        //        format                   name             bytes blk         kind               
        { PixelFormat::Undefined,           "Undefined",          0,   0, 0, PixelFormatKind::Uint },
        { PixelFormat::R8Unorm,             "R8Unorm",           1,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::R8Snorm,             "R8Snorm",           1,   1, 1, PixelFormatKind::Snorm },
        { PixelFormat::R8Uint,              "R8Uint",            1,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::R8Sint,              "R8Sint",            1,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::R16Unorm,            "R16Unorm",          2,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::R16Snorm,            "R16Snorm",          2,   1, 1, PixelFormatKind::Snorm },
        { PixelFormat::R16Uint,             "R16Uint",           2,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::R16Sint,             "R16Sint",           2,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::R16Float,            "R16Float",          2,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RG8Unorm,            "RG8Unorm",          2,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::RG8Snorm,            "RG8Snorm",          2,   1, 1, PixelFormatKind::Snorm },
        { PixelFormat::RG8Uint,             "RG8Uint",           2,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RG8Sint,             "RG8Sint",           2,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::B5G6R5Unorm,         "B5G6R5Unorm",       2,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::BGR5A1Unorm,         "BGR5A1Unorm",      2,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::BGRA4Unorm,          "BGRA4Unorm",        2,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::R32Uint,             "R32Uint",           4,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::R32Sint,             "R32Sint",           4,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::R32Float,            "R32Float",          4,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RG16Unorm,           "RG16Unorm",         4,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::RG16Snorm,           "RG16Snorm",         4,   1, 1, PixelFormatKind::Snorm },
        { PixelFormat::RG16Uint,            "RG16Uint",          4,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RG16Sint,            "RG16Sint",          4,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::RG16Float,           "RG16Float",         4,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RGBA8Unorm,          "RGBA8Unorm",        4,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::RGBA8UnormSrgb,      "RGBA8Unorm",    4,   1, 1, PixelFormatKind::UnormSrgb },
        { PixelFormat::RGBA8Snorm,          "RGRA8Snorm",        4,   1, 1, PixelFormatKind::Snorm },
        { PixelFormat::RGBA8Uint,           "RGRA8Uint",         4,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RGBA8Sint,           "RGRA8Sint",         4,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::BGRA8Unorm,          "BGRA8Unorm",        4,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::BGRA8UnormSrgb,      "BGRA8UnormSrgb",    4,   1, 1, PixelFormatKind::UnormSrgb},
        { PixelFormat::RGB10A2Unorm,        "RGB10A2Unorm",      4,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::RGB10A2Uint,         "RGB10A2Uint",       4,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RG11B10Float,        "RG11B10Float",      4,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RGB9E5Float,         "RGB9E5Float",      4,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RG32Uint,            "RG32Uint",          8,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RG32Sint,            "RG32Sint",          8,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::RG32Float,           "RG32Float",         8,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RGBA16Unorm,         "RGBA16Unorm",       8,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::RGBA16Snorm,         "RGBA16Snorm",       8,   1, 1, PixelFormatKind::Snorm },
        { PixelFormat::RGBA16Uint,          "RGBA16Uint",        8,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RGBA16Sint,          "RGBA16Sint",        8,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::RGBA16Float,         "RGBA16Float",       8,   1, 1, PixelFormatKind::Float },
        { PixelFormat::RGBA32Uint,          "RGBA32Uint",       16,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::RGBA32Sint,          "RGBA32Sint",       16,   1, 1, PixelFormatKind::Sint },
        { PixelFormat::RGBA32Float,         "RGBA32Float",      16,   1, 1, PixelFormatKind::Float },

        // Depth-stencil formats
        { PixelFormat::Stencil8,                "Stencil8",             4,   1, 1, PixelFormatKind::Uint },
        { PixelFormat::Depth16Unorm,            "Depth16Unorm",         2,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::Depth24UnormStencil8,    "Depth24UnormStencil8", 4,   1, 1, PixelFormatKind::Unorm },
        { PixelFormat::Depth32Float,            "Depth32Float",         4,   1, 1, PixelFormatKind::Float },
        { PixelFormat::Depth32FloatStencil8,    "Depth32FloatStencil8", 8,   1, 1, PixelFormatKind::Float },

        // BC compressed formats
        { PixelFormat::BC1RGBAUnorm,            "BC1RGBAUnorm",         8,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::BC1RGBAUnormSrgb,        "BC1RGBAUnormSrgb",     8,   4, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::BC2RGBAUnorm,            "BC2RGBAUnorm",         16,  4, 4, PixelFormatKind::Unorm },
        { PixelFormat::BC2RGBAUnormSrgb,        "BC2RGBAUnormSrgb",     16,  4, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::BC3RGBAUnorm,            "BC3RGBAUnorm",         16,  4, 4, PixelFormatKind::Unorm },
        { PixelFormat::BC3RGBAUnormSrgb,        "BC3RGBAUnormSrgb",     16,  4, 4, PixelFormatKind::UnormSrgb  },
        { PixelFormat::BC4RUnorm,               "BC4RUnorm",            8,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::BC4RSnorm,               "BC4RSnorm",            8,   4, 4, PixelFormatKind::Snorm },
        { PixelFormat::BC5RGUnorm,              "BC5RGUnorm",           16,  4, 4, PixelFormatKind::Unorm },
        { PixelFormat::BC5RGSnorm,              "BC5RGSnorm",           16,  4, 4, PixelFormatKind::Snorm },
        { PixelFormat::BC6HRGBUfloat,           "BC6HRGBUfloat",        16,  4, 4, PixelFormatKind::Float },
        { PixelFormat::BC6HRGBFloat,            "BC6HRGBFloat",        16,  4, 4, PixelFormatKind::Float },
        { PixelFormat::BC7RGBAUnorm,            "BC7RGBAUnorm",         16,  4, 4, PixelFormatKind::Unorm },
        { PixelFormat::BC7RGBAUnormSrgb,        "BC7RGBAUnormSrgb",     16,  4, 4, PixelFormatKind::UnormSrgb },

        // ETC2/EAC compressed formats
        { PixelFormat::ETC2RGB8Unorm,           "ETC2RGB8Unorm",         8,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::ETC2RGB8UnormSrgb,       "ETC2RGB8UnormSrgb",     8,   4, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::ETC2RGB8A1Unorm,         "ETC2RGB8A1Unorm,",     16,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::ETC2RGB8A1UnormSrgb,     "ETC2RGB8A1UnormSrgb",  16,   4, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::ETC2RGBA8Unorm,          "ETC2RGBA8Unorm",       16,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::ETC2RGBA8UnormSrgb,      "ETC2RGBA8UnormSrgb",   16,   4, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::EACR11Unorm,             "EACR11Unorm",          8,    4, 4, PixelFormatKind::Unorm },
        { PixelFormat::EACR11Snorm,             "EACR11Snorm",          8,    4, 4, PixelFormatKind::Snorm },
        { PixelFormat::EACRG11Unorm,            "EACRG11Unorm",         16,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::EACRG11Snorm,            "EACRG11Snorm",         16,   4, 4, PixelFormatKind::Snorm },

        // ASTC compressed formats
        { PixelFormat::ASTC4x4Unorm,            "ASTC4x4Unorm",         16,   4, 4, PixelFormatKind::Unorm },
        { PixelFormat::ASTC4x4UnormSrgb,        "ASTC4x4UnormSrgb",     16,   4, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC5x4Unorm,            "ASTC5x4Unorm",         16,   5, 4, PixelFormatKind::Unorm },
        { PixelFormat::ASTC5x4UnormSrgb,        "ASTC5x4UnormSrgb",     16,   5, 4, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC5x5Unorm,            "ASTC5x5UnormSrgb",     16,   5, 5, PixelFormatKind::Unorm },
        { PixelFormat::ASTC5x5UnormSrgb,        "ASTC5x5UnormSrgb",     16,   5, 5, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC6x5Unorm,            "ASTC6x5Unorm",         16,   6, 5, PixelFormatKind::Unorm },
        { PixelFormat::ASTC6x5UnormSrgb,        "ASTC6x5UnormSrgb",     16,   6, 5, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC6x6Unorm,            "ASTC6x6Unorm",         16,   6, 6, PixelFormatKind::Unorm },
        { PixelFormat::ASTC6x6UnormSrgb,        "ASTC6x6UnormSrgb",     16,   6, 6, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC8x5Unorm,            "ASTC8x5Unorm",         16,   8, 5, PixelFormatKind::Unorm },
        { PixelFormat::ASTC8x5UnormSrgb,        "ASTC8x5UnormSrgb",     16,   8, 5, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC8x6Unorm,            "ASTC8x6Unorm",         16,   8, 6, PixelFormatKind::Unorm },
        { PixelFormat::ASTC8x6UnormSrgb,        "ASTC8x6UnormSrgb",     16,   8, 6, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC8x8Unorm,            "ASTC8x8Unorm",         16,   8, 8, PixelFormatKind::Unorm },
        { PixelFormat::ASTC8x8UnormSrgb,        "ASTC8x8UnormSrgb",     16,   8, 8, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC10x5Unorm,           "ASTC10x5Unorm",        16,   10, 5, PixelFormatKind::Unorm },
        { PixelFormat::ASTC10x5UnormSrgb,       "ASTC10x5UnormSrgb",    16,   10, 5, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC10x6Unorm,           "ASTC10x6Unorm",        16,   10, 6, PixelFormatKind::Unorm },
        { PixelFormat::ASTC10x6UnormSrgb,       "ASTC10x6UnormSrgb",    16,   10, 6, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC10x8Unorm,           "ASTC10x8Unorm",        16,   10, 8, PixelFormatKind::Unorm },
        { PixelFormat::ASTC10x8UnormSrgb,       "ASTC10x8UnormSrgb",    16,   10, 8, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC10x10Unorm,          "ASTC10x10Unorm",       16,   10, 10, PixelFormatKind::Unorm  },
        { PixelFormat::ASTC10x10UnormSrgb,      "ASTC10x10UnormSrgb",   16,   10, 10, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC12x10Unorm,          "ASTC12x10Unorm",       16,   12, 10, PixelFormatKind::Unorm  },
        { PixelFormat::ASTC12x10UnormSrgb,      "ASTC12x10UnormSrgb",   16,   12, 10, PixelFormatKind::UnormSrgb },
        { PixelFormat::ASTC12x12Unorm,          "ASTC12x12Unorm",       16,   12, 12, PixelFormatKind::Unorm  },
        { PixelFormat::ASTC12x12UnormSrgb,      "ASTC12x12UnormSrgb",   16,   12, 12, PixelFormatKind::UnormSrgb },
        // ASTC HDR compressed formats
        { PixelFormat::ASTC4x4HDR,              "ASTC4x4HDR",         16,   4, 4, PixelFormatKind::Float },
        { PixelFormat::ASTC5x4HDR,              "ASTC5x4HDR",         16,   5, 4, PixelFormatKind::Float },
        { PixelFormat::ASTC5x5HDR,              "ASTC5x5HDR",         16,   5, 5, PixelFormatKind::Float },
        { PixelFormat::ASTC6x5HDR,              "ASTC6x5HDR",         16,   6, 5, PixelFormatKind::Float },
        { PixelFormat::ASTC6x6HDR,              "ASTC6x6HDR",         16,   6, 6, PixelFormatKind::Float },
        { PixelFormat::ASTC8x5HDR,              "ASTC8x5HDR",         16,   8, 5, PixelFormatKind::Float },
        { PixelFormat::ASTC8x6HDR,              "ASTC8x6HDR",         16,   8, 6, PixelFormatKind::Float },
        { PixelFormat::ASTC8x8HDR,              "ASTC8x8HDR",         16,   8, 8, PixelFormatKind::Float },
        { PixelFormat::ASTC10x5HDR,             "ASTC10x5HDR",        16,   10, 5, PixelFormatKind::Float },
        { PixelFormat::ASTC10x6HDR,             "ASTC10x6HDR",        16,   10, 6, PixelFormatKind::Float },
        { PixelFormat::ASTC10x8HDR,             "ASTC10x8HDR",        16,   10, 8, PixelFormatKind::Float },
        { PixelFormat::ASTC10x10HDR,            "ASTC10x10HDR",       16,   10, 10, PixelFormatKind::Float },
        { PixelFormat::ASTC12x10HDR,            "ASTC12x10HDR",       16,   12, 10, PixelFormatKind::Float },
        { PixelFormat::ASTC12x12HDR,            "ASTC12x12HDR",       16,   12, 12, PixelFormatKind::Float },
    };

    static_assert(
        sizeof(kFormatDesc) / sizeof(PixelFormatInfo) == size_t(PixelFormat::Count),
        "The format info table doesn't have the right number of elements"
        );

    const PixelFormatInfo& GetPixelFormatInfo(PixelFormat format)
    {
        if (uint32_t(format) >= uint32_t(PixelFormat::Count))
            return kFormatDesc[0]; // Unknown

        const PixelFormatInfo& info = kFormatDesc[uint32_t(format)];
        ALIMER_ASSERT(info.format == format);
        return info;
    }

    uint32_t GetFormatBytesPerBlock(PixelFormat format)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
        return formatInfo.bytesPerBlock;
    }

    bool IsDepthFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::Depth16Unorm:
            case PixelFormat::Depth24UnormStencil8:
            case PixelFormat::Depth32Float:
            case PixelFormat::Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    bool IsStencilFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::Stencil8:
            case PixelFormat::Depth24UnormStencil8:
            case PixelFormat::Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    bool IsDepthStencilFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::Stencil8:
            case PixelFormat::Depth16Unorm:
            case PixelFormat::Depth24UnormStencil8:
            case PixelFormat::Depth32Float:
            case PixelFormat::Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    bool IsDepthOnlyFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::Depth16Unorm:
            case PixelFormat::Depth32Float:
                return true;
            default:
                return false;
        }
    }

    bool IsStencilOnlyFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::Stencil8:
                return true;
            default:
                return false;
        }
    }

    bool IsCompressedFormat(PixelFormat format)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
        return formatInfo.blockWidth > 1 || formatInfo.blockHeight > 1;
    }

    bool IsCompressedBCFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::BC1RGBAUnorm:
            case PixelFormat::BC1RGBAUnormSrgb:
            case PixelFormat::BC2RGBAUnorm:
            case PixelFormat::BC2RGBAUnormSrgb:
            case PixelFormat::BC3RGBAUnorm:
            case PixelFormat::BC3RGBAUnormSrgb:
            case PixelFormat::BC4RUnorm:
            case PixelFormat::BC4RSnorm:
            case PixelFormat::BC5RGUnorm:
            case PixelFormat::BC5RGSnorm:
            case PixelFormat::BC6HRGBUfloat:
            case PixelFormat::BC6HRGBFloat:
            case PixelFormat::BC7RGBAUnorm:
            case PixelFormat::BC7RGBAUnormSrgb:
                return true;
            default:
                return false;
        }
    }

    bool IsCompressedASTCFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::ASTC4x4Unorm:
            case PixelFormat::ASTC4x4UnormSrgb:
            case PixelFormat::ASTC5x4Unorm:
            case PixelFormat::ASTC5x4UnormSrgb:
            case PixelFormat::ASTC5x5Unorm:
            case PixelFormat::ASTC5x5UnormSrgb:
            case PixelFormat::ASTC6x5Unorm:
            case PixelFormat::ASTC6x5UnormSrgb:
            case PixelFormat::ASTC6x6Unorm:
            case PixelFormat::ASTC6x6UnormSrgb:
            case PixelFormat::ASTC8x5Unorm:
            case PixelFormat::ASTC8x5UnormSrgb:
            case PixelFormat::ASTC8x6Unorm:
            case PixelFormat::ASTC8x6UnormSrgb:
            case PixelFormat::ASTC8x8Unorm:
            case PixelFormat::ASTC8x8UnormSrgb:
            case PixelFormat::ASTC10x5Unorm:
            case PixelFormat::ASTC10x5UnormSrgb:
            case PixelFormat::ASTC10x6Unorm:
            case PixelFormat::ASTC10x6UnormSrgb:
            case PixelFormat::ASTC10x8Unorm:
            case PixelFormat::ASTC10x8UnormSrgb:
            case PixelFormat::ASTC10x10Unorm:
            case PixelFormat::ASTC10x10UnormSrgb:
            case PixelFormat::ASTC12x10Unorm:
            case PixelFormat::ASTC12x10UnormSrgb:
            case PixelFormat::ASTC12x12Unorm:
                return true;

            case PixelFormat::ASTC4x4HDR:
            case PixelFormat::ASTC5x4HDR:
            case PixelFormat::ASTC5x5HDR:
            case PixelFormat::ASTC6x5HDR:
            case PixelFormat::ASTC6x6HDR:
            case PixelFormat::ASTC8x5HDR:
            case PixelFormat::ASTC8x6HDR:
            case PixelFormat::ASTC8x8HDR:
            case PixelFormat::ASTC10x5HDR:
            case PixelFormat::ASTC10x6HDR:
            case PixelFormat::ASTC10x8HDR:
            case PixelFormat::ASTC10x10HDR:
            case PixelFormat::ASTC12x10HDR:
            case PixelFormat::ASTC12x12HDR:
                return true;

            default:
                return false;
        }
    }

    PixelFormatKind GetPixelFormatKind(PixelFormat format)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
        return formatInfo.kind;
    }

    bool IsIntegerFormat(PixelFormat format)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
        return formatInfo.kind == PixelFormatKind::Uint || formatInfo.kind == PixelFormatKind::Sint;
    }

    bool IsSrgbFormat(PixelFormat format)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
        return formatInfo.kind == PixelFormatKind::UnormSrgb;
    }

    PixelFormat SrgbToLinearFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::RGBA8UnormSrgb:       return PixelFormat::RGBA8Unorm;
            case PixelFormat::BGRA8UnormSrgb:       return PixelFormat::BGRA8Unorm;
                // Bc compressed formats
            case PixelFormat::BC1RGBAUnormSrgb:     return PixelFormat::BC1RGBAUnorm;
            case PixelFormat::BC2RGBAUnormSrgb:     return PixelFormat::BC2RGBAUnorm;
            case PixelFormat::BC3RGBAUnormSrgb:     return PixelFormat::BC3RGBAUnorm;
            case PixelFormat::BC7RGBAUnormSrgb:     return PixelFormat::BC7RGBAUnorm;

                // Etc2/Eac compressed formats
            case PixelFormat::ETC2RGB8UnormSrgb:    return PixelFormat::ETC2RGB8Unorm;
            case PixelFormat::ETC2RGB8A1UnormSrgb:  return PixelFormat::ETC2RGB8A1Unorm;
            case PixelFormat::ETC2RGBA8UnormSrgb:   return PixelFormat::ETC2RGBA8Unorm;

                // Astc compressed formats
            case PixelFormat::ASTC4x4UnormSrgb:     return PixelFormat::ASTC4x4Unorm;
            case PixelFormat::ASTC5x4UnormSrgb:     return PixelFormat::ASTC5x4Unorm;
            case PixelFormat::ASTC5x5UnormSrgb:     return PixelFormat::ASTC5x5Unorm;
            case PixelFormat::ASTC6x5UnormSrgb:     return PixelFormat::ASTC6x5Unorm;
            case PixelFormat::ASTC6x6UnormSrgb:     return PixelFormat::ASTC6x6Unorm;
            case PixelFormat::ASTC8x5UnormSrgb:     return PixelFormat::ASTC8x5Unorm;
            case PixelFormat::ASTC8x6UnormSrgb:     return PixelFormat::ASTC8x6Unorm;
            case PixelFormat::ASTC8x8UnormSrgb:     return PixelFormat::ASTC8x8Unorm;
            case PixelFormat::ASTC10x5UnormSrgb:    return PixelFormat::ASTC10x5Unorm;
            case PixelFormat::ASTC10x6UnormSrgb:    return PixelFormat::ASTC10x6Unorm;
            case PixelFormat::ASTC10x8UnormSrgb:    return PixelFormat::ASTC10x8Unorm;
            case PixelFormat::ASTC10x10UnormSrgb:   return PixelFormat::ASTC10x10Unorm;
            case PixelFormat::ASTC12x10UnormSrgb:   return PixelFormat::ASTC12x10Unorm;
            case PixelFormat::ASTC12x12UnormSrgb:   return PixelFormat::ASTC12x12Unorm;

            default:
                ALIMER_ASSERT(IsSrgbFormat(format) == false);
                return format;
        }
    }

    PixelFormat LinearToSrgbFormat(PixelFormat format)
    {
        switch (format)
        {
            case PixelFormat::RGBA8Unorm:       return PixelFormat::RGBA8UnormSrgb;
            case PixelFormat::BGRA8Unorm:       return PixelFormat::BGRA8UnormSrgb;

                // Bc compressed formats
            case PixelFormat::BC1RGBAUnorm:     return PixelFormat::BC1RGBAUnormSrgb;
            case PixelFormat::BC2RGBAUnorm:     return PixelFormat::BC2RGBAUnormSrgb;
            case PixelFormat::BC3RGBAUnorm:     return PixelFormat::BC3RGBAUnormSrgb;
            case PixelFormat::BC7RGBAUnorm:     return PixelFormat::BC7RGBAUnormSrgb;

                // Etc2/Eac compressed formats
            case PixelFormat::ETC2RGB8Unorm:    return PixelFormat::ETC2RGB8UnormSrgb;
            case PixelFormat::ETC2RGB8A1Unorm:  return PixelFormat::ETC2RGB8A1UnormSrgb;
            case PixelFormat::ETC2RGBA8Unorm:   return PixelFormat::ETC2RGBA8UnormSrgb;

                // Astc compressed formats
            case PixelFormat::ASTC4x4Unorm:     return PixelFormat::ASTC4x4UnormSrgb;
            case PixelFormat::ASTC5x4Unorm:     return PixelFormat::ASTC5x4UnormSrgb;
            case PixelFormat::ASTC5x5Unorm:     return PixelFormat::ASTC5x5UnormSrgb;
            case PixelFormat::ASTC6x5Unorm:     return PixelFormat::ASTC6x5UnormSrgb;
            case PixelFormat::ASTC6x6Unorm:     return PixelFormat::ASTC6x6UnormSrgb;
            case PixelFormat::ASTC8x5Unorm:     return PixelFormat::ASTC8x5UnormSrgb;
            case PixelFormat::ASTC8x6Unorm:     return PixelFormat::ASTC8x6UnormSrgb;
            case PixelFormat::ASTC8x8Unorm:     return PixelFormat::ASTC8x8UnormSrgb;
            case PixelFormat::ASTC10x5Unorm:    return PixelFormat::ASTC10x5UnormSrgb;
            case PixelFormat::ASTC10x6Unorm:    return PixelFormat::ASTC10x6UnormSrgb;
            case PixelFormat::ASTC10x8Unorm:    return PixelFormat::ASTC10x8UnormSrgb;
            case PixelFormat::ASTC10x10Unorm:   return PixelFormat::ASTC10x10UnormSrgb;
            case PixelFormat::ASTC12x10Unorm:   return PixelFormat::ASTC12x10UnormSrgb;
            case PixelFormat::ASTC12x12Unorm:   return PixelFormat::ASTC12x12UnormSrgb;

            default:
                return format;
        }
    }

    const char* ToString(PixelFormat format)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);
        return formatInfo.name;
    }

    uint32_t BitsPerPixel(PixelFormat format) noexcept
    {
        switch (format)
        {
            case PixelFormat::RGBA32Uint:
            case PixelFormat::RGBA32Sint:
            case PixelFormat::RGBA32Float:
                return 128;

                //case PixelFormat.Rgb32Uint:
                //case PixelFormat.Rgb32Sint:
                //case PixelFormat.Rgb32Float:
                //    return 96;

            case PixelFormat::RGBA16Unorm:
            case PixelFormat::RGBA16Snorm:
            case PixelFormat::RGBA16Uint:
            case PixelFormat::RGBA16Sint:
            case PixelFormat::RGBA16Float:
            case PixelFormat::RG32Uint:
            case PixelFormat::RG32Sint:
            case PixelFormat::RG32Float:
            case PixelFormat::Depth32FloatStencil8:
                return 64;

            case PixelFormat::RGB10A2Unorm:
            case PixelFormat::RGB10A2Uint:
            case PixelFormat::RG11B10Float:
            case PixelFormat::RGB9E5Float:
            case PixelFormat::RGBA8Unorm:
            case PixelFormat::RGBA8UnormSrgb:
            case PixelFormat::RGBA8Snorm:
            case PixelFormat::RGBA8Uint:
            case PixelFormat::RGBA8Sint:
            case PixelFormat::RG16Unorm:
            case PixelFormat::RG16Snorm:
            case PixelFormat::RG16Uint:
            case PixelFormat::RG16Sint:
            case PixelFormat::RG16Float:
            case PixelFormat::Depth32Float:
            case PixelFormat::R32Uint:
            case PixelFormat::R32Sint:
            case PixelFormat::R32Float:
            case PixelFormat::Depth24UnormStencil8:
            case PixelFormat::BGRA8Unorm:
            case PixelFormat::BGRA8UnormSrgb:
                return 32;

            case PixelFormat::RG8Unorm:
            case PixelFormat::RG8Snorm:
            case PixelFormat::RG8Uint:
            case PixelFormat::RG8Sint:
            case PixelFormat::R16Unorm:
            case PixelFormat::R16Snorm:
            case PixelFormat::R16Uint:
            case PixelFormat::R16Sint:
            case PixelFormat::R16Float:
            case PixelFormat::Depth16Unorm:
            case PixelFormat::B5G6R5Unorm:
            case PixelFormat::BGR5A1Unorm:
            case PixelFormat::BGRA4Unorm:
                return 16;

            case PixelFormat::R8Unorm:
            case PixelFormat::R8Snorm:
            case PixelFormat::R8Uint:
            case PixelFormat::R8Sint:
            case PixelFormat::BC2RGBAUnorm:
            case PixelFormat::BC2RGBAUnormSrgb:
            case PixelFormat::BC3RGBAUnorm:
            case PixelFormat::BC3RGBAUnormSrgb:
            case PixelFormat::BC5RGUnorm:
            case PixelFormat::BC5RGSnorm:
            case PixelFormat::BC6HRGBUfloat:
            case PixelFormat::BC6HRGBFloat:
            case PixelFormat::BC7RGBAUnorm:
            case PixelFormat::BC7RGBAUnormSrgb:
            case PixelFormat::ETC2RGB8A1Unorm:
            case PixelFormat::ETC2RGB8A1UnormSrgb:
            case PixelFormat::ETC2RGBA8Unorm:
            case PixelFormat::ETC2RGBA8UnormSrgb:
            case PixelFormat::EACRG11Unorm:
            case PixelFormat::EACRG11Snorm:
                return 8;

            case PixelFormat::BC1RGBAUnorm:
            case PixelFormat::BC1RGBAUnormSrgb:
            case PixelFormat::BC4RUnorm:
            case PixelFormat::BC4RSnorm:
            case PixelFormat::ETC2RGB8Unorm:
            case PixelFormat::ETC2RGB8UnormSrgb:
            case PixelFormat::EACR11Unorm:
            case PixelFormat::EACR11Snorm:
                return 4;

            default:
                return 0;
        }
    }

    void GetSurfaceInfo(PixelFormat format, uint32_t width, uint32_t height,
        _Out_opt_ uint32_t* pRowPitch,
        _Out_opt_ uint32_t* pSlicePitch,
        _Out_opt_ uint32_t* pWidthCount,
        _Out_opt_ uint32_t* pHeightCount)
    {
        const PixelFormatInfo& formatInfo = GetPixelFormatInfo(format);

        uint32_t rowPitch = 0;
        uint32_t slicePitch = 0;
        uint32_t widthCount = width;
        uint32_t heightCount = height;

        switch (format)
        {
            case PixelFormat::BC1RGBAUnorm:
            case PixelFormat::BC1RGBAUnormSrgb:
            case PixelFormat::BC2RGBAUnorm:
            case PixelFormat::BC2RGBAUnormSrgb:
            case PixelFormat::BC3RGBAUnorm:
            case PixelFormat::BC3RGBAUnormSrgb:
            case PixelFormat::BC4RUnorm:
            case PixelFormat::BC4RSnorm:
            case PixelFormat::BC5RGUnorm:
            case PixelFormat::BC5RGSnorm:
            case PixelFormat::BC6HRGBUfloat:
            case PixelFormat::BC6HRGBFloat:
            case PixelFormat::BC7RGBAUnorm:
            case PixelFormat::BC7RGBAUnormSrgb:
                widthCount = Max(1u, (width + 3) / 4);
                heightCount = Max(1u, (height + 3) / 4);
                rowPitch = widthCount * formatInfo.bytesPerBlock; // BytesPerBlock is 8 or 16
                slicePitch = rowPitch * heightCount;
                break;

                // ETC2/EAC compressed formats
            case PixelFormat::ETC2RGB8Unorm:
            case PixelFormat::ETC2RGB8UnormSrgb:
            case PixelFormat::ETC2RGB8A1Unorm:
            case PixelFormat::ETC2RGB8A1UnormSrgb:
            case PixelFormat::ETC2RGBA8Unorm:
            case PixelFormat::ETC2RGBA8UnormSrgb:
            case PixelFormat::EACR11Unorm:
            case PixelFormat::EACR11Snorm:
            case PixelFormat::EACRG11Unorm:
            case PixelFormat::EACRG11Snorm:
                widthCount = Max(1u, (width + formatInfo.blockWidth - 1) / formatInfo.blockWidth);
                heightCount = Max(1u, (height + formatInfo.blockHeight - 1) / formatInfo.blockHeight);
                rowPitch = widthCount * formatInfo.bytesPerBlock; // BytesPerBlock is 8 or 16
                slicePitch = rowPitch * heightCount;
                break;

                // ASTC compressed formats
            case PixelFormat::ASTC4x4Unorm:
            case PixelFormat::ASTC4x4UnormSrgb:
            case PixelFormat::ASTC5x4Unorm:
            case PixelFormat::ASTC5x4UnormSrgb:
            case PixelFormat::ASTC5x5Unorm:
            case PixelFormat::ASTC5x5UnormSrgb:
            case PixelFormat::ASTC6x5Unorm:
            case PixelFormat::ASTC6x5UnormSrgb:
            case PixelFormat::ASTC6x6Unorm:
            case PixelFormat::ASTC6x6UnormSrgb:
            case PixelFormat::ASTC8x5Unorm:
            case PixelFormat::ASTC8x5UnormSrgb:
            case PixelFormat::ASTC8x6Unorm:
            case PixelFormat::ASTC8x6UnormSrgb:
            case PixelFormat::ASTC8x8Unorm:
            case PixelFormat::ASTC8x8UnormSrgb:
            case PixelFormat::ASTC10x5Unorm:
            case PixelFormat::ASTC10x5UnormSrgb:
            case PixelFormat::ASTC10x6Unorm:
            case PixelFormat::ASTC10x6UnormSrgb:
            case PixelFormat::ASTC10x8Unorm:
            case PixelFormat::ASTC10x8UnormSrgb:
            case PixelFormat::ASTC10x10Unorm:
            case PixelFormat::ASTC10x10UnormSrgb:
            case PixelFormat::ASTC12x10Unorm:
            case PixelFormat::ASTC12x10UnormSrgb:
            case PixelFormat::ASTC12x12Unorm:
                widthCount = Max(1u, (width + formatInfo.blockWidth - 1) / formatInfo.blockWidth);
                heightCount = Max(1u, (height + formatInfo.blockHeight - 1) / formatInfo.blockHeight);
                rowPitch = widthCount * formatInfo.bytesPerBlock;  // BytesPerBlock is always 16
                slicePitch = rowPitch * heightCount;
                break;

                // ASTC HDR compressed formats
            case PixelFormat::ASTC4x4HDR:
            case PixelFormat::ASTC5x4HDR:
            case PixelFormat::ASTC5x5HDR:
            case PixelFormat::ASTC6x5HDR:
            case PixelFormat::ASTC6x6HDR:
            case PixelFormat::ASTC8x5HDR:
            case PixelFormat::ASTC8x6HDR:
            case PixelFormat::ASTC8x8HDR:
            case PixelFormat::ASTC10x5HDR:
            case PixelFormat::ASTC10x6HDR:
            case PixelFormat::ASTC10x8HDR:
            case PixelFormat::ASTC10x10HDR:
            case PixelFormat::ASTC12x10HDR:
            case PixelFormat::ASTC12x12HDR:
                widthCount = Max(1u, (width + formatInfo.blockWidth - 1) / formatInfo.blockWidth);
                heightCount = Max(1u, (height + formatInfo.blockHeight - 1) / formatInfo.blockHeight);
                rowPitch = widthCount * formatInfo.bytesPerBlock;  // BytesPerBlock is always 16
                slicePitch = rowPitch * heightCount;
                break;

                //case Format.R8G8_B8G8_UNorm:
                //case Format.G8R8_G8B8_UNorm:
                //case Format.YUY2:
                //    packed = true;
                //    bpe = 4;
                //    break;
                //
                //case Format.Y210:
                //case Format.Y216:
                //    packed = true;
                //    bpe = 8;
                //    break;
                //
                //case Format.NV12:
                //case Format.Opaque420:
                //case Format.P208:
                //    planar = true;
                //    bpe = 2;
                //    break;
                //
                //case Format.P010:
                //case Format.P016:
                //    planar = true;
                //    bpe = 4;
                //    break;

            default:
                const uint32_t bpp = BitsPerPixel(format);
                rowPitch = (width * bpp + 7) / 8; // round up to nearest byte
                slicePitch = rowPitch * height;
                break;
        }

        if (pRowPitch)
            *pRowPitch = rowPitch;

        if (pSlicePitch)
            *pSlicePitch = slicePitch;

        if (pWidthCount)
            *pWidthCount = widthCount;

        if (pHeightCount)
            *pHeightCount = heightCount;
    }

    uint32_t ToDxgiFormat(PixelFormat format)
    {
        const bool UsePackedDepth24UnormStencil8Format = true;

        switch (format)
        {
            // 8-bit formats
            case PixelFormat::R8Unorm:                  return DXGI_FORMAT_R8_UNORM;
            case PixelFormat::R8Snorm:                  return DXGI_FORMAT_R8_SNORM;
            case PixelFormat::R8Uint:                   return DXGI_FORMAT_R8_UINT;
            case PixelFormat::R8Sint:                   return DXGI_FORMAT_R8_SINT;
                // 16-bit formats
            case PixelFormat::R16Unorm:                 return DXGI_FORMAT_R16_UNORM;
            case PixelFormat::R16Snorm:                 return DXGI_FORMAT_R16_SNORM;
            case PixelFormat::R16Uint:                  return DXGI_FORMAT_R16_UINT;
            case PixelFormat::R16Sint:                  return DXGI_FORMAT_R16_SINT;
            case PixelFormat::R16Float:                 return DXGI_FORMAT_R16_FLOAT;
            case PixelFormat::RG8Unorm:                 return DXGI_FORMAT_R8G8_UNORM;
            case PixelFormat::RG8Snorm:                 return DXGI_FORMAT_R8G8_SNORM;
            case PixelFormat::RG8Uint:                  return DXGI_FORMAT_R8G8_UINT;
            case PixelFormat::RG8Sint:                  return DXGI_FORMAT_R8G8_SINT;
                // Packed 16-Bit Pixel Formats
            case PixelFormat::B5G6R5Unorm:              return DXGI_FORMAT_B5G6R5_UNORM;
            case PixelFormat::BGR5A1Unorm:              return DXGI_FORMAT_B5G5R5A1_UNORM;
            case PixelFormat::BGRA4Unorm:               return DXGI_FORMAT_B4G4R4A4_UNORM;
                // 32-bit formats
            case PixelFormat::R32Uint:                  return DXGI_FORMAT_R32_UINT;
            case PixelFormat::R32Sint:                  return DXGI_FORMAT_R32_SINT;
            case PixelFormat::R32Float:                 return DXGI_FORMAT_R32_FLOAT;
            case PixelFormat::RG16Unorm:                return DXGI_FORMAT_R16G16_UNORM;
            case PixelFormat::RG16Snorm:                return DXGI_FORMAT_R16G16_SNORM;
            case PixelFormat::RG16Uint:                 return DXGI_FORMAT_R16G16_UINT;
            case PixelFormat::RG16Sint:                 return DXGI_FORMAT_R16G16_SINT;
            case PixelFormat::RG16Float:                return DXGI_FORMAT_R16G16_FLOAT;
            case PixelFormat::RGBA8Unorm:               return DXGI_FORMAT_R8G8B8A8_UNORM;
            case PixelFormat::RGBA8UnormSrgb:           return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case PixelFormat::RGBA8Snorm:               return DXGI_FORMAT_R8G8B8A8_SNORM;
            case PixelFormat::RGBA8Uint:                return DXGI_FORMAT_R8G8B8A8_UINT;
            case PixelFormat::RGBA8Sint:                return DXGI_FORMAT_R8G8B8A8_SINT;
            case PixelFormat::BGRA8Unorm:               return DXGI_FORMAT_B8G8R8A8_UNORM;
            case PixelFormat::BGRA8UnormSrgb:           return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                // Packed 32-Bit formats
            case PixelFormat::RGB10A2Unorm:             return DXGI_FORMAT_R10G10B10A2_UNORM;
            case PixelFormat::RGB10A2Uint:              return DXGI_FORMAT_R10G10B10A2_UINT;
            case PixelFormat::RG11B10Float:             return DXGI_FORMAT_R11G11B10_FLOAT;
            case PixelFormat::RGB9E5Float:              return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                // 64-Bit formats
            case PixelFormat::RG32Uint:                 return DXGI_FORMAT_R32G32_UINT;
            case PixelFormat::RG32Sint:                 return DXGI_FORMAT_R32G32_SINT;
            case PixelFormat::RG32Float:                return DXGI_FORMAT_R32G32_FLOAT;
            case PixelFormat::RGBA16Unorm:              return DXGI_FORMAT_R16G16B16A16_UNORM;
            case PixelFormat::RGBA16Snorm:              return DXGI_FORMAT_R16G16B16A16_SNORM;
            case PixelFormat::RGBA16Uint:               return DXGI_FORMAT_R16G16B16A16_UINT;
            case PixelFormat::RGBA16Sint:               return DXGI_FORMAT_R16G16B16A16_SINT;
            case PixelFormat::RGBA16Float:              return DXGI_FORMAT_R16G16B16A16_FLOAT;
                // 128-Bit formats
            case PixelFormat::RGBA32Uint:               return DXGI_FORMAT_R32G32B32A32_UINT;
            case PixelFormat::RGBA32Sint:               return DXGI_FORMAT_R32G32B32A32_SINT;
            case PixelFormat::RGBA32Float:              return DXGI_FORMAT_R32G32B32A32_FLOAT;
                // Depth-stencil formats
            case PixelFormat::Stencil8:                 return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case PixelFormat::Depth16Unorm:             return DXGI_FORMAT_D16_UNORM;

            case PixelFormat::Depth24UnormStencil8:
                return UsePackedDepth24UnormStencil8Format ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            case PixelFormat::Depth32Float:             return DXGI_FORMAT_D32_FLOAT;

            case PixelFormat::Depth32FloatStencil8:     return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                // Compressed BC formats
            case PixelFormat::BC1RGBAUnorm:             return DXGI_FORMAT_BC1_UNORM;
            case PixelFormat::BC1RGBAUnormSrgb:         return DXGI_FORMAT_BC1_UNORM_SRGB;
            case PixelFormat::BC2RGBAUnorm:             return DXGI_FORMAT_BC2_UNORM;
            case PixelFormat::BC2RGBAUnormSrgb:         return DXGI_FORMAT_BC2_UNORM_SRGB;
            case PixelFormat::BC3RGBAUnorm:             return DXGI_FORMAT_BC3_UNORM;
            case PixelFormat::BC3RGBAUnormSrgb:         return DXGI_FORMAT_BC3_UNORM_SRGB;
            case PixelFormat::BC4RSnorm:                return DXGI_FORMAT_BC4_UNORM;
            case PixelFormat::BC4RUnorm:                return DXGI_FORMAT_BC4_SNORM;
            case PixelFormat::BC5RGUnorm:               return DXGI_FORMAT_BC5_UNORM;
            case PixelFormat::BC5RGSnorm:               return DXGI_FORMAT_BC5_SNORM;
            case PixelFormat::BC6HRGBUfloat:            return DXGI_FORMAT_BC6H_UF16;
            case PixelFormat::BC6HRGBFloat:             return DXGI_FORMAT_BC6H_SF16;
            case PixelFormat::BC7RGBAUnorm:             return DXGI_FORMAT_BC7_UNORM;
            case PixelFormat::BC7RGBAUnormSrgb:         return DXGI_FORMAT_BC7_UNORM_SRGB;

            default:
                return DXGI_FORMAT_UNKNOWN;
        }
    }

    PixelFormat FromDxgiFormat(uint32_t dxgiFormat)
    {
        switch (dxgiFormat)
        {
            // 8-bit formats
            case DXGI_FORMAT_R8_UNORM:                  return PixelFormat::R8Unorm;
            case DXGI_FORMAT_R8_SNORM:                  return PixelFormat::R8Snorm;
            case DXGI_FORMAT_R8_UINT:                   return PixelFormat::R8Uint;
            case DXGI_FORMAT_R8_SINT:                   return PixelFormat::R8Sint;
                // 16-bit formats
            case DXGI_FORMAT_R16_UNORM:                 return PixelFormat::R16Unorm;
            case DXGI_FORMAT_R16_SNORM:                 return PixelFormat::R16Snorm;
            case DXGI_FORMAT_R16_UINT:                  return PixelFormat::R16Uint;
            case DXGI_FORMAT_R16_SINT:                  return PixelFormat::R16Sint;
            case DXGI_FORMAT_R16_FLOAT:                 return PixelFormat::R16Float;
            case DXGI_FORMAT_R8G8_UNORM:                return PixelFormat::RG8Unorm;
            case DXGI_FORMAT_R8G8_SNORM:                return PixelFormat::RG8Snorm;
            case DXGI_FORMAT_R8G8_UINT:                 return PixelFormat::RG8Uint;
            case DXGI_FORMAT_R8G8_SINT:                 return PixelFormat::RG8Sint;
                // Packed 16-Bit Pixel Formats
            case DXGI_FORMAT_B5G6R5_UNORM:              return PixelFormat::B5G6R5Unorm;
            case DXGI_FORMAT_B5G5R5A1_UNORM:            return PixelFormat::BGR5A1Unorm;
            case DXGI_FORMAT_B4G4R4A4_UNORM:            return PixelFormat::BGRA4Unorm;
                // 32-bit formats
            case DXGI_FORMAT_R32_UINT:                  return PixelFormat::R32Uint;
            case DXGI_FORMAT_R32_SINT:                  return PixelFormat::R32Sint;
            case DXGI_FORMAT_R32_FLOAT:                 return PixelFormat::R32Float;
            case DXGI_FORMAT_R16G16_UNORM:              return PixelFormat::RG16Unorm;
            case DXGI_FORMAT_R16G16_SNORM:              return PixelFormat::RG16Snorm;
            case DXGI_FORMAT_R16G16_UINT:               return PixelFormat::RG16Uint;
            case DXGI_FORMAT_R16G16_SINT:               return PixelFormat::RG16Sint;
            case DXGI_FORMAT_R16G16_FLOAT:              return PixelFormat::RG16Float;
            case DXGI_FORMAT_R8G8B8A8_UNORM:            return PixelFormat::RGBA8Unorm;
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:       return PixelFormat::RGBA8UnormSrgb;
            case DXGI_FORMAT_R8G8B8A8_SNORM:            return PixelFormat::RGBA8Snorm;
            case DXGI_FORMAT_R8G8B8A8_UINT:             return PixelFormat::RGBA8Uint;
            case DXGI_FORMAT_R8G8B8A8_SINT:             return PixelFormat::RGBA8Sint;
            case DXGI_FORMAT_B8G8R8A8_UNORM:            return PixelFormat::BGRA8Unorm;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:       return PixelFormat::BGRA8UnormSrgb;
                // Packed 32-Bit formats
            case DXGI_FORMAT_R10G10B10A2_UNORM:         return PixelFormat::RGB10A2Unorm;
            case DXGI_FORMAT_R10G10B10A2_UINT:          return PixelFormat::RGB10A2Uint;
            case DXGI_FORMAT_R11G11B10_FLOAT:           return PixelFormat::RG11B10Float;
            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:        return PixelFormat::RGB9E5Float;
                // 64-Bit formats
            case DXGI_FORMAT_R32G32_UINT:               return PixelFormat::RG32Uint;
            case DXGI_FORMAT_R32G32_SINT:               return PixelFormat::RG32Sint;
            case DXGI_FORMAT_R32G32_FLOAT:              return PixelFormat::RG32Float;
            case DXGI_FORMAT_R16G16B16A16_UNORM:        return PixelFormat::RGBA16Unorm;
            case DXGI_FORMAT_R16G16B16A16_SNORM:        return PixelFormat::RGBA16Snorm;
            case DXGI_FORMAT_R16G16B16A16_UINT:         return PixelFormat::RGBA16Uint;
            case DXGI_FORMAT_R16G16B16A16_SINT:         return PixelFormat::RGBA16Sint;
            case DXGI_FORMAT_R16G16B16A16_FLOAT:        return PixelFormat::RGBA16Float;
                // 128-Bit formats
            case DXGI_FORMAT_R32G32B32A32_UINT:         return PixelFormat::RGBA32Uint;
            case DXGI_FORMAT_R32G32B32A32_SINT:         return PixelFormat::RGBA32Sint;
            case DXGI_FORMAT_R32G32B32A32_FLOAT:        return PixelFormat::RGBA32Float;
                // Depth-stencil formats
            case DXGI_FORMAT_D16_UNORM:			        return PixelFormat::Depth16Unorm;
            case DXGI_FORMAT_D32_FLOAT:			        return PixelFormat::Depth32Float;
                //case DXGI_FORMAT_D24_UNORM_S8_UINT:		return PixelFormat::Stencil8;
            case DXGI_FORMAT_D24_UNORM_S8_UINT:         return PixelFormat::Depth24UnormStencil8;
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:      return PixelFormat::Depth32FloatStencil8;
                // Compressed BC formats
            case DXGI_FORMAT_BC1_UNORM:                 return PixelFormat::BC1RGBAUnorm;
            case DXGI_FORMAT_BC1_UNORM_SRGB:            return PixelFormat::BC1RGBAUnormSrgb;
            case DXGI_FORMAT_BC2_UNORM:                 return PixelFormat::BC2RGBAUnorm;
            case DXGI_FORMAT_BC2_UNORM_SRGB:            return PixelFormat::BC2RGBAUnormSrgb;
            case DXGI_FORMAT_BC3_UNORM:                 return PixelFormat::BC3RGBAUnorm;
            case DXGI_FORMAT_BC3_UNORM_SRGB:            return PixelFormat::BC3RGBAUnormSrgb;
            case DXGI_FORMAT_BC4_UNORM:                 return PixelFormat::BC4RSnorm;
            case DXGI_FORMAT_BC4_SNORM:                 return PixelFormat::BC4RUnorm;
            case DXGI_FORMAT_BC5_UNORM:                 return PixelFormat::BC5RGUnorm;
            case DXGI_FORMAT_BC5_SNORM:                 return PixelFormat::BC5RGSnorm;
            case DXGI_FORMAT_BC6H_UF16:                 return PixelFormat::BC6HRGBUfloat;
            case DXGI_FORMAT_BC6H_SF16:                 return PixelFormat::BC6HRGBFloat;
            case DXGI_FORMAT_BC7_UNORM:                 return PixelFormat::BC7RGBAUnorm;
            case DXGI_FORMAT_BC7_UNORM_SRGB:            return PixelFormat::BC7RGBAUnormSrgb;

            default:
                return PixelFormat::Undefined;
        }
    }

    uint32_t ToVkFormat(PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case PixelFormat::R8Unorm:                    return VK_FORMAT_R8_UNORM;
            case PixelFormat::R8Snorm:                    return VK_FORMAT_R8_SNORM;
            case PixelFormat::R8Uint:                     return VK_FORMAT_R8_UINT;
            case PixelFormat::R8Sint:                     return VK_FORMAT_R8_SINT;
                // 16-bit formats
            case PixelFormat::R16Uint:                    return VK_FORMAT_R16_UINT;
            case PixelFormat::R16Sint:                    return VK_FORMAT_R16_SINT;
            case PixelFormat::R16Unorm:                   return VK_FORMAT_R16_UNORM;
            case PixelFormat::R16Snorm:                   return VK_FORMAT_R16_SNORM;
            case PixelFormat::R16Float:                   return VK_FORMAT_R16_SFLOAT;
            case PixelFormat::RG8Unorm:                   return VK_FORMAT_R8G8_UNORM;
            case PixelFormat::RG8Snorm:                   return VK_FORMAT_R8G8_SNORM;
            case PixelFormat::RG8Uint:                    return VK_FORMAT_R8G8_UINT;
            case PixelFormat::RG8Sint:                    return VK_FORMAT_R8G8_SINT;
                // Packed 16-Bit Pixel Formats
            case PixelFormat::B5G6R5Unorm:                return VK_FORMAT_B5G6R5_UNORM_PACK16;
            case PixelFormat::BGR5A1Unorm:                return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            case PixelFormat::BGRA4Unorm:                 return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
                // 32-bit formats
            case PixelFormat::R32Uint:                    return VK_FORMAT_R32_UINT;
            case PixelFormat::R32Sint:                    return VK_FORMAT_R32_SINT;
            case PixelFormat::R32Float:                   return VK_FORMAT_R32_SFLOAT;
            case PixelFormat::RG16Uint:                   return VK_FORMAT_R16G16_UINT;
            case PixelFormat::RG16Sint:                   return VK_FORMAT_R16G16_SINT;
            case PixelFormat::RG16Unorm:                  return VK_FORMAT_R16G16_UNORM;
            case PixelFormat::RG16Snorm:                  return VK_FORMAT_R16G16_SNORM;
            case PixelFormat::RG16Float:                  return VK_FORMAT_R16G16_SFLOAT;
            case PixelFormat::RGBA8Unorm:                 return VK_FORMAT_R8G8B8A8_UNORM;
            case PixelFormat::RGBA8UnormSrgb:             return VK_FORMAT_R8G8B8A8_SRGB;
            case PixelFormat::RGBA8Snorm:                 return VK_FORMAT_R8G8B8A8_SNORM;
            case PixelFormat::RGBA8Uint:                  return VK_FORMAT_R8G8B8A8_UINT;
            case PixelFormat::RGBA8Sint:                  return VK_FORMAT_R8G8B8A8_SINT;
            case PixelFormat::BGRA8Unorm:                 return VK_FORMAT_B8G8R8A8_UNORM;
            case PixelFormat::BGRA8UnormSrgb:             return VK_FORMAT_B8G8R8A8_SRGB;
                // Packed 32-Bit formats
            case PixelFormat::RGB10A2Unorm:               return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case PixelFormat::RGB10A2Uint:                return VK_FORMAT_A2R10G10B10_UINT_PACK32;
            case PixelFormat::RG11B10Float:               return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            case PixelFormat::RGB9E5Float:                return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
                // 64-Bit formats
            case PixelFormat::RG32Uint:                   return VK_FORMAT_R32G32_UINT;
            case PixelFormat::RG32Sint:                   return VK_FORMAT_R32G32_SINT;
            case PixelFormat::RG32Float:                  return VK_FORMAT_R32G32_SFLOAT;
            case PixelFormat::RGBA16Uint:                 return VK_FORMAT_R16G16B16A16_UINT;
            case PixelFormat::RGBA16Sint:                 return VK_FORMAT_R16G16B16A16_SINT;
            case PixelFormat::RGBA16Unorm:                return VK_FORMAT_R16G16B16A16_UNORM;
            case PixelFormat::RGBA16Snorm:                return VK_FORMAT_R16G16B16A16_SNORM;
            case PixelFormat::RGBA16Float:                return VK_FORMAT_R16G16B16A16_SFLOAT;
                // 128-Bit formats
            case PixelFormat::RGBA32Uint:                 return VK_FORMAT_R32G32B32A32_UINT;
            case PixelFormat::RGBA32Sint:                 return VK_FORMAT_R32G32B32A32_SINT;
            case PixelFormat::RGBA32Float:                return VK_FORMAT_R32G32B32A32_SFLOAT;
                // Depth-stencil formats
            case PixelFormat::Stencil8:                 return VK_FORMAT_S8_UINT;
            case PixelFormat::Depth16Unorm:             return VK_FORMAT_D16_UNORM;
            case PixelFormat::Depth24UnormStencil8:     return VK_FORMAT_D24_UNORM_S8_UINT;
            case PixelFormat::Depth32Float:             return VK_FORMAT_D32_SFLOAT;
            case PixelFormat::Depth32FloatStencil8:     return VK_FORMAT_D32_SFLOAT_S8_UINT;

                // Compressed BC formats
            case PixelFormat::BC1RGBAUnorm:                 return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case PixelFormat::BC1RGBAUnormSrgb:             return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            case PixelFormat::BC2RGBAUnorm:                 return VK_FORMAT_BC2_UNORM_BLOCK;
            case PixelFormat::BC2RGBAUnormSrgb:             return VK_FORMAT_BC2_SRGB_BLOCK;
            case PixelFormat::BC3RGBAUnorm:                 return VK_FORMAT_BC3_UNORM_BLOCK;
            case PixelFormat::BC3RGBAUnormSrgb:             return VK_FORMAT_BC3_SRGB_BLOCK;
            case PixelFormat::BC4RUnorm:                    return VK_FORMAT_BC4_UNORM_BLOCK;
            case PixelFormat::BC4RSnorm:                    return VK_FORMAT_BC4_SNORM_BLOCK;
            case PixelFormat::BC5RGUnorm:                   return VK_FORMAT_BC5_UNORM_BLOCK;
            case PixelFormat::BC5RGSnorm:                   return VK_FORMAT_BC5_SNORM_BLOCK;
            case PixelFormat::BC6HRGBUfloat:                return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case PixelFormat::BC6HRGBFloat:                 return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case PixelFormat::BC7RGBAUnorm:                 return VK_FORMAT_BC7_UNORM_BLOCK;
            case PixelFormat::BC7RGBAUnormSrgb:             return VK_FORMAT_BC7_SRGB_BLOCK;
                // EAC/ETC compressed formats
            case PixelFormat::ETC2RGB8Unorm:              return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            case PixelFormat::ETC2RGB8UnormSrgb:          return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
            case PixelFormat::ETC2RGB8A1Unorm:            return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            case PixelFormat::ETC2RGB8A1UnormSrgb:        return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
            case PixelFormat::ETC2RGBA8Unorm:             return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            case PixelFormat::ETC2RGBA8UnormSrgb:         return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
            case PixelFormat::EACR11Unorm:                return VK_FORMAT_EAC_R11_UNORM_BLOCK;
            case PixelFormat::EACR11Snorm:                return VK_FORMAT_EAC_R11_SNORM_BLOCK;
            case PixelFormat::EACRG11Unorm:               return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
            case PixelFormat::EACRG11Snorm:               return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
                // ASTC compressed formats
            case PixelFormat::ASTC4x4Unorm:               return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            case PixelFormat::ASTC4x4UnormSrgb:           return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
            case PixelFormat::ASTC5x4Unorm:               return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            case PixelFormat::ASTC5x4UnormSrgb:           return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
            case PixelFormat::ASTC5x5Unorm:               return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            case PixelFormat::ASTC5x5UnormSrgb:           return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
            case PixelFormat::ASTC6x5Unorm:               return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            case PixelFormat::ASTC6x5UnormSrgb:           return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
            case PixelFormat::ASTC6x6Unorm:               return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            case PixelFormat::ASTC6x6UnormSrgb:           return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
            case PixelFormat::ASTC8x5Unorm:               return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            case PixelFormat::ASTC8x5UnormSrgb:           return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
            case PixelFormat::ASTC8x6Unorm:               return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            case PixelFormat::ASTC8x6UnormSrgb:           return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
            case PixelFormat::ASTC8x8Unorm:               return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            case PixelFormat::ASTC8x8UnormSrgb:           return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
            case PixelFormat::ASTC10x5Unorm:              return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            case PixelFormat::ASTC10x5UnormSrgb:          return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
            case PixelFormat::ASTC10x6Unorm:              return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            case PixelFormat::ASTC10x6UnormSrgb:          return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
            case PixelFormat::ASTC10x8Unorm:              return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            case PixelFormat::ASTC10x8UnormSrgb:          return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
            case PixelFormat::ASTC10x10Unorm:             return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            case PixelFormat::ASTC10x10UnormSrgb:         return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
            case PixelFormat::ASTC12x10Unorm:             return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            case PixelFormat::ASTC12x10UnormSrgb:         return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
            case PixelFormat::ASTC12x12Unorm:             return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            case PixelFormat::ASTC12x12UnormSrgb:         return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
                // ASTC HDR compressed formats
            case PixelFormat::ASTC4x4HDR:           return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
            case PixelFormat::ASTC5x4HDR:           return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK;
            case PixelFormat::ASTC5x5HDR:           return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK;
            case PixelFormat::ASTC6x5HDR:           return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK;
            case PixelFormat::ASTC6x6HDR:           return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK;
            case PixelFormat::ASTC8x5HDR:           return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK;
            case PixelFormat::ASTC8x6HDR:           return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK;
            case PixelFormat::ASTC8x8HDR:           return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK;
            case PixelFormat::ASTC10x5HDR:          return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK;
            case PixelFormat::ASTC10x6HDR:          return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK;
            case PixelFormat::ASTC10x8HDR:          return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;
            case PixelFormat::ASTC10x10HDR:         return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK;
            case PixelFormat::ASTC12x10HDR:         return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK;
            case PixelFormat::ASTC12x12HDR:         return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK;

                //case PixelFormat::R8BG8Biplanar420Unorm:      return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    PixelFormat FromVkFormat(uint32_t vkFormat)
    {
        switch (vkFormat)
        {
            // 8-bit formats
            case VK_FORMAT_R8_UNORM:                        return PixelFormat::R8Unorm;
            case VK_FORMAT_R8_SNORM:                        return PixelFormat::R8Snorm;
            case VK_FORMAT_R8_UINT:                         return PixelFormat::R8Uint;
            case VK_FORMAT_R8_SINT:                         return PixelFormat::R8Sint;
                // 16-bit formats
            case VK_FORMAT_R16_UNORM:                       return PixelFormat::R16Unorm;
            case VK_FORMAT_R16_SNORM:                       return PixelFormat::R16Snorm;
            case VK_FORMAT_R16_UINT:                        return PixelFormat::R16Uint;
            case VK_FORMAT_R16_SINT:                        return PixelFormat::R16Sint;
            case VK_FORMAT_R16_SFLOAT:                      return PixelFormat::R16Float;
            case VK_FORMAT_R8G8_UNORM:                      return PixelFormat::RG8Unorm;
            case VK_FORMAT_R8G8_SNORM:                      return PixelFormat::RG8Snorm;
            case VK_FORMAT_R8G8_UINT:                       return PixelFormat::RG8Uint;
            case VK_FORMAT_R8G8_SINT:                       return PixelFormat::RG8Sint;
                // Packed 16-Bit Pixel Formats
            case VK_FORMAT_B5G6R5_UNORM_PACK16:             return PixelFormat::B5G6R5Unorm;
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:           return PixelFormat::BGR5A1Unorm;
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:           return PixelFormat::BGRA4Unorm;
                // 32-bit formats
            case VK_FORMAT_R32_UINT:                        return PixelFormat::R32Uint;
            case VK_FORMAT_R32_SINT:                        return PixelFormat::R32Sint;
            case VK_FORMAT_R32_SFLOAT:                      return PixelFormat::R32Float;
            case VK_FORMAT_R16G16_UNORM:                    return PixelFormat::RG16Unorm;
            case VK_FORMAT_R16G16_SNORM:                    return PixelFormat::RG16Snorm;
            case VK_FORMAT_R16G16_UINT:                     return PixelFormat::RG16Uint;
            case VK_FORMAT_R16G16_SINT:                     return PixelFormat::RG16Sint;
            case VK_FORMAT_R16G16_SFLOAT:                   return PixelFormat::RG16Float;
            case VK_FORMAT_R8G8B8A8_UNORM:                  return PixelFormat::RGBA8Unorm;
            case VK_FORMAT_R8G8B8A8_SRGB:                   return PixelFormat::RGBA8UnormSrgb;
            case VK_FORMAT_R8G8B8A8_SNORM:                  return PixelFormat::RGBA8Snorm;
            case VK_FORMAT_R8G8B8A8_UINT:                   return PixelFormat::RGBA8Uint;
            case VK_FORMAT_R8G8B8A8_SINT:                   return PixelFormat::RGBA8Sint;
            case VK_FORMAT_B8G8R8A8_UNORM:                  return PixelFormat::BGRA8Unorm;
            case VK_FORMAT_B8G8R8A8_SRGB:                   return PixelFormat::BGRA8UnormSrgb;
                // Packed 32-Bit formats
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:        return PixelFormat::RGB10A2Unorm;
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:         return PixelFormat::RGB10A2Uint;
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:         return PixelFormat::RG11B10Float;
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:          return PixelFormat::RGB9E5Float;
                // 64-Bit formats
            case VK_FORMAT_R32G32_UINT:                     return PixelFormat::RG32Uint;
            case VK_FORMAT_R32G32_SINT:                     return PixelFormat::RG32Sint;
            case VK_FORMAT_R32G32_SFLOAT:                   return PixelFormat::RG32Float;
            case VK_FORMAT_R16G16B16A16_UNORM:              return PixelFormat::RGBA16Unorm;
            case VK_FORMAT_R16G16B16A16_SNORM:              return PixelFormat::RGBA16Snorm;
            case VK_FORMAT_R16G16B16A16_UINT:               return PixelFormat::RGBA16Uint;
            case VK_FORMAT_R16G16B16A16_SINT:               return PixelFormat::RGBA16Sint;
            case VK_FORMAT_R16G16B16A16_SFLOAT:             return PixelFormat::RGBA16Float;
                // 128-Bit formats
            case VK_FORMAT_R32G32B32A32_UINT:               return PixelFormat::RGBA32Uint;
            case VK_FORMAT_R32G32B32A32_SINT:               return PixelFormat::RGBA32Sint;
            case VK_FORMAT_R32G32B32A32_SFLOAT:             return PixelFormat::RGBA32Float;
                // Depth-stencil formats
            case VK_FORMAT_D16_UNORM:			            return PixelFormat::Depth16Unorm;
            case VK_FORMAT_D32_SFLOAT:			            return PixelFormat::Depth32Float;
            case VK_FORMAT_D24_UNORM_S8_UINT:               return PixelFormat::Depth24UnormStencil8;
            case VK_FORMAT_D32_SFLOAT_S8_UINT:              return PixelFormat::Depth32FloatStencil8;
                // Compressed BC formats
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:            return PixelFormat::BC1RGBAUnorm;
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:             return PixelFormat::BC1RGBAUnormSrgb;
            case VK_FORMAT_BC2_UNORM_BLOCK:                 return PixelFormat::BC2RGBAUnorm;
            case VK_FORMAT_BC2_SRGB_BLOCK:                  return PixelFormat::BC2RGBAUnormSrgb;
            case VK_FORMAT_BC3_UNORM_BLOCK:                 return PixelFormat::BC3RGBAUnorm;
            case VK_FORMAT_BC3_SRGB_BLOCK:                  return PixelFormat::BC3RGBAUnormSrgb;
            case VK_FORMAT_BC4_UNORM_BLOCK:                 return PixelFormat::BC4RSnorm;
            case VK_FORMAT_BC4_SNORM_BLOCK:                 return PixelFormat::BC4RUnorm;
            case VK_FORMAT_BC5_UNORM_BLOCK:                 return PixelFormat::BC5RGUnorm;
            case VK_FORMAT_BC5_SNORM_BLOCK:                 return PixelFormat::BC5RGSnorm;
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:               return PixelFormat::BC6HRGBUfloat;
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:               return PixelFormat::BC6HRGBFloat;
            case VK_FORMAT_BC7_UNORM_BLOCK:                 return PixelFormat::BC7RGBAUnorm;
            case VK_FORMAT_BC7_SRGB_BLOCK:                  return PixelFormat::BC7RGBAUnormSrgb;
                // EAC/ETC compressed formats
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:         return PixelFormat::ETC2RGB8Unorm;
            case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:          return PixelFormat::ETC2RGB8UnormSrgb;
            case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:       return PixelFormat::ETC2RGB8A1Unorm;
            case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:        return PixelFormat::ETC2RGB8A1UnormSrgb;
            case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:       return PixelFormat::ETC2RGBA8Unorm;
            case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:        return PixelFormat::ETC2RGBA8UnormSrgb;
            case VK_FORMAT_EAC_R11_UNORM_BLOCK:             return PixelFormat::EACR11Unorm;
            case VK_FORMAT_EAC_R11_SNORM_BLOCK:             return PixelFormat::EACR11Snorm;
            case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:          return PixelFormat::EACRG11Unorm;
            case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:          return PixelFormat::EACRG11Snorm;
                // ASTC compressed formats
            case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:            return PixelFormat::ASTC4x4Unorm;
            case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:             return PixelFormat::ASTC4x4UnormSrgb;
            case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:            return PixelFormat::ASTC5x4Unorm;
            case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:             return PixelFormat::ASTC5x4UnormSrgb;
            case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:            return PixelFormat::ASTC5x5Unorm;
            case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:             return PixelFormat::ASTC5x5UnormSrgb;
            case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:            return PixelFormat::ASTC6x5Unorm;
            case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:             return PixelFormat::ASTC6x5UnormSrgb;
            case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:            return PixelFormat::ASTC6x6Unorm;
            case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:             return PixelFormat::ASTC6x6UnormSrgb;
            case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:            return PixelFormat::ASTC8x5Unorm;
            case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:             return PixelFormat::ASTC8x5UnormSrgb;
            case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:            return PixelFormat::ASTC8x6Unorm;
            case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:             return PixelFormat::ASTC8x6UnormSrgb;
            case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:            return PixelFormat::ASTC8x8Unorm;
            case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:             return PixelFormat::ASTC8x8UnormSrgb;
            case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:           return PixelFormat::ASTC10x5Unorm;
            case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:            return PixelFormat::ASTC10x5UnormSrgb;
            case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:           return PixelFormat::ASTC10x6Unorm;
            case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:            return PixelFormat::ASTC10x6UnormSrgb;
            case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:           return PixelFormat::ASTC10x8Unorm;
            case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:            return PixelFormat::ASTC10x8UnormSrgb;
            case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:          return PixelFormat::ASTC10x10Unorm;
            case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:           return PixelFormat::ASTC10x10UnormSrgb;
            case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:          return PixelFormat::ASTC12x10Unorm;
            case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:           return PixelFormat::ASTC12x10UnormSrgb;
            case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:          return PixelFormat::ASTC12x12Unorm;
            case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:           return PixelFormat::ASTC12x12UnormSrgb;
                // ASTC HDR compressed formats
            case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:           return PixelFormat::ASTC4x4HDR;
            case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:           return PixelFormat::ASTC5x4HDR;
            case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:           return PixelFormat::ASTC5x5HDR;
            case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:           return PixelFormat::ASTC6x5HDR;
            case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:           return PixelFormat::ASTC6x6HDR;
            case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:           return PixelFormat::ASTC8x5HDR;
            case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:           return PixelFormat::ASTC8x6HDR;
            case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:           return PixelFormat::ASTC8x8HDR;
            case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:          return PixelFormat::ASTC10x5HDR;
            case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:          return PixelFormat::ASTC10x6HDR;
            case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:          return PixelFormat::ASTC10x8HDR;
            case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:         return PixelFormat::ASTC10x10HDR;
            case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:         return PixelFormat::ASTC12x10HDR;
            case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:         return PixelFormat::ASTC12x12HDR;
            default:
                return PixelFormat::Undefined;
        }
    }
}
