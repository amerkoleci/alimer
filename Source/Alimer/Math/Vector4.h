// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/Vector3.h"

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
    /// Four-dimensional vector.
    struct ALIMER_API Vector4 final
    {
    public:
        union
        {
            float data[4];
            struct
            {
                /// X coordinate.
                float x;
                /// Y coordinate.
                float y;
                /// Z coordinate.
                float z;
                /// W coordinate.
                float w;
            };
        };

        Vector4() noexcept
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
            , w(0.0f)
        {
        }

        constexpr explicit Vector4(float value) noexcept
            : x(value)
            , y(value)
            , z(value)
            , w(value)
        {
        }

        /// Construct from a 3-dimensional vector and the W coordinate.
        constexpr Vector4(const Vector3& vector, float w_) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(vector.z)
            , w(w_)
        {
        }

        /// Construct from coordinates.
        constexpr Vector4(float x_, float y_, float z_, float w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        /// Construct from a float array.
        explicit Vector4(_In_reads_(4) const float* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
            , w(data[3])
        {
        }

        Vector4(const Vector4&) = default;
        Vector4& operator=(const Vector4&) = default;
        Vector4(Vector4&&) = default;
        Vector4& operator=(Vector4&&) = default;

        /// Sets the elements of this vector to the specified values.
        void Set(float x, float y, float z, float w);

        /// Sets the elements of this vector to those in the specified vector.
        void Set(const Vector4& rhs);

        /// Return normalized to unit length.
        [[nodiscard]] static Vector4 Normalize(const Vector4& vector) noexcept;

        [[nodiscard]] static Vector4 Min(const Vector4& value1, const Vector4& value2) noexcept;
        [[nodiscard]] static Vector4 Max(const Vector4& value1, const Vector4& value2) noexcept;
        [[nodiscard]] static Vector4 Round(const Vector4& vector) noexcept;
        [[nodiscard]] static Vector4 Truncate(const Vector4& vector) noexcept;
        [[nodiscard]] static Vector4 Floor(const Vector4& vector) noexcept;
        [[nodiscard]] static Vector4 Ceiling(const Vector4& vector) noexcept;
        [[nodiscard]] static Vector4 Clamp(const Vector4& value, const Vector4& min, const Vector4& max) noexcept;
        [[nodiscard]] static Vector4 Saturate(const Vector4& vector) noexcept;
        [[nodiscard]] static Vector4 Lerp(const Vector4& value1, const Vector4& value2, float amount) noexcept;

        /// Test for equality with another vector without epsilon.
        bool operator == (const Vector4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Vector4& rhs) const { return !(*this == rhs); }
        /// Add a vector.
        Vector4 operator + (const Vector4& rhs) const { return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
        /// Return negation.
        Vector4 operator - () const { return Vector4(-x, -y, -z, -w); }
        /// Subtract a vector.
        Vector4 operator - (const Vector4& rhs) const { return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
        /// Multiply with a scalar.
        Vector4 operator * (float rhs) const { return Vector4(x * rhs, y * rhs, z * rhs, w * rhs); }
        /// Multiply with a vector.
        Vector4 operator * (const Vector4& rhs) const { return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
        /// Divide by a scalar.
        Vector4 operator / (float rhs) const { return Vector4(x / rhs, y / rhs, z / rhs, w / rhs); }
        /// Divide by a vector.
        Vector4 operator / (const Vector4& rhs) const { return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

        /// Add-assign a vector.
        Vector4& operator += (const Vector4& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector4& operator -= (const Vector4& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector4& operator *= (float rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            w *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector4& operator *= (const Vector4& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector4& operator /= (float rhs)
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            z *= invRhs;
            w *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector4& operator /= (const Vector4& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        [[nodiscard]] float Length() const noexcept;
        [[nodiscard]] float LengthSquared() const noexcept;

        /// Normalize to unit length.
        void Normalize() noexcept
        {
            float lenSquared = LengthSquared();
            if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x *= invLen;
                y *= invLen;
                z *= invLen;
                w *= invLen;
            }
        }

        /// Return normalized to unit length.
        [[nodiscard]] Vector4 Normalized() const noexcept
        {
            float lenSquared = LengthSquared();
            if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }

            return *this;
        }

        /// Test for equality with another vector with epsilon.
        [[nodiscard]] bool Equals(const Vector4& rhs, float epsilon = M_EPSILON) const
        {
            return
                MathF::Equals(x, rhs.x, epsilon) &&
                MathF::Equals(y, rhs.y, epsilon) &&
                MathF::Equals(z, rhs.z, epsilon) &&
                MathF::Equals(w, rhs.w, epsilon);
        }

        /// Return whether all components are zeros.
        [[nodiscard]] bool IsZero() const { return x == 0.f && y == 0.f && z == 0.f && w == 0.f; }

        /// Return whether all components are ones.
        [[nodiscard]] bool IsOne() const { return x == 1.f && y == 1.f && z == 1.f && w == 1.f; }

        /// Return whether any component is NaN.
        [[nodiscard]] bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z) || Alimer::IsNaN(w); }

        /// Return whether any component is Inf.
        [[nodiscard]] bool IsInf() const { return Alimer::IsInf(x) || Alimer::IsInf(y) || Alimer::IsInf(z) || Alimer::IsInf(w); }

        /// Return hash value of the vector.
        [[nodiscard]] size_t GetHashCode() const;

        /// Parse vector from a string. Return Vector4::Zero on failure.
        [[nodiscard]] static Vector4 Parse(StringView str);

        /// Parse from a string. Return true on success.
        [[nodiscard]] static bool TryParse(StringView str, Vector4* result);

        /// Return as string.
        [[nodiscard]] std::string ToString() const;

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
        explicit Vector4(simd_float4_param xyzw);

        /// Return SIMD vector.
        [[nodiscard]] simd_float4 ToSIMD() const;

        operator simd_float4() const noexcept { return ToSIMD(); }
#endif

        /// Zero vector.
        static const Vector4 Zero;
        /// (1,1,1) vector.
        static const Vector4 One;
        static const Vector4 UnitX;
        static const Vector4 UnitY;
        static const Vector4 UnitZ;
        static const Vector4 UnitW;
    };

    /// Four-dimensional vector with 32 bit signed integer components.
    struct ALIMER_API Int4
    {
    public:
        union
        {
            int32_t data[4];
            struct
            {
                /// X coordinate.
                int32_t x;
                /// Y coordinate.
                int32_t y;
                /// Z coordinate.
                int32_t z;
                /// W coordinate.
                int32_t w;
            };
        };

        Int4() noexcept
            : x(0)
            , y(0)
            , z(0)
            , w(0)
        {
        }

        constexpr Int4(int32_t x_, int32_t y_, int32_t z_, int32_t w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        /// Construct from a float array.
        explicit Int4(_In_reads_(4) const int32_t* data) noexcept
            : x(data[0])
            , y(data[1])
            , z(data[2])
            , w(data[3])
        {
        }

        Int4(const Int4&) = default;
        Int4& operator=(const Int4&) = default;

        Int4(Int4&&) = default;
        Int4& operator=(Int4&&) = default;

        /// Test for equality with another vector.
        bool operator == (const Int4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        /// Test for inequality with another vector.
        bool operator != (const Int4& rhs) const { return !(*this == rhs); }

        /// Zero vector.
        static const Int4 Zero;
    };

    /// Four-dimensional vector with 32 bit unsigned integer components.
    struct ALIMER_API UInt4
    {
    public:
        union
        {
            uint32_t data[4];
            struct
            {
                /// X coordinate.
                uint32_t x;
                /// Y coordinate.
                uint32_t y;
                /// Z coordinate.
                uint32_t z;
                /// W coordinate.
                uint32_t w;
            };
        };

        UInt4() noexcept
            : x(0)
            , y(0)
            , z(0)
            , w(0)
        {
        }

        constexpr UInt4(uint32_t x_, uint32_t y_, uint32_t z_, uint32_t w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        /// Construct from a float array.
        explicit UInt4(_In_reads_(4) const uint32_t* data) noexcept
            : x(data[0])
            , y(data[1])
            , z(data[2])
            , w(data[3])
        {
        }

        UInt4(const UInt4&) = default;
        UInt4& operator=(const UInt4&) = default;

        UInt4(UInt4&&) = default;
        UInt4& operator=(UInt4&&) = default;

        /// Test for equality with another vector.
        bool operator == (const UInt4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        /// Test for inequality with another vector.
        bool operator != (const UInt4& rhs) const { return !(*this == rhs); }

        /// Zero vector.
        static const UInt4 Zero;
    };

    /// Four-dimensional vector with 64 bit floating point components.
    struct ALIMER_API Double4
    {
    public:
        union
        {
            double data[4];
            struct
            {
                /// X coordinate.
                double x;
                /// Y coordinate.
                double y;
                /// Z coordinate.
                double z;
                /// W coordinate.
                double w;
            };
        };

        Double4() noexcept
            : x(0.0)
            , y(0.0)
            , z(0.0)
            , w(0.0)
        {
        }

        constexpr Double4(double x_, double y_, double z_, double w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        /// Construct from a float array.
        explicit Double4(_In_reads_(4) const double* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
            , w(data[3])
        {
        }

        Double4(const Double4&) = default;
        Double4& operator=(const Double4&) = default;

        Double4(Double4&&) = default;
        Double4& operator=(Double4&&) = default;

        /// Test for equality with another vector.
        bool operator == (const Double4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        /// Test for inequality with another vector.
        bool operator != (const Double4& rhs) const { return !(*this == rhs); }

        /// Add a vector.
        Double4 operator + (const Double4& rhs) const { return Double4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
        /// Return negation.
        Double4 operator - () const { return Double4(-x, -y, -z, -w); }
        /// Subtract a vector.
        Double4 operator - (const Double4& rhs) const { return Double4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
        /// Multiply with a scalar.
        Double4 operator * (double rhs) const { return Double4(x * rhs, y * rhs, z * rhs, w * rhs); }
        /// Multiply with a vector.
        Double4 operator * (const Double4& rhs) const { return Double4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
        /// Divide by a scalar.
        Double4 operator / (double rhs) const { return Double4(x / rhs, y / rhs, z / rhs, w / rhs); }
        /// Divide by a vector.
        Double4 operator / (const Double4& rhs) const { return Double4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

        /// Add-assign a vector.
        Double4& operator += (const Double4& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        /// Subtract-assign a vector.
        Double4& operator -= (const Double4& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        /// Multiply-assign a scalar.
        Double4& operator *= (double rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            w *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Double4& operator *= (const Double4& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }

        /// Divide-assign a scalar.
        Double4& operator /= (double rhs)
        {
            double invRhs = 1.0 / rhs;
            x *= invRhs;
            y *= invRhs;
            z *= invRhs;
            w *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Double4& operator /= (const Double4& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        double Length() const noexcept;
        double LengthSquared() const noexcept;

        /// Zero vector.
        static const Double4 Zero;
    };

    /// 4D Vector; 16 bit floating point components
    struct ALIMER_API Half4
    {
    public:
        union
        {
            struct
            {
                half x;
                half y;
                half z;
                half w;
            };
            uint64_t packedValue;
        };

        Half4() = default;

        Half4(const Half4&) = default;
        Half4& operator=(const Half4&) = default;
        Half4(Half4&&) = default;
        Half4& operator=(Half4&&) = default;

        explicit constexpr Half4(uint64_t packed) noexcept
            : packedValue(packed)
        {
        }

        constexpr Half4(half x_, half y_, half z_, half w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        explicit Half4(_In_reads_(4) const half* pArray) noexcept
            : x(pArray[0])
            , y(pArray[1])
            , z(pArray[2])
            , w(pArray[3])
        {
        }

        Half4(float x, float y, float z, float w) noexcept;
        explicit Half4(_In_reads_(4) const float* floatArray) noexcept;
        explicit Half4(const Vector4& vector) noexcept;

        Half4& operator= (uint64_t packed) noexcept
        {
            packedValue = packed;
            return *this;
        }

        Half4& operator= (const Vector4& vector) noexcept
        {
            x = FloatToHalf(vector.x);
            y = FloatToHalf(vector.y);
            z = FloatToHalf(vector.z);
            w = FloatToHalf(vector.w);
            return *this;
        }

        Vector4 ToVector4() const;
    };

    /// 4D Vector; 16 bit unsigned integer components
    struct UShort4
    {
        union
        {
            struct
            {
                uint16_t x;
                uint16_t y;
                uint16_t z;
                uint16_t w;
            };
            uint64_t packedValue;
        };

        UShort4() = default;

        UShort4(const UShort4&) = default;
        UShort4& operator=(const UShort4&) = default;

        UShort4(UShort4&&) = default;
        UShort4& operator=(UShort4&&) = default;

        explicit constexpr UShort4(uint64_t packedValue_) noexcept
            : packedValue(packedValue_)
        {
        }

        constexpr UShort4(uint16_t x_, uint16_t y_, uint16_t z_, uint16_t w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        explicit UShort4(_In_reads_(4) const uint16_t* pArray) noexcept
            : x(pArray[0])
            , y(pArray[1])
            , z(pArray[2])
            , w(pArray[3])
        {
        }

        UShort4(float x_, float y_, float z_, float w_) noexcept;
        explicit UShort4(_In_reads_(4) const float* pArray) noexcept;

        UShort4& operator= (uint64_t value) noexcept { packedValue = value; return *this; }
    };

    /// Multiply Vector4 with a scalar.
    inline Vector4 operator * (float lhs, const Vector4& rhs) { return rhs * lhs; }

    /// Multiply Double4 with a scalar.
    inline Double4 operator * (double lhs, const Double4& rhs) { return rhs * lhs; }
}

template<> struct std::hash<Alimer::Vector4>
{
    std::size_t operator()(const Alimer::Vector4& value) const noexcept
    {
        return value.GetHashCode();
    }
};

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif
