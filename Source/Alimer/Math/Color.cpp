// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Color.h"
#include "Alimer/Core/Hash.h"

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
#include "Alimer/Math/SIMD.h"
#endif

using namespace Alimer;

namespace
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    static const VectorU32 g_XMMaskA8R8G8B8 = { { { 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 } } };
    static const VectorU32 g_XMFlipA8R8G8B8 = { { { 0x00000000, 0x00000000, 0x00000000, 0x80000000 } } };
    static const VectorF32 g_XMFixAA8R8G8B8 = { { { 0.0f, 0.0f, 0.0f, float(0x80000000U) } } };
    static const VectorF32 g_XMNormalizeA8R8G8B8 = { { { 1.0f / (255.0f * float(0x10000)), 1.0f / (255.0f * float(0x100)), 1.0f / 255.0f, 1.0f / (255.0f * float(0x1000000)) } } };

    // XMStoreColor
    inline ColorBgra simd_store_color_bgra(simd_float4_param vector) noexcept
    {
        ColorBgra result;
#if defined(ALIMER_USE_SSE)
        // Set <0 to 0
        simd_float4 vResult = _mm_max_ps(vector, g_XMZero);
        // Set>1 to 1
        vResult = _mm_min_ps(vResult, g_XMOne);
        // Convert to 0-255
        vResult = _mm_mul_ps(vResult, g_UByteMax);
        // Shuffle RGBA to ARGB
        vResult = XM_PERMUTE_PS(vResult, _MM_SHUFFLE(3, 0, 1, 2));
        // Convert to int
        __m128i vInt = _mm_cvtps_epi32(vResult);
        // Mash to shorts
        vInt = _mm_packs_epi32(vInt, vInt);
        // Mash to bytes
        vInt = _mm_packus_epi16(vInt, vInt);
        // Store the color
        _mm_store_ss(reinterpret_cast<float*>(&result.packedValue), _mm_castsi128_ps(vInt));
#elif defined(ALIMER_USE_NEON)
        float32x4_t R = vmaxq_f32(vector, vdupq_n_f32(0));
        R = vminq_f32(R, vdupq_n_f32(1.0f));
        R = vmulq_n_f32(R, 255.0f);
        R = simd_float4_round(R);
        uint32x4_t vInt32 = vcvtq_u32_f32(R);
        uint16x4_t vInt16 = vqmovn_u32(vInt32);
        uint8x8_t vInt8 = vqmovn_u16(vcombine_u16(vInt16, vInt16));
        uint32_t rgba = vget_lane_u32(vreinterpret_u32_u8(vInt8), 0);
        result.packedValue = (rgba & 0xFF00FF00) | ((rgba >> 16) & 0xFF) | ((rgba << 16) & 0xFF0000);
#endif
        return result;
    }

    // XMStoreUByteN4
    inline ColorRgba simd_store_color_rgba(simd_float4_param vector) noexcept
    {
        ColorRgba result;
#if defined(ALIMER_USE_SSE)
        static const VectorF32 ScaleUByteN4 = { { { 255.0f, 255.0f * 256.0f * 0.5f, 255.0f * 256.0f * 256.0f, 255.0f * 256.0f * 256.0f * 256.0f * 0.5f } } };
        static const VectorF32 MaskUByteN4 = { { { 0xFF, 0xFF << (8 - 1), 0xFF << 16, 0xFF << (24 - 1) } } };
        // Clamp to bounds
        __m128 vResult = _mm_max_ps(vector, g_XMZero);
        vResult = _mm_min_ps(vResult, g_XMOne);
        // Scale by multiplication
        vResult = _mm_mul_ps(vResult, ScaleUByteN4);
        // Convert to int
        __m128i vResulti = _mm_cvttps_epi32(vResult);
        // Mask off any fraction
        vResulti = _mm_and_si128(vResulti, MaskUByteN4);
        // Do a horizontal or of 4 entries
        __m128i vResulti2 = _mm_shuffle_epi32(vResulti, _MM_SHUFFLE(3, 2, 3, 2));
        // x = x|z, y = y|w
        vResulti = _mm_or_si128(vResulti, vResulti2);
        // Move Z to the x position
        vResulti2 = _mm_shuffle_epi32(vResulti, _MM_SHUFFLE(1, 1, 1, 1));
        // Perform a single bit left shift to fix y|w
        vResulti2 = _mm_add_epi32(vResulti2, vResulti2);
        // i = x|y|z|w
        vResulti = _mm_or_si128(vResulti, vResulti2);
        _mm_store_ss(reinterpret_cast<float*>(&result.packedValue), _mm_castsi128_ps(vResulti));
#elif defined(ALIMER_USE_NEON)
        float32x4_t R = vmaxq_f32(vector, vdupq_n_f32(0));
        R = vminq_f32(R, vdupq_n_f32(1.0f));
        R = vmulq_n_f32(R, 255.0f);
        uint32x4_t vInt32 = vcvtq_u32_f32(R);
        uint16x4_t vInt16 = vqmovn_u32(vInt32);
        uint8x8_t vInt8 = vqmovn_u16(vcombine_u16(vInt16, vInt16));
        vst1_lane_u32(&result.packedValue, vreinterpret_u32_u8(vInt8), 0);
#endif
        return result;
    }
#else
    static constexpr Vector3 g_Vector3UByteMax = { 255.0f, 255.0f, 255.0f };
    static constexpr Vector4 g_Vector4UByteMax = { 255.0f, 255.0f, 255.0f, 255.0f };
#endif
}

ColorBgra ColorBgra::FromFloat4(float r_, float g_, float b_, float a_) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    return simd_store_color_bgra(simd_make_float4(r_, g_, b_, a_));
#else
    Vector4 N = Vector4::Saturate(Vector4(r_, g_, b_, a_));
    N *= g_Vector4UByteMax;
    N = Vector4::Round(N);

    uint32_t packedValue = (static_cast<uint32_t>(N.w) << 24) |
        (static_cast<uint32_t>(N.x) << 16) |
        (static_cast<uint32_t>(N.y) << 8) |
        static_cast<uint32_t>(N.z);
    return ColorBgra(packedValue);
#endif
}

ColorBgra ColorBgra::FromFloat4(_In_reads_(4) const float* data) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    return simd_store_color_bgra(simd_make_float4(data));
#else
    Vector4 N = Vector4::Saturate(Vector4(data));
    N *= g_Vector4UByteMax;
    N = Vector4::Round(N);

    uint32_t packedValue = (static_cast<uint32_t>(N.w) << 24) |
        (static_cast<uint32_t>(N.x) << 16) |
        (static_cast<uint32_t>(N.y) << 8) |
        static_cast<uint32_t>(N.z);
    return ColorBgra(packedValue);
#endif
}

ColorRgba ColorRgba::FromFloat3(float r, float g, float b) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    return simd_store_color_rgba(simd_make_float4(r, g, b, 1.0f));
#else
    Vector3 N = Vector3::Saturate(Vector3(r, g, b));
    N *= g_Vector3UByteMax;
    N = Vector3::Truncate(N);

    return ColorRgba(
        static_cast<uint8_t>(N.x),
        static_cast<uint8_t>(N.y),
        static_cast<uint8_t>(N.z),
        255);
#endif
}


ColorRgba ColorRgba::FromFloat4(float r, float g, float b, float a) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    return simd_store_color_rgba(simd_make_float4(r, g, b, a));
#else
    Vector4 N = Vector4::Saturate(Vector4(r, g, b, a));
    N *= g_Vector4UByteMax;
    N = Vector4::Truncate(N);

    return ColorRgba(
        static_cast<uint8_t>(N.x),
        static_cast<uint8_t>(N.y),
        static_cast<uint8_t>(N.z),
        static_cast<uint8_t>(N.w)
    );
#endif
}

ColorRgba ColorRgba::FromFloat4(_In_reads_(4) const float* data) noexcept
{
#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
    return simd_store_color_rgba(simd_make_float4(data));
#else
    Vector4 N = Vector4::Saturate(Vector4(data));
    N *= g_Vector4UByteMax;
    N = Vector4::Truncate(N);

    return ColorRgba(
        static_cast<uint8_t>(N.x),
        static_cast<uint8_t>(N.y),
        static_cast<uint8_t>(N.z),
        static_cast<uint8_t>(N.w)
    );
#endif
}

Color::Color(const ColorBgra& colorBgra) noexcept
{
    // XMLoadColor
#if defined(ALIMER_USE_SSE)
    // Splat the color in all four entries
    __m128i vInt = _mm_set1_epi32(static_cast<int>(colorBgra.packedValue));
    // Shift R&0xFF0000, G&0xFF00, B&0xFF, A&0xFF000000
    vInt = _mm_and_si128(vInt, g_XMMaskA8R8G8B8);
    // a is unsigned! Flip the bit to convert the order to signed
    vInt = _mm_xor_si128(vInt, g_XMFlipA8R8G8B8);
    // Convert to floating point numbers
    __m128 vTemp = _mm_cvtepi32_ps(vInt);
    // RGB + 0, A + 0x80000000.f to undo the signed order.
    vTemp = _mm_add_ps(vTemp, g_XMFixAA8R8G8B8);
    // Convert 0-255 to 0.0f-1.0f
    simd_float4 simdVector = _mm_mul_ps(vTemp, g_XMNormalizeA8R8G8B8);
    simd_float4_store(reinterpret_cast<Vector4*>(this), simdVector);
#elif defined(ALIMER_USE_NEON)
    uint32_t bgra = colorBgra.packedValue;
    uint32_t rgba = (bgra & 0xFF00FF00) | ((bgra >> 16) & 0xFF) | ((bgra << 16) & 0xFF0000);
    uint32x2_t vInt8 = vdup_n_u32(rgba);
    uint16x8_t vInt16 = vmovl_u8(vreinterpret_u8_u32(vInt8));
    uint32x4_t vInt = vmovl_u16(vget_low_u16(vInt16));
    float32x4_t R = vcvtq_f32_u32(vInt);
    simd_float4 simdVector = vmulq_n_f32(R, 1.0f / 255.0f);
    simd_float4_store(reinterpret_cast<Vector4*>(this), simdVector);
#else
    // int32_t -> Float conversions are done in one instruction.
    // uint32_t -> Float calls a runtime function. Keep in int32_t
    auto iColor = static_cast<int32_t>(colorBgra.packedValue);
    r = static_cast<float>((iColor >> 16) & 0xFF) * (1.0f / 255.0f);
    g = static_cast<float>((iColor >> 8) & 0xFF) * (1.0f / 255.0f);
    b = static_cast<float>(iColor & 0xFF) * (1.0f / 255.0f);
    a = static_cast<float>((iColor >> 24) & 0xFF) * (1.0f / 255.0f);
#endif
}

Color::Color(const ColorRgba& colorRgba) noexcept
{
    // XMLoadUByteN4
#if defined(ALIMER_USE_SSE)
    static const VectorF32 LoadUByteN4Mul = { { { 1.0f / 255.0f, 1.0f / (255.0f * 256.0f), 1.0f / (255.0f * 65536.0f), 1.0f / (255.0f * 65536.0f * 256.0f) } } };
    // Splat the color in all four entries (x,z,y,w)
    __m128 vTemp = _mm_load1_ps(reinterpret_cast<const float*>(&colorRgba.r));
    // Mask x&0ff,y&0xff00,z&0xff0000,w&0xff000000
    vTemp = _mm_and_ps(vTemp, g_XMMaskByte4);
    // w is signed! Flip the bits to convert the order to unsigned
    vTemp = _mm_xor_ps(vTemp, g_XMFlipW);
    // Convert to floating point numbers
    vTemp = _mm_cvtepi32_ps(_mm_castps_si128(vTemp));
    // w + 0x80 to complete the conversion
    vTemp = _mm_add_ps(vTemp, g_XMAddUDec4);
    // Fix y, z and w because they are too large
    vTemp = _mm_mul_ps(vTemp, LoadUByteN4Mul);
    simd_float4_store(reinterpret_cast<Vector4*>(this), vTemp);
#elif defined(ALIMER_USE_NEON)
    uint32x2_t vInt8 = vld1_dup_u32(reinterpret_cast<const uint32_t*>(&colorRgba));
    uint16x8_t vInt16 = vmovl_u8(vreinterpret_u8_u32(vInt8));
    uint32x4_t vInt = vmovl_u16(vget_low_u16(vInt16));
    float32x4_t R = vcvtq_f32_u32(vInt);
    simd_float4 simdVector = vmulq_n_f32(R, 1.0f / 255.0f);
    simd_float4_store(reinterpret_cast<Vector4*>(this), simdVector);
#else
    r = static_cast<float>(colorRgba.r) / 255.0f;
    g = static_cast<float>(colorRgba.g) / 255.0f;
    b = static_cast<float>(colorRgba.b) / 255.0f;
    a = static_cast<float>(colorRgba.a) / 255.0f;
#endif
}

Color Color::Parse(StringView str)
{
    Color result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return Colors::Black;
}

bool Color::TryParse(StringView str, Color* result)
{
    ALIMER_ASSERT(result);
    size_t elements = StringUtils::CountElements(str, ' ');
    if (elements < 3)
        return false;

    char* ptr = (char*)str.data();
    result->r = (float)strtod(ptr, &ptr);
    result->g = (float)strtod(ptr, &ptr);
    result->b = (float)strtod(ptr, &ptr);
    if (elements > 3)
        result->a = (float)strtod(ptr, &ptr);

    return true;
}

float Color::Chroma() const
{
    float min, max;
    Bounds(&min, &max, true);

    return max - min;
}

float Color::Hue() const
{
    float min, max;
    Bounds(&min, &max, true);

    return Hue(min, max);
}

float Color::SaturationHSL() const
{
    float min, max;
    Bounds(&min, &max, true);

    return SaturationHSL(min, max);
}

float Color::SaturationHSV() const
{
    float min, max;
    Bounds(&min, &max, true);

    return SaturationHSV(min, max);
}

float Color::Lightness() const
{
    float min, max;
    Bounds(&min, &max, true);

    return (max + min) * 0.5f;
}

void Color::Bounds(float* min, float* max, bool clipped) const
{
    ALIMER_ASSERT(min && max);

    if (r > g)
    {
        if (g > b) // r > g > b
        {
            *max = r;
            *min = b;
        }
        else // r > g && g <= b
        {
            *max = r > b ? r : b;
            *min = g;
        }
    }
    else
    {
        if (b > g) // r <= g < b
        {
            *max = b;
            *min = r;
        }
        else // r <= g && b <= g
        {
            *max = g;
            *min = r < b ? r : b;
        }
    }

    if (clipped)
    {
        *max = *max > 1.0f ? 1.0f : (*max < 0.0f ? 0.0f : *max);
        *min = *min > 1.0f ? 1.0f : (*min < 0.0f ? 0.0f : *min);
    }
}

float Color::MaxRGB() const
{
    if (r > g)
        return (r > b) ? r : b;

    return (g > b) ? g : b;
}

float Color::MinRGB() const
{
    if (r < g)
        return (r < b) ? r : b;

    return (g < b) ? g : b;
}

float Color::Range() const
{
    float min, max;
    Bounds(&min, &max);
    return max - min;
}

uint32_t Color::ToBgra() const
{
    // XMStoreColor
    uint32_t a = (uint32_t)(this->a * 255.0f) & 255;
    uint32_t r = (uint32_t)(this->r * 255.0f) & 255;
    uint32_t g = (uint32_t)(this->g * 255.0f) & 255;
    uint32_t b = (uint32_t)(this->b * 255.0f) & 255;

    uint32_t value = b;
    value |= g << 8;
    value |= r << 16;
    value |= a << 24;
    return value;
}

ColorRgba Color::ToRgba() const
{
    // XMStoreUByteN4
    ColorRgba result;
#if defined(ALIMER_USE_SSE)
    static const VectorF32 ScaleUByteN4 = { { { 255.0f, 255.0f * 256.0f * 0.5f, 255.0f * 256.0f * 256.0f, 255.0f * 256.0f * 256.0f * 256.0f * 0.5f } } };
    static const VectorF32 MaskUByteN4 = { { { 0xFF, 0xFF << (8 - 1), 0xFF << 16, 0xFF << (24 - 1) } } };
    // Clamp to bounds
    __m128 vResult = _mm_max_ps(simd_make_float4(r, g, b, a), g_XMZero);
    vResult = _mm_min_ps(vResult, g_XMOne);
    // Scale by multiplication
    vResult = _mm_mul_ps(vResult, ScaleUByteN4);
    // Convert to int
    __m128i vResulti = _mm_cvttps_epi32(vResult);
    // Mask off any fraction
    vResulti = _mm_and_si128(vResulti, MaskUByteN4);
    // Do a horizontal or of 4 entries
    __m128i vResulti2 = _mm_shuffle_epi32(vResulti, _MM_SHUFFLE(3, 2, 3, 2));
    // x = x|z, y = y|w
    vResulti = _mm_or_si128(vResulti, vResulti2);
    // Move Z to the x position
    vResulti2 = _mm_shuffle_epi32(vResulti, _MM_SHUFFLE(1, 1, 1, 1));
    // Perform a single bit left shift to fix y|w
    vResulti2 = _mm_add_epi32(vResulti2, vResulti2);
    // i = x|y|z|w
    vResulti = _mm_or_si128(vResulti, vResulti2);
    _mm_store_ss(reinterpret_cast<float*>(&result.packedValue), _mm_castsi128_ps(vResulti));
#elif defined(ALIMER_USE_NEON)
    float32x4_t R = vmaxq_f32(simd_make_float4(r, g, b, a), vdupq_n_f32(0));
    R = vminq_f32(R, vdupq_n_f32(1.0f));
    R = vmulq_n_f32(R, 255.0f);
    uint32x4_t vInt32 = vcvtq_u32_f32(R);
    uint16x4_t vInt16 = vqmovn_u32(vInt32);
    uint8x8_t vInt8 = vqmovn_u16(vcombine_u16(vInt16, vInt16));
    vst1_lane_u32(&result.packedValue, vreinterpret_u32_u8(vInt8), 0);
#else
    Vector4 N = Vector4::Saturate(Vector4(r, g, b, a));
    N *= g_Vector4UByteMax;
    N = Vector4::Truncate(N);

    result.r = static_cast<uint8_t>(N.x);
    result.g = static_cast<uint8_t>(N.y);
    result.b = static_cast<uint8_t>(N.z);
    result.a = static_cast<uint8_t>(N.w);
#endif
    return result;
}

Vector3 Color::ToHSL() const
{
    float min, max;
    Bounds(&min, &max, true);

    float h = Hue(min, max);
    float s = SaturationHSL(min, max);
    float l = (max + min) * 0.5f;

    return Vector3(h, s, l);
}

Vector3 Color::ToHSV() const
{
    float min, max;
    Bounds(&min, &max, true);

    float h = Hue(min, max);
    float s = SaturationHSV(min, max);
    float v = max;

    return Vector3(h, s, v);
}

Color Color::Clamp(const Color& value, const Color& min, const Color& max) noexcept
{
    return Color(
        Alimer::Clamp(value.r, min.r, max.r),
        Alimer::Clamp(value.g, min.g, max.b),
        Alimer::Clamp(value.b, min.b, max.a),
        Alimer::Clamp(value.a, min.a, max.a)
    );
}

Color Color::Saturate(const Color& vector) noexcept
{
    return Clamp(vector, Colors::Transparent, Colors::White);
}

Color Color::Lerp(const Color& value1, const Color& value2, float amount) noexcept
{
    return Color(
        value1.r + (value2.r - value1.r) * amount,
        value1.g + (value2.g - value1.g) * amount,
        value1.b + (value2.b - value1.b) * amount,
        value1.a + (value2.a - value1.a) * amount
    );
}

Color Color::LinearToSRgb(const Color& value)
{
    return Color(
        MathF::LinearToSRgb(value.r),
        MathF::LinearToSRgb(value.g),
        MathF::LinearToSRgb(value.b),
        value.a
    );
}

Color Color::SRgbToLinear(const Color& value)
{
    return Color(
        MathF::SRgbToLinear(value.r),
        MathF::SRgbToLinear(value.g),
        MathF::SRgbToLinear(value.b),
        value.a
    );
}

float Color::Hue(float min, float max) const
{
    float chroma = max - min;

    // If chroma equals zero, hue is undefined
    if (chroma <= M_EPSILON)
        return 0.0f;

    // Calculate and return hue
    if (MathF::Equals(g, max))
    {
        return (b + 2.0f * chroma - r) / (6.0f * chroma);
    }
    else if (MathF::Equals(b, max))
    {
        return (4.0f * chroma - g + r) / (6.0f * chroma);
    }
    else
    {
        float result = (g - b) / (6.0f * chroma);
        return (result < 0.0f) ? 1.0f + result : ((result >= 1.0f) ? result - 1.0f : result);
    }
}

float Color::SaturationHSV(float min, float max) const
{
    // Avoid div-by-zero: result undefined
    if (max <= M_EPSILON)
        return 0.0f;

    // Saturation equals chroma:value ratio
    return 1.0f - (min / max);
}

float Color::SaturationHSL(float min, float max) const
{
    // Avoid div-by-zero: result undefined
    if (max <= M_EPSILON || min >= 1.0f - M_EPSILON)
        return 0.0f;

    // Chroma = max - min, lightness = (max + min) * 0.5
    const float hl = (max + min);
    if (hl <= 1.0f)
        return (max - min) / hl;

    return (min - max) / (hl - 2.0f);
}

std::string Color::ToString() const
{
    char tempBuffer[kConversionBufferLength];
    snprintf(tempBuffer, kConversionBufferLength, "%g %g %g %g", r, g, b, a);
    return std::string(tempBuffer);
}

std::string Color::ToArgbHex(bool includeAlpha) const
{
    if (includeAlpha || a < 1.0f)
        return "#" + ToHex(a) + ToHex(r) + ToHex(g) + ToHex(b);

    return "#" + ToHex(r) + ToHex(g) + ToHex(b);
}

std::string Color::ToRgbaHex(bool includeAlpha) const
{
    if (includeAlpha || a < 1.0f)
        return "#" + ToHex(r) + ToHex(g) + ToHex(b) + ToHex(a);

    return "#" + ToHex(r) + ToHex(g) + ToHex(b);
}

std::string Color::ToHex(float value)
{
    int intValue = (int)(255.0f * value);
    std::string stringValue = FMT::format("{:x}", intValue);
    if (stringValue.length() == 1)
    {
        return "0" + stringValue;
    }

    return stringValue;
}

size_t Color::GetHashCode() const
{
    size_t hash = 0;
    HashCombine(hash, r, g, b, a);
    return hash;
}
