// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/MathHelper.h"

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
    struct Vector3;
    struct Vector4;
    struct Quaternion;
    struct Color;
    struct Matrix3x2;
    struct Matrix4x4;

    /// Two-dimensional vector with 32 bit floating point components.
    struct ALIMER_API Vector2 final
    {
    public:
        union
        {
            float data[2];
            struct
            {
                /// X coordinate.
                float x;
                /// Y coordinate.
                float y;
            };
        };

        Vector2() noexcept
            : x(0.0f)
            , y(0.0f)
        {
        }

        constexpr explicit Vector2(float value) noexcept
            : x(value)
            , y(value)
        {
        }

        constexpr Vector2(float x_, float y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        /// Construct from a float array.
        explicit Vector2(_In_reads_(2) const float* data) noexcept
            : x(data[0])
            , y(data[1])
        {
        }

        Vector2(const Vector2&) = default;
        Vector2& operator=(const Vector2&) = default;
        Vector2(Vector2&&) = default;
        Vector2& operator=(Vector2&&) = default;

        /// Sets the elements of this vector to the specified values.
        void Set(float x, float y);

        /// Sets the elements of this vector to those in the specified vector.
        void Set(const Vector2& rhs);

        /// Add-assign a vector.
        Vector2& operator += (const Vector2& rhs) noexcept
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector2& operator -= (const Vector2& rhs) noexcept
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector2& operator *= (float rhs) noexcept
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector2& operator *= (const Vector2& rhs) noexcept
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector2& operator /= (float rhs) noexcept
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector2& operator /= (const Vector2& rhs) noexcept
        {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        /// Test for equality with another vector without epsilon.
        bool operator == (const Vector2& rhs) const noexcept { return x == rhs.x && y == rhs.y; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Vector2& rhs) const noexcept { return !(*this == rhs); }
        /// Add a vector.
        Vector2 operator + (const Vector2& rhs) const noexcept { return Vector2(x + rhs.x, y + rhs.y); }
        /// Return negation.
        Vector2 operator - () const noexcept { return Vector2(-x, -y); }
        /// Subtract a vector.
        Vector2 operator - (const Vector2& rhs) const noexcept { return Vector2(x - rhs.x, y - rhs.y); }
        /// Multiply with a scalar.
        Vector2 operator * (float rhs) const noexcept { return Vector2(x * rhs, y * rhs); }
        /// Multiply with a vector.
        Vector2 operator * (const Vector2& rhs) const noexcept { return Vector2(x * rhs.x, y * rhs.y); }
        /// Divide by a scalar.
        Vector2 operator / (float rhs) const noexcept { return Vector2(x / rhs, y / rhs); }
        /// Divide by a vector.
        Vector2 operator / (const Vector2& rhs) const noexcept { return Vector2(x / rhs.x, y / rhs.y); }

        /// Test for equality with another vector with epsilon.
        [[nodiscard]] bool Equals(const Vector2& rhs, float epsilon = M_EPSILON) const
        {
            return
                MathF::Equals(x, rhs.x, epsilon) &&
                MathF::Equals(y, rhs.y, epsilon);
        }

        /// Return whether all components are zeros.
        [[nodiscard]] bool IsZero() const { return x == 0.f && y == 0.f; }

        /// Return whether all components are ones.
        [[nodiscard]] bool IsOne() const { return x == 1.f && y == 1.f; }

        /// Return whether any component is NaN.
        [[nodiscard]] bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y); }

        /// Return whether any component is Inf.
        [[nodiscard]] bool IsInf() const { return Alimer::IsInf(x) || Alimer::IsInf(y); }

        /// Return length.
        [[nodiscard]] float Length() const noexcept { return sqrtf(x * x + y * y); }

        /// Return squared length.
        [[nodiscard]] float LengthSquared() const noexcept { return x * x + y * y; }

        /// Normalize to unit length.
        void Normalize() noexcept
        {
            float lenSquared = LengthSquared();
            if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x *= invLen;
                y *= invLen;
            }
        }

        /// Return normalized to unit length.
        [[nodiscard]] Vector2 Normalized() const noexcept
        {
            float lenSquared = LengthSquared();
            if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }

            return *this;
        }

        [[nodiscard]] static float Dot(const Vector2& v1, const Vector2& v2);
        [[nodiscard]] static float Cross(const Vector2& v1, const Vector2& v2);

        /// Return normalized to unit length.
        [[nodiscard]] static Vector2 Normalize(const Vector2& vector) noexcept;

        [[nodiscard]] static float Distance(const Vector2& v1, const Vector2& v2);
        [[nodiscard]] static float DistanceSquared(const Vector2& v1, const Vector2& v2);

        [[nodiscard]] static Vector2 Min(const Vector2& value1, const Vector2& value2);
        [[nodiscard]] static Vector2 Max(const Vector2& value1, const Vector2& value2);
        [[nodiscard]] static Vector2 Round(const Vector2& value) noexcept;
        [[nodiscard]] static Vector2 Truncate(const Vector2& value) noexcept;
        [[nodiscard]] static Vector2 Floor(const Vector2& value) noexcept;
        [[nodiscard]] static Vector2 Ceiling(const Vector2& value) noexcept;
        [[nodiscard]] static Vector2 Clamp(const Vector2& value, const Vector2& min, const Vector2& max) noexcept;
        [[nodiscard]] static Vector2 Saturate(const Vector2& vector) noexcept;

        [[nodiscard]] static Vector2 Lerp(const Vector2& value1, const Vector2& value2, float amount) noexcept;
        [[nodiscard]] static Vector2 SmoothStep(const Vector2& value1, const Vector2& value2, float amount) noexcept;

        [[nodiscard]] static Vector2 Transform(const Vector2& value, const Matrix3x2& matrix) noexcept;
        [[nodiscard]] static Vector2 Transform(const Vector2& value, const Matrix4x4& matrix) noexcept;

        [[nodiscard]] static Vector2 Transform(const Vector2& value, const Quaternion& rotation) noexcept;
        [[nodiscard]] static Vector2 TransformNormal(const Vector2& value, const Matrix3x2& matrix) noexcept;
        [[nodiscard]] static Vector2 TransformNormal(const Vector2& value, const Matrix4x4& matrix) noexcept;

        /// Parse vector from a string. Return Vector2::Zero on failure.
        [[nodiscard]] static Vector2 Parse(StringView str);

        /// Try parse from a string. Return true on success.
        [[nodiscard]] static bool TryParse(StringView str, Vector2* result);

        /// Return hash value of the vector.
        [[nodiscard]] size_t GetHashCode() const;

        /// Return as string.
        [[nodiscard]] std::string ToString() const;

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
        explicit Vector2(simd_float4_param xy);

        /// Return SIMD vector.
        [[nodiscard]] simd_float4 ToSIMD() const;

        operator simd_float4() const noexcept { return ToSIMD(); }
#endif

        /// Zero vector.
        static const Vector2 Zero;
        /// (1,1) vector.
        static const Vector2 One;
        /// (1,0) vector.
        static const Vector2 UnitX;
        /// (0,1) vector.
        static const Vector2 UnitY;
    };

    /// Multiply Vector2 with a scalar
    inline Vector2 operator * (float lhs, const Vector2& rhs) { return rhs * lhs; }

    /// Two-dimensional vector with 32 bit signed integer components.
    struct ALIMER_API Int2 final
    {
    public:
        union
        {
            int32_t data[2];
            struct
            {
                /// X coordinate.
                int32_t x;
                /// Y coordinate.
                int32_t y;
            };
        };

        Int2() noexcept
            : x(0)
            , y(0)
        {
        }

        constexpr Int2(int32_t x_, int32_t y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        /// Construct from a float array.
        explicit Int2(_In_reads_(2) const int32_t* data) noexcept
            : x(data[0])
            , y(data[1])
        {
        }

        Int2(const Int2&) = default;
        Int2& operator=(const Int2&) = default;

        Int2(Int2&&) = default;
        Int2& operator=(Int2&&) = default;

        /// Test for equality with another vector.
        bool operator == (const Int2& rhs) const { return x == rhs.x && y == rhs.y; }
        /// Test for inequality with another vector.
        bool operator != (const Int2& rhs) const { return !(*this == rhs); }

        /// Return as string.
        std::string ToString() const;

        /// Zero vector.
        static const Int2 Zero;
        /// (1,1) vector.
        static const Int2 One;
        /// (1,0) vector.
        static const Int2 UnitX;
        /// (0,1) vector.
        static const Int2 UnitY;
    };

    /// Two-dimensional vector with 32 bit unsigned integer components.
    struct ALIMER_API UInt2 final
    {
    public:
        union
        {
            uint32_t data[2];
            struct
            {
                /// X coordinate.
                uint32_t x;
                /// Y coordinate.
                uint32_t y;
            };
        };

        UInt2() noexcept
            : x(0)
            , y(0)
        {
        }

        constexpr UInt2(uint32_t x_, uint32_t y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        /// Construct from a float array.
        explicit UInt2(_In_reads_(2) const uint32_t* data)
            : x(data[0])
            , y(data[1])
        {
        }

        UInt2(const UInt2&) = default;
        UInt2& operator=(const UInt2&) = default;
        UInt2(UInt2&&) = default;
        UInt2& operator=(UInt2&&) = default;

        /// Test for equality with another vector.
        bool operator == (const UInt2& rhs) const { return x == rhs.x && y == rhs.y; }
        /// Test for inequality with another vector.
        bool operator != (const UInt2& rhs) const { return !(*this == rhs); }

        /// Return as string.
        std::string ToString() const;

        /// Zero vector.
        static const UInt2 Zero;
        /// (1,1) vector.
        static const UInt2 One;
        /// (1,0) vector.
        static const UInt2 UnitX;
        /// (0,1) vector.
        static const UInt2 UnitY;
    };

    /// Two-dimensional vector with 64 bit floating point components.
    struct ALIMER_API Double2 final
    {
    public:
        union
        {
            double data[2];
            struct
            {
                /// X coordinate.
                double x;
                /// Y coordinate.
                double y;
            };
        };

        Double2() noexcept
            : x(0.0)
            , y(0.0)
        {
        }

        constexpr Double2(double x_, double y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        /// Construct from a float array.
        explicit Double2(_In_reads_(2) const double* data)
            : x(data[0])
            , y(data[1])
        {
        }

        Double2(const Double2&) = default;
        Double2& operator=(const Double2&) = default;
        Double2(Double2&&) = default;
        Double2& operator=(Double2&&) = default;

        /// Test for equality with another vector.
        bool operator == (const Double2& rhs) const { return x == rhs.x && y == rhs.y; }
        /// Test for inequality with another vector.
        bool operator != (const Double2& rhs) const { return !(*this == rhs); }

        /// Zero vector.
        static const Double2 Zero;
        /// (1,1) vector.
        static const Double2 One;
        /// (1,0) vector.
        static const Double2 UnitX;
        /// (0,1) vector.
        static const Double2 UnitY;
    };

    /// 2D Vector; 16 bit floating point components
    struct ALIMER_API Half2 final
    {
    public:
        union
        {
            struct
            {
                half x;
                half y;
            };
            uint32_t packedValue;
        };

        Half2() = default;

        Half2(const Half2&) = default;
        Half2& operator=(const Half2&) = default;

        Half2(Half2&&) = default;
        Half2& operator=(Half2&&) = default;

        explicit constexpr Half2(uint32_t packed) noexcept
            : packedValue(packed)
        {
        }

        constexpr Half2(half x_, half y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        explicit Half2(_In_reads_(2) const half* pArray) noexcept
            : x(pArray[0])
            , y(pArray[1])
        {
        }

        Half2(float x, float y) noexcept;
        explicit Half2(_In_reads_(2) const float* floatArray) noexcept;
        explicit Half2(const Vector2& vector) noexcept;

        Half2& operator= (uint32_t packed) noexcept
        {
            packedValue = packed;
            return *this;
        }

        Half2& operator= (const Vector2& vector) noexcept
        {
            x = FloatToHalf(vector.x);
            y = FloatToHalf(vector.y);
            return *this;
        }

        Vector2 ToVector2() const;
    };

    /// 2D Vector; 16 bit unsigned integer components
    struct ALIMER_API UShort2 final
    {
        union
        {
            struct
            {
                uint16_t x;
                uint16_t y;
            };
            uint32_t packedValue;
        };

        UShort2() = default;

        UShort2(const UShort2&) = default;
        UShort2& operator=(const UShort2&) = default;

        UShort2(UShort2&&) = default;
        UShort2& operator=(UShort2&&) = default;

        explicit constexpr UShort2(uint32_t packed) noexcept
            : packedValue(packed)
        {
        }

        constexpr UShort2(uint16_t x_, uint16_t y_) noexcept
            : x(x_), y(y_)
        {
        }

        explicit UShort2(_In_reads_(2) const uint16_t* pArray) noexcept
            : x(pArray[0])
            , y(pArray[1])
        {
        }

        UShort2(float _x, float _y) noexcept;
        explicit UShort2(_In_reads_(2) const float* pArray) noexcept;

        UShort2& operator= (uint32_t packed) noexcept
        {
            packedValue = packed; return *this;
        }

        Vector2 ToVector2() const noexcept;
    };

    ALIMER_API UInt2 PackHalf4(float x, float y, float z, float w);
    ALIMER_API UInt2 PackHalf4(const Vector4& value);
    ALIMER_API UInt2 PackHalf4(const Color& value);
}

template<> struct std::hash<Alimer::Vector2>
{
    std::size_t operator()(const Alimer::Vector2& value) const noexcept
    {
        return value.GetHashCode();
    }
};

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif
