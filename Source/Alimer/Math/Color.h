// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/MathHelper.h"
#include "Alimer/Math/Vector4.h"

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#   pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4201) // nameless struct/union
#endif

namespace Alimer
{
    /// ARGB Color; 8-8-8-8 bit unsigned normalized integer components packed into a 32 bit integer.
    struct ALIMER_API ColorBgra final
    {
        union
        {
            struct
            {
                uint8_t b;  // Blue:    0/255 to 255/255
                uint8_t g;  // Green:   0/255 to 255/255
                uint8_t r;  // Red:     0/255 to 255/255
                uint8_t a;  // Alpha:   0/255 to 255/255
            };
            uint32_t packedValue;
        };

        ColorBgra() = default;

        ColorBgra(const ColorBgra&) = default;
        ColorBgra& operator=(const ColorBgra&) = default;

        ColorBgra(ColorBgra&&) = default;
        ColorBgra& operator=(ColorBgra&&) = default;

        constexpr ColorBgra(uint8_t b_, uint8_t g_, uint8_t r_, uint8_t a_ = 255) noexcept
            : b(b_)
            , g(g_)
            , r(r_)
            , a(a_)
        {
        }

        constexpr ColorBgra(uint32_t packedValue_) noexcept : packedValue(packedValue_) {}
        operator uint32_t () const noexcept { return packedValue; }

        ColorBgra& operator= (const uint32_t color) noexcept { packedValue = color; return *this; }

        static ColorBgra FromFloat4(float r, float g, float b, float a) noexcept;
        static ColorBgra FromFloat4(_In_reads_(4) const float* data) noexcept;
    };

    // 4D Vector; 8 bit unsigned normalized integer components
    struct ALIMER_API ColorRgba final
    {
        union
        {
            struct
            {
                uint8_t r;  // Red:     0/255 to 255/255
                uint8_t g;  // Green:   0/255 to 255/255
                uint8_t b;  // Blue:    0/255 to 255/255
                uint8_t a;  // Alpha:   0/255 to 255/255
            };
            uint32_t packedValue;
        };

        ColorRgba() = default;

        ColorRgba(const ColorRgba&) = default;
        ColorRgba& operator=(const ColorRgba&) = default;

        ColorRgba(ColorRgba&&) = default;
        ColorRgba& operator=(ColorRgba&&) = default;

        constexpr ColorRgba(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) noexcept
            : r(r_)
            , g(g_)
            , b(b_)
            , a(a_)
        {
        }

        explicit constexpr ColorRgba(uint32_t packedValue_) noexcept
            : packedValue(packedValue_)
        {
        }
        explicit ColorRgba(_In_reads_(4) const uint8_t* pArray) noexcept
            : r(pArray[0])
            , g(pArray[1])
            , b(pArray[2])
            , a(pArray[3])
        {
        }

        ColorRgba(float r_, float g_, float b_, float a_) noexcept;
        explicit ColorRgba(_In_reads_(4) const float* pArray) noexcept;

        operator uint32_t () const noexcept { return packedValue; }

        ColorRgba& operator= (const uint32_t color) noexcept { packedValue = color; return *this; }
    };

    /// Class specifying a floating-point rgba color.
    struct ALIMER_API Color final
    {
    public:
        /// Specifies the red component of the color.
        float r;
        /// Specifies the green component of the color.
        float g;
        /// Specifies the blue component of the color.
        float b;
        /// Specifies the alpha component of the color.
        float a;

        /// Constructor.
        constexpr Color() noexcept
            : r(0.0f)
            , g(0.0f)
            , b(0.0f)
            , a(1.0f)
        {
        }

        constexpr Color(float r_, float g_, float b_) noexcept
            : r(r_), g(g_), b(b_), a(1.0f)
        {
        }

        constexpr Color(float r_, float g_, float b_, float a_) noexcept
            : r(r_), g(g_), b(b_), a(a_)
        {
        }

        constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) noexcept
            : r((float)r_ / 255.0f)
            , g((float)g_ / 255.0f)
            , b((float)b_ / 255.0f)
            , a((float)a_ / 255.0f)
        {
        }

        /// Construct from another color and modify the alpha.
        Color(const Color& color, float a_) noexcept
            : r(color.r)
            , g(color.g)
            , b(color.b)
            , a(a_)
        {
        }

        explicit Color(float rgba) noexcept
            : r(rgba)
            , g(rgba)
            , b(rgba)
            , a(rgba)
        {
        }

        explicit Color(const Vector3& vector, float a_ = 1.0f) noexcept
            : r(vector.x), g(vector.y), b(vector.z), a(a_)
        {
        }

        explicit Color(const Vector4& vector) noexcept
            : r(vector.x), g(vector.y), b(vector.z), a(vector.w)
        {
        }

        explicit Color(_In_reads_(4) const float* data) noexcept
            : r(data[0]), g(data[1]), b(data[2]), a(data[3])
        {
        }

        // BGRA packed color.
        explicit Color(const ColorBgra& colorBgra) noexcept;

        // RGBA packed color.
        explicit Color(const ColorRgba& colorRgba) noexcept;

        Color(const Color&) = default;
        Color& operator=(const Color&) = default;

        Color(Color&&) = default;
        Color& operator=(Color&&) = default;

        // Comparison operators
        bool operator==(const Color& rhs) const noexcept { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; }
        bool operator!=(const Color& rhs) const noexcept { return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a; }

        /// Multiply with a scalar.
        Color operator *(float rhs) const { return Color(r * rhs, g * rhs, b * rhs, a * rhs); }

        /// Modulate color.
        Color operator *(const Color& rhs) const { return Color(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a); }

        /// Add a color.
        Color operator +(const Color& rhs) const { return Color(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a); }

        /// Return negation.
        Color operator -() const { return Color(-r, -g, -b, -a); }

        /// Subtract a color.
        Color operator -(const Color& rhs) const { return Color(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a); }

        /// Add-assign a color.
        Color& operator +=(const Color& rhs)
        {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;
            a += rhs.a;
            return *this;
        }

        operator const float* () const noexcept { return &r; }

        /// Convert from HSV to RGB color space.
        [[nodiscard]] static Color FromHSV(float h, float s, float v, float a = 1.0f);

        [[nodiscard]] static Color Clamp(const Color& value, const Color& min, const Color& max) noexcept;
        [[nodiscard]] static Color Saturate(const Color& vector) noexcept;

        [[nodiscard]] static Color Lerp(const Color& value1, const Color& value2, float amount) noexcept;

        /// Convert color from linear to sRGB (gamma) space.
        [[nodiscard]] static Color LinearToSRgb(const Color& value);

        /// Convert color from sRGB (gamma) to linear space.
        [[nodiscard]] static Color SRgbToLinear(const Color& value);

        /// Parse color from a string. Return Colors::Black on failure.
        [[nodiscard]] static Color Parse(StringView str);

        /// Try parse from a string. Return true on success.
        [[nodiscard]] static bool TryParse(StringView str, Color* result);

        /// Return color packed to a 32-bit integer in BGRA.
        uint32_t ToBgra() const;

        /// Return color packed to a 32-bit integer in BGRA.
        [[nodiscard]] ColorRgba ToRgba() const;

        /// Return HSL color-space representation as a Vector3; the RGB values are clipped before conversion but not changed in the process.
        [[nodiscard]] Vector3 ToHSL() const;
        /// Return HSV color-space representation as a Vector3; the RGB values are clipped before conversion but not changed in the process.
        [[nodiscard]] Vector3 ToHSV() const;

        /// Return RGB as a three-dimensional vector.
        [[nodiscard]] constexpr Vector3 ToVector3() const { return Vector3(r, g, b); }

        /// Return RGBA as a four-dimensional vector.
        [[nodiscard]] constexpr Vector4 ToVector4() const { return Vector4(r, g, b, a); }

        /// Return sum of RGB components.
        [[nodiscard]] constexpr float SumRGB() const { return r + g + b; }

        /// Return average value of the RGB channels.
        [[nodiscard]] constexpr float Average() const { return (r + g + b) / 3.0f; }

        /// Return the 'grayscale' representation of RGB values, as used by JPEG and PAL/NTSC among others.
        [[nodiscard]] constexpr float Luma() const { return r * 0.299f + g * 0.587f + b * 0.114f; }

        /// Return the colorfulness relative to the brightness of a similarly illuminated white.
        [[nodiscard]] float Chroma() const;
        /// Return hue mapped to range [0, 1.0).
        [[nodiscard]] float Hue() const;
        /// Return saturation as defined for HSL.
        [[nodiscard]] float SaturationHSL() const;
        /// Return saturation as defined for HSV.
        [[nodiscard]] float SaturationHSV() const;
        /// Return lightness as defined for HSL: average of the largest and smallest values of the RGB components.
        [[nodiscard]] float Lightness() const;
        /// Return value as defined for HSV: largest value of the RGB components. Equivalent to calling MinRGB().
        [[nodiscard]] float Value() const { return MaxRGB(); }

        /// Stores the values of least and greatest RGB component at specified pointer addresses, optionally clipping those values to [0, 1] range.
        void Bounds(float* min, float* max, bool clipped = false) const;
        /// Return the largest value of the RGB components.
        [[nodiscard]] float MaxRGB() const;
        /// Return the smallest value of the RGB components.
        [[nodiscard]] float MinRGB() const;
        /// Return range, defined as the difference between the greatest and least RGB component.
        [[nodiscard]] float Range() const;

        [[nodiscard]] constexpr Color Premultiply() const { return { r * a, g * a, b * a, a }; }

        /// Return float data.
        const float* Data() const { return &r; }

        /// Return as string.
        [[nodiscard]] std::string ToString() const;

        [[nodiscard]] std::string ToArgbHex(bool includeAlpha = false) const;
        [[nodiscard]] std::string ToRgbaHex(bool includeAlpha = false) const;

        /// Return hash value of the color.
        [[nodiscard]] size_t GetHashCode() const;

    protected:
        /// Return hue value given greatest and least RGB component, value-wise.
        float Hue(float min, float max) const;
        /// Return saturation (HSV) given greatest and least RGB component, value-wise.
        float SaturationHSV(float min, float max) const;
        /// Return saturation (HSL) given greatest and least RGB component, value-wise.
        float SaturationHSL(float min, float max) const;

        static std::string ToHex(float value);
    };

    /// Multiply Color with a scalar.
    inline Color operator *(float lhs, const Color& rhs) { return rhs * lhs; }

    /// Copied from DirectXMath (https://github.com/microsoft/DirectXMath/blob/b412a61c518923e52e2d43d9e4d7084af8352ca2/Inc/DirectXColors.h)
    class ALIMER_API Colors
    {
    public:
        // Standard colors (Red/Green/Blue/Alpha)
        static constexpr Color Black = { 0.0f, 0.0f, 0.0f, 1.0f };
        static constexpr Color White = { 1.0f, 1.0f, 1.0f, 1.0f };
        static constexpr Color Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };
        static constexpr Color CornflowerBlue = { 0.392156899f, 0.584313750f, 0.929411829f, 1.0f };

        static constexpr Color AliceBlue = { 0.941176534f, 0.972549081f, 1.0f, 1.0f };
        static constexpr Color AntiqueWhite = { 0.980392218f, 0.921568692f, 0.843137324f, 1.0f };
        static constexpr Color Aqua = { 0.0f, 1.0f, 1.0f, 1.0f };
        static constexpr Color Aquamarine = { 0.498039246f, 1.0f, 0.831372619f, 1.0f };
        static constexpr Color Azure = { 0.941176534f, 1.0f, 1.0f, 1.0f };
        static constexpr Color Beige = { 0.960784376f, 0.960784376f, 0.862745166f, 1.0f };
        static constexpr Color Bisque = { 1.0f, 0.894117713f, 0.768627524f, 1.0f };
        static constexpr Color BlanchedAlmond = { 1.0f, 0.921568692f, 0.803921640f, 1.0f };
        static constexpr Color Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
        static constexpr Color BlueViolet = { 0.541176498f, 0.168627456f, 0.886274576f, 1.0f };
        static constexpr Color Brown = { 0.647058845f, 0.164705887f, 0.164705887f, 1.0f };
        static constexpr Color BurlyWood = { 0.870588303f, 0.721568644f, 0.529411793f, 1.0f };
        static constexpr Color CadetBlue = { 0.372549027f, 0.619607866f, 0.627451003f, 1.0f };
        static constexpr Color Chartreuse = { 0.498039246f, 1.0f, 0.0f, 1.0f };
        static constexpr Color Chocolate = { 0.823529482f, 0.411764741f, 0.117647067f, 1.0f };
        static constexpr Color Coral = { 1.0f, 0.498039246f, 0.313725501f, 1.0f };

        static constexpr Color Cornsilk = { 1.0f, 0.972549081f, 0.862745166f, 1.0f };
        static constexpr Color Crimson = { 0.862745166f, 0.078431375f, 0.235294133f, 1.0f };
        static constexpr Color Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
        static constexpr Color DarkBlue = { 0.0f, 0.0f, 0.545098066f, 1.0f };
        static constexpr Color DarkCyan = { 0.0f, 0.545098066f, 0.545098066f, 1.0f };
        static constexpr Color DarkGoldenrod = { 0.721568644f, 0.525490224f, 0.043137256f, 1.0f };
        static constexpr Color DarkGray = { 0.662745118f, 0.662745118f, 0.662745118f, 1.0f };
        static constexpr Color DarkGreen = { 0.0f, 0.392156899f, 0.0f, 1.0f };
        static constexpr Color DarkKhaki = { 0.741176486f, 0.717647076f, 0.419607878f, 1.0f };
        static constexpr Color DarkMagenta = { 0.545098066f, 0.0f, 0.545098066f, 1.0f };
        static constexpr Color DarkOliveGreen = { 0.333333343f, 0.419607878f, 0.184313729f, 1.0f };
        static constexpr Color DarkOrange = { 1.0f, 0.549019635f, 0.0f, 1.0f };
        static constexpr Color DarkOrchid = { 0.600000024f, 0.196078449f, 0.800000072f, 1.0f };
        static constexpr Color DarkRed = { 0.545098066f, 0.0f, 0.0f, 1.0f };
        static constexpr Color DarkSalmon = { 0.913725555f, 0.588235319f, 0.478431404f, 1.0f };
        static constexpr Color DarkSeaGreen = { 0.560784340f, 0.737254918f, 0.545098066f, 1.0f };
        static constexpr Color DarkSlateBlue = { 0.282352954f, 0.239215702f, 0.545098066f, 1.0f };
        static constexpr Color DarkSlateGray = { 0.184313729f, 0.309803933f, 0.309803933f, 1.0f };
        static constexpr Color DarkTurquoise = { 0.0f, 0.807843208f, 0.819607913f, 1.0f };
        static constexpr Color DarkViolet = { 0.580392182f, 0.0f, 0.827451050f, 1.0f };
        static constexpr Color DeepPink = { 1.0f, 0.078431375f, 0.576470613f, 1.0f };
        static constexpr Color DeepSkyBlue = { 0.0f, 0.749019623f, 1.0f, 1.0f };
        static constexpr Color DimGray = { 0.411764741f, 0.411764741f, 0.411764741f, 1.0f };
        static constexpr Color DodgerBlue = { 0.117647067f, 0.564705908f, 1.0f, 1.0f };
        static constexpr Color Firebrick = { 0.698039234f, 0.133333340f, 0.133333340f, 1.0f };
        static constexpr Color FloralWhite = { 1.0f, 0.980392218f, 0.941176534f, 1.0f };
        static constexpr Color ForestGreen = { 0.133333340f, 0.545098066f, 0.133333340f, 1.0f };
        static constexpr Color Fuchsia = { 1.0f, 0.0f, 1.0f, 1.0f };
        static constexpr Color Gainsboro = { 0.862745166f, 0.862745166f, 0.862745166f, 1.0f };
        static constexpr Color GhostWhite = { 0.972549081f, 0.972549081f, 1.0f, 1.0f };
        static constexpr Color Gold = { 1.0f, 0.843137324f, 0.0f, 1.0f };
        static constexpr Color Goldenrod = { 0.854902029f, 0.647058845f, 0.125490203f, 1.0f };
        static constexpr Color Gray = { 0.501960814f, 0.501960814f, 0.501960814f, 1.0f };
        static constexpr Color Green = { 0.0f, 0.501960814f, 0.0f, 1.0f };
        static constexpr Color GreenYellow = { 0.678431392f, 1.0f, 0.184313729f, 1.0f };
        static constexpr Color Honeydew = { 0.941176534f, 1.0f, 0.941176534f, 1.0f };
        static constexpr Color HotPink = { 1.0f, 0.411764741f, 0.705882370f, 1.0f };
        static constexpr Color IndianRed = { 0.803921640f, 0.360784322f, 0.360784322f, 1.0f };
        static constexpr Color Indigo = { 0.294117659f, 0.0f, 0.509803951f, 1.0f };
        static constexpr Color Ivory = { 1.0f, 1.0f, 0.941176534f, 1.0f };
        static constexpr Color Khaki = { 0.941176534f, 0.901960850f, 0.549019635f, 1.0f };
        static constexpr Color Lavender = { 0.901960850f, 0.901960850f, 0.980392218f, 1.0f };
        static constexpr Color LavenderBlush = { 1.0f, 0.941176534f, 0.960784376f, 1.0f };
        static constexpr Color LawnGreen = { 0.486274540f, 0.988235354f, 0.0f, 1.0f };
        static constexpr Color LemonChiffon = { 1.0f, 0.980392218f, 0.803921640f, 1.0f };
        static constexpr Color LightBlue = { 0.678431392f, 0.847058892f, 0.901960850f, 1.0f };
        static constexpr Color LightCoral = { 0.941176534f, 0.501960814f, 0.501960814f, 1.0f };
        static constexpr Color LightCyan = { 0.878431439f, 1.0f, 1.0f, 1.0f };
        static constexpr Color LightGoldenrodYellow = { 0.980392218f, 0.980392218f, 0.823529482f, 1.0f };
        static constexpr Color LightGreen = { 0.564705908f, 0.933333397f, 0.564705908f, 1.0f };
        static constexpr Color LightGray = { 0.827451050f, 0.827451050f, 0.827451050f, 1.0f };
        static constexpr Color LightPink = { 1.0f, 0.713725507f, 0.756862819f, 1.0f };
        static constexpr Color LightSalmon = { 1.0f, 0.627451003f, 0.478431404f, 1.0f };
        static constexpr Color LightSeaGreen = { 0.125490203f, 0.698039234f, 0.666666687f, 1.0f };
        static constexpr Color LightSkyBlue = { 0.529411793f, 0.807843208f, 0.980392218f, 1.0f };
        static constexpr Color LightSlateGray = { 0.466666698f, 0.533333361f, 0.600000024f, 1.0f };
        static constexpr Color LightSteelBlue = { 0.690196097f, 0.768627524f, 0.870588303f, 1.0f };
        static constexpr Color LightYellow = { 1.0f, 1.0f, 0.878431439f, 1.0f };
        static constexpr Color Lime = { 0.0f, 1.0f, 0.0f, 1.0f };
        static constexpr Color LimeGreen = { 0.196078449f, 0.803921640f, 0.196078449f, 1.0f };
        static constexpr Color Linen = { 0.980392218f, 0.941176534f, 0.901960850f, 1.0f };
        static constexpr Color Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
        static constexpr Color Maroon = { 0.501960814f, 0.0f, 0.0f, 1.0f };
        static constexpr Color MediumAquamarine = { 0.400000036f, 0.803921640f, 0.666666687f, 1.0f };
        static constexpr Color MediumBlue = { 0.0f, 0.0f, 0.803921640f, 1.0f };
        static constexpr Color MediumOrchid = { 0.729411781f, 0.333333343f, 0.827451050f, 1.0f };
        static constexpr Color MediumPurple = { 0.576470613f, 0.439215720f, 0.858823597f, 1.0f };
        static constexpr Color MediumSeaGreen = { 0.235294133f, 0.701960802f, 0.443137288f, 1.0f };
        static constexpr Color MediumSlateBlue = { 0.482352972f, 0.407843173f, 0.933333397f, 1.0f };
        static constexpr Color MediumSpringGreen = { 0.0f, 0.980392218f, 0.603921592f, 1.0f };
        static constexpr Color MediumTurquoise = { 0.282352954f, 0.819607913f, 0.800000072f, 1.0f };
        static constexpr Color MediumVioletRed = { 0.780392230f, 0.082352944f, 0.521568656f, 1.0f };
        static constexpr Color MidnightBlue = { 0.098039225f, 0.098039225f, 0.439215720f, 1.0f };
        static constexpr Color MintCream = { 0.960784376f, 1.0f, 0.980392218f, 1.0f };
        static constexpr Color MistyRose = { 1.0f, 0.894117713f, 0.882353008f, 1.0f };
        static constexpr Color Moccasin = { 1.0f, 0.894117713f, 0.709803939f, 1.0f };
        static constexpr Color NavajoWhite = { 1.0f, 0.870588303f, 0.678431392f, 1.0f };
        static constexpr Color Navy = { 0.0f, 0.0f, 0.501960814f, 1.0f };
        static constexpr Color OldLace = { 0.992156923f, 0.960784376f, 0.901960850f, 1.0f };
        static constexpr Color Olive = { 0.501960814f, 0.501960814f, 0.0f, 1.0f };
        static constexpr Color OliveDrab = { 0.419607878f, 0.556862772f, 0.137254909f, 1.0f };
        static constexpr Color Orange = { 1.0f, 0.647058845f, 0.0f, 1.0f };
        static constexpr Color OrangeRed = { 1.0f, 0.270588249f, 0.0f, 1.0f };
        static constexpr Color Orchid = { 0.854902029f, 0.439215720f, 0.839215755f, 1.0f };
        static constexpr Color PaleGoldenrod = { 0.933333397f, 0.909803987f, 0.666666687f, 1.0f };
        static constexpr Color PaleGreen = { 0.596078455f, 0.984313786f, 0.596078455f, 1.0f };
        static constexpr Color PaleTurquoise = { 0.686274529f, 0.933333397f, 0.933333397f, 1.0f };
        static constexpr Color PaleVioletRed = { 0.858823597f, 0.439215720f, 0.576470613f, 1.0f };
        static constexpr Color PapayaWhip = { 1.0f, 0.937254965f, 0.835294187f, 1.0f };
        static constexpr Color PeachPuff = { 1.0f, 0.854902029f, 0.725490212f, 1.0f };
        static constexpr Color Peru = { 0.803921640f, 0.521568656f, 0.247058839f, 1.0f };
        static constexpr Color Pink = { 1.0f, 0.752941251f, 0.796078503f, 1.0f };
        static constexpr Color Plum = { 0.866666734f, 0.627451003f, 0.866666734f, 1.0f };
        static constexpr Color PowderBlue = { 0.690196097f, 0.878431439f, 0.901960850f, 1.0f };
        static constexpr Color Purple = { 0.501960814f, 0.0f, 0.501960814f, 1.0f };
        static constexpr Color Red = { 1.0f, 0.0f, 0.0f, 1.0f };
        static constexpr Color RosyBrown = { 0.737254918f, 0.560784340f, 0.560784340f, 1.0f };
        static constexpr Color RoyalBlue = { 0.254901975f, 0.411764741f, 0.882353008f, 1.0f };
        static constexpr Color SaddleBrown = { 0.545098066f, 0.270588249f, 0.074509807f, 1.0f };
        static constexpr Color Salmon = { 0.980392218f, 0.501960814f, 0.447058856f, 1.0f };
        static constexpr Color SandyBrown = { 0.956862807f, 0.643137276f, 0.376470625f, 1.0f };
        static constexpr Color SeaGreen = { 0.180392161f, 0.545098066f, 0.341176480f, 1.0f };
        static constexpr Color SeaShell = { 1.0f, 0.960784376f, 0.933333397f, 1.0f };
        static constexpr Color Sienna = { 0.627451003f, 0.321568638f, 0.176470593f, 1.0f };
        static constexpr Color Silver = { 0.752941251f, 0.752941251f, 0.752941251f, 1.0f };
        static constexpr Color SkyBlue = { 0.529411793f, 0.807843208f, 0.921568692f, 1.0f };
        static constexpr Color SlateBlue = { 0.415686309f, 0.352941185f, 0.803921640f, 1.0f };
        static constexpr Color SlateGray = { 0.439215720f, 0.501960814f, 0.564705908f, 1.0f };
        static constexpr Color Snow = { 1.0f, 0.980392218f, 0.980392218f, 1.0f };
        static constexpr Color SpringGreen = { 0.0f, 1.0f, 0.498039246f, 1.0f };
        static constexpr Color SteelBlue = { 0.274509817f, 0.509803951f, 0.705882370f, 1.0f };
        static constexpr Color Tan = { 0.823529482f, 0.705882370f, 0.549019635f, 1.0f };
        static constexpr Color Teal = { 0.0f, 0.501960814f, 0.501960814f, 1.0f };
        static constexpr Color Thistle = { 0.847058892f, 0.749019623f, 0.847058892f, 1.0f };
        static constexpr Color Tomato = { 1.0f, 0.388235331f, 0.278431386f, 1.0f };
        static constexpr Color Turquoise = { 0.250980407f, 0.878431439f, 0.815686345f, 1.0f };
        static constexpr Color Violet = { 0.933333397f, 0.509803951f, 0.933333397f, 1.0f };
        static constexpr Color Wheat = { 0.960784376f, 0.870588303f, 0.701960802f, 1.0f };
        static constexpr Color WhiteSmoke = { 0.960784376f, 0.960784376f, 0.960784376f, 1.0f };
        static constexpr Color Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
        static constexpr Color YellowGreen = { 0.603921592f, 0.803921640f, 0.196078449f, 1.0f };
    };
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif
