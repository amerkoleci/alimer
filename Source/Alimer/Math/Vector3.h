// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/Vector2.h"

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
    /// Three-dimensional vector with 32 bit floating point components.
    struct ALIMER_API Vector3 final
    {
    public:
        union
        {
            float data[3];
            struct
            {
                /// X coordinate.
                float x;
                /// Y coordinate.
                float y;
                /// Z coordinate.
                float z;
            };
        };

        Vector3() noexcept
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
        {
        }

        constexpr explicit Vector3(float value) noexcept
            : x(value)
            , y(value)
            , z(value)
        {
        }

        /// Construct from coordinates.
        constexpr Vector3(float x_, float y_, float z_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
        {
        }

        /// Construct from two-dimensional coordinates, with Z coordinate left zero.
        constexpr Vector3(float x_, float y_) noexcept
            : x(x_)
            , y(y_)
            , z(0.0f)
        {
        }

        /// Construct from a two-dimensional vector and the Z coordinate.
        constexpr Vector3(const Vector2& vector, float z_) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(z_)
        {
        }

        constexpr explicit Vector3(const Vector2& vector) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(0.0f)
        {
        }

        /// Construct from a float array.
        explicit Vector3(_In_reads_(3) const float* data) noexcept
            : x(data[0])
            , y(data[1])
            , z(data[2])
        {
        }

        Vector3(const Vector3&) = default;
        Vector3& operator=(const Vector3&) = default;

        Vector3(Vector3&&) = default;
        Vector3& operator=(Vector3&&) = default;

        /// Sets the elements of this vector to the specified values.
        void Set(float x, float y, float z);

        /// Sets the elements of this vector to those in the specified vector.
        void Set(const Vector3& rhs);

        /// Test for equality with another vector without epsilon.
        bool operator == (const Vector3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Vector3& rhs) const noexcept { return x != rhs.x || y != rhs.y || z != rhs.z; }

        /// Return whether all components are zeros.
        [[nodiscard]] bool IsZero() const { return x == 0.f && y == 0.f && z == 0.f; }

        /// Return whether all components are ones.
        [[nodiscard]] bool IsOne() const { return x == 1.f && y == 1.f && z == 1.f; }

        /// Return whether any component is NaN.
        [[nodiscard]] bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z); }

        /// Return whether any component is Inf.
        [[nodiscard]] bool IsInf() const { return Alimer::IsInf(x) || Alimer::IsInf(y) || Alimer::IsInf(z); }

        /// Add a vector.
        Vector3 operator + (const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
        /// Return negation.
        Vector3 operator - () const { return Vector3(-x, -y, -z); }
        /// Subtract a vector.
        Vector3 operator - (const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
        /// Multiply with a scalar.
        Vector3 operator * (float rhs) const { return Vector3(x * rhs, y * rhs, z * rhs); }
        /// Multiply with a vector.
        Vector3 operator * (const Vector3& rhs) const { return Vector3(x * rhs.x, y * rhs.y, z * rhs.z); }
        /// Divide by a scalar.
        Vector3 operator / (float rhs) const { return Vector3(x / rhs, y / rhs, z / rhs); }
        /// Divide by a vector.
        Vector3 operator / (const Vector3& rhs) const { return Vector3(x / rhs.x, y / rhs.y, z / rhs.z); }

        /// Add-assign a vector.
        Vector3& operator += (const Vector3& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector3& operator -= (const Vector3& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector3& operator *= (float rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector3& operator *= (const Vector3& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector3& operator /= (float rhs)
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            z *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector3& operator /= (const Vector3& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        /// Test for equality with another vector with epsilon.
        [[nodiscard]] bool Equals(const Vector3& rhs, float epsilon = M_EPSILON) const noexcept
        {
            return
                MathF::Equals(x, rhs.x, epsilon) &&
                MathF::Equals(y, rhs.y, epsilon) &&
                MathF::Equals(z, rhs.z, epsilon);
        }

        /// Return length.
        [[nodiscard]] float Length() const noexcept { return MathF::Sqrt(x * x + y * y + z * z); }
        /// Return squared length.
        [[nodiscard]] float LengthSquared() const noexcept { return x * x + y * y + z * z; }

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
            }
        }

        /// Return normalized to unit length.
        [[nodiscard]] Vector3 Normalized() const noexcept
        {
            float lenSquared = LengthSquared();
            if (!MathF::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }

            return *this;
        }

        [[nodiscard]] static Vector3 Add(const Vector3& value1, const Vector3& value2) noexcept;
        [[nodiscard]] static Vector3 Subtract(const Vector3& value1, const Vector3& value2) noexcept;
        [[nodiscard]] static Vector3 Multiply(const Vector3& value1, const Vector3& value2) noexcept;
        [[nodiscard]] static Vector3 MultiplyAdd(const Vector3& value1, const Vector3& value2, const Vector3& value3) noexcept;
        [[nodiscard]] static Vector3 MultiplyAdd(const Vector3& value1, float value2, const Vector3& value3) noexcept;
        [[nodiscard]] static Vector3 Negate(const Vector3& value) noexcept;

        [[nodiscard]] static Vector3 Min(const Vector3& value1, const Vector3& value2) noexcept;
        [[nodiscard]] static Vector3 Max(const Vector3& value1, const Vector3& value2) noexcept;
        [[nodiscard]] static Vector3 Round(const Vector3& value) noexcept;
        [[nodiscard]] static Vector3 Truncate(const Vector3& value) noexcept;
        [[nodiscard]] static Vector3 Floor(const Vector3& value) noexcept;
        [[nodiscard]] static Vector3 Ceiling(const Vector3& value) noexcept;
        [[nodiscard]] static Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max) noexcept;
        [[nodiscard]] static Vector3 Saturate(const Vector3& vector) noexcept;

        /// Calculate dot product.
        [[nodiscard]] static float Dot(const Vector3& v1, const Vector3& v2) noexcept;

        /// Calculate cross product.
        [[nodiscard]] static Vector3 Cross(const Vector3& v1, const Vector3& v2) noexcept;

        /// Return normalized to unit length.
        [[nodiscard]] static Vector3 Normalize(const Vector3& vector) noexcept;

        [[nodiscard]] static float Distance(const Vector3& v1, const Vector3& v2) noexcept;
        [[nodiscard]] static float DistanceSquared(const Vector3& v1, const Vector3& v2) noexcept;

        [[nodiscard]] static Vector3 Reflect(const Vector3& vector, const Vector3& normal) noexcept;

        [[nodiscard]] static Vector3 Lerp(const Vector3& value1, const Vector3& value2, float amount) noexcept;

        [[nodiscard]] static Vector3 Transform(const Vector3& position, const Matrix4x4& matrix) noexcept;
        [[nodiscard]] static Vector3 Transform(const Vector3& value, const Quaternion& rotation) noexcept;
        [[nodiscard]] static Vector3 TransformNormal(const Vector3& value, const Matrix4x4& matrix) noexcept;

        /// Return hash value of the vector.
        [[nodiscard]] size_t GetHashCode() const;

        /// Parse vector from a string. Return Vector3::Zero on failure.
        [[nodiscard]] static Vector3 Parse(StringView str);

        /// Try parse from a string. Return true on success.
        [[nodiscard]] static bool TryParse(StringView str, Vector3* result);

        /// Return as a two-dimensional vector.
        [[nodiscard]] constexpr Vector2 ToVector2() const noexcept { return Vector2(x, y); }

        /// Return as string.
        [[nodiscard]] std::string ToString() const;

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
        explicit Vector3(simd_float4_param xyz);

        /// Return SIMD vector.
        [[nodiscard]] simd_float4 ToSIMD() const;

        operator simd_float4() const noexcept { return ToSIMD(); }
#endif

        /// Return SIMD vector.
        //float32x4 ToSIMD() const;

        /// Zero vector.
        static const Vector3 Zero;
        /// (1,1,1) vector.
        static const Vector3 One;
        /// (1,0,0) vector.
        static const Vector3 UnitX;
        /// (0,1,0) vector.
        static const Vector3 UnitY;
        /// (0,0,1) vector.
        static const Vector3 UnitZ;
        /// (-1,0,0) vector.
        static const Vector3 Left;
        /// (1,0,0) vector.
        static const Vector3 Right;
        /// (0,1,0) vector.
        static const Vector3 Up;
        /// (0,-1,0) vector.
        static const Vector3 Down;
        /// (0,0,-1) vector designating forward in the default (right-handed coordinate) system.
        static const Vector3 Forward;
        /// (0,0,1) vector designating backward in the default (right-handed coordinate) system.
        static const Vector3 Backward;
    };

    /// Three-dimensional vector with 32 bit signed integer components.
    struct ALIMER_API Int3
    {
    public:
        union
        {
            int32_t data[3];
            struct
            {
                /// X coordinate.
                int32_t x;
                /// Y coordinate.
                int32_t y;
                /// Z coordinate.
                int32_t z;
            };
        };

        Int3() noexcept
            : x(0)
            , y(0)
            , z(0)
        {
        }

        constexpr Int3(int32_t x_, int32_t y_, int32_t z_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
        {
        }

        /// Construct from a float array.
        explicit Int3(_In_reads_(3) const int32_t* data) noexcept
            : x(data[0])
            , y(data[1])
            , z(data[2])
        {
        }

        Int3(const Int3&) = default;
        Int3& operator=(const Int3&) = default;

        Int3(Int3&&) = default;
        Int3& operator=(Int3&&) = default;

        /// Test for equality with another vector.
        bool operator == (const Int3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        /// Test for inequality with another vector.
        bool operator != (const Int3& rhs) const noexcept { return x != rhs.x || y != rhs.y || z != rhs.z; }

        /// Return as string.
        std::string ToString() const;

        /// Zero vector.
        static const Int3 Zero;
    };

    /// Three-dimensional vector with 32 bit unsigned integer components.
    struct ALIMER_API UInt3
    {
    public:
        union
        {
            uint32_t data[3];
            struct
            {
                /// X coordinate.
                uint32_t x;
                /// Y coordinate.
                uint32_t y;
                /// Z coordinate.
                uint32_t z;
            };
        };

        UInt3() noexcept
            : x(0)
            , y(0)
            , z(0)
        {
        }

        constexpr UInt3(uint32_t x_, uint32_t y_, uint32_t z_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
        {
        }

        /// Construct from a float array.
        explicit UInt3(_In_reads_(3) const uint32_t* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
        {
        }

        UInt3(const UInt3&) = default;
        UInt3& operator=(const UInt3&) = default;

        UInt3(UInt3&&) = default;
        UInt3& operator=(UInt3&&) = default;

        /// Test for equality with another vector.
        bool operator == (const UInt3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        /// Test for inequality with another vector.
        bool operator != (const UInt3& rhs) const noexcept { return x != rhs.x || y != rhs.y || z != rhs.z; }

        /// Zero vector.
        static const UInt3 Zero;
    };

    /// Three-dimensional vector with 64 bit floating point components.
    struct ALIMER_API Double3
    {
    public:
        union
        {
            double data[3];
            struct
            {
                /// X coordinate.
                double x;
                /// Y coordinate.
                double y;
                /// Z coordinate.
                double z;
            };
        };

        Double3() noexcept
            : x(0.0)
            , y(0.0)
            , z(0.0)
        {
        }

        constexpr Double3(double x_, double y_, double z_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
        {
        }

        /// Construct from a float array.
        explicit Double3(_In_reads_(3) const double* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
        {
        }

        Double3(const Double3&) = default;
        Double3& operator=(const Double3&) = default;

        Double3(Double3&&) = default;
        Double3& operator=(Double3&&) = default;

        /// Test for equality with another vector.
        bool operator == (const Double3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        /// Test for inequality with another vector.
        bool operator != (const Double3& rhs) const noexcept { return x != rhs.x || y != rhs.y || z != rhs.z; }

        /// Zero vector.
        static const Double3 Zero;
    };

    /// Multiply Vector3 with a scalar.
    inline Vector3 operator * (float lhs, const Vector3& rhs) { return rhs * lhs; }
}

template<> struct std::hash<Alimer::Vector3>
{
    std::size_t operator()(const Alimer::Vector3& value) const noexcept
    {
        return value.GetHashCode();
    }
};

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif
