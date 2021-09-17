// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Vector2.h"

namespace Alimer
{
    /// Three-dimensional vector.
    class ALIMER_API Vector3
    {
    public:
        union
        {
            struct
            {
                /// X coordinate.
                float x;
                /// Y coordinate.
                float y;
                /// Z coordinate.
                float z;
            };

            float data[3];
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
        Vector3(float x_, float y_, float z_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
        {
        }

        /// Construct from two-dimensional coordinates, with Z coordinate left zero.
        Vector3(float x_, float y_) noexcept
            : x(x_)
            , y(y_)
            , z(0.0f)
        {
        }


        /// Construct from a two-dimensional vector and the Z coordinate.
        Vector3(const Vector2& vector, float z_) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(z_)
        {
        }

        /// Construct from a two-dimensional vector, with Z coordinate left zero.
        Vector3(const Vector2& vector) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(0.0f)
        {
        }

        /// Construct from a float array.
        explicit Vector3(_In_reads_(3) const float* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
        {
        }

        Vector3(const Vector3&) = default;
        Vector3& operator=(const Vector3&) = default;
        Vector3(Vector3&&) = default;
        Vector3& operator=(Vector3&&) = default;

        /// Test for equality with another vector without epsilon.
        bool operator == (const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Vector3& rhs) const { return !(*this == rhs); }

        static Vector3 Add(const Vector3& value1, const Vector3& value2) noexcept;
        static Vector3 Subtract(const Vector3& value1, const Vector3& value2) noexcept;
        static Vector3 Multiply(const Vector3& value1, const Vector3& value2) noexcept;
        static Vector3 Cross(const Vector3& value1, const Vector3& value2) noexcept;

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

        /// Normalize to unit length.
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x *= invLen;
                y *= invLen;
                z *= invLen;
            }
        }

        /// Return length.
        float Length() const { return sqrtf(x * x + y * y + z * z); }
        /// Return squared length.
        float LengthSquared() const { return x * x + y * y + z * z; }
        /// Calculate dot product.
        float DotProduct(const Vector3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
        /// Calculate absolute dot product.
        float AbsDotProduct(const Vector3& rhs) const { return Alimer::Abs(x * rhs.x) + Alimer::Abs(y * rhs.y) + Alimer::Abs(z * rhs.z); }

        /// Calculate cross product.
        Vector3 CrossProduct(const Vector3& rhs) const
        {
            return Vector3(
                y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x
            );
        }

        /// Return absolute vector.
        Vector3 Abs() const { return Vector3(Alimer::Abs(x), Alimer::Abs(y), Alimer::Abs(z)); }
        /// Linear interpolation with another vector.
        Vector3 Lerp(const Vector3& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }
        /// Test for equality with another vector with epsilon.
        bool Equals(const Vector3& rhs) const { return Alimer::Equals(x, rhs.x) && Alimer::Equals(y, rhs.y) && Alimer::Equals(z, rhs.z); }
        /// Return the angle between this vector and another vector in degrees.
        //float Angle(const Vector3& rhs) const { return Alimer::Acos(DotProduct(rhs) / (Length() * rhs.Length())); }
        /// Return whether is NaN.
        bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z); }

        /// Return normalized to unit length.
        Vector3 Normalized() const
        {
            float lenSquared = LengthSquared();
            if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }
            return *this;
        }

        /// Return as string.
        std::string ToString() const;

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
        /// (0,1,0) vector.
        static const Vector3 Up;
        /// (0,-1,0) vector.
        static const Vector3 Down;
        /// (1,0,0) vector.
        static const Vector3 Right;
        /// (-1,0,0) vector.
        static const Vector3 Left;
        /// (0,0,-1) vector (right handed).
        static const Vector3 Forward;
        /// (0,0,1) vector (right handed).
        static const Vector3 Backward;
    };

    /// Three-dimensional vector with 32 bit signed integer components.
    struct ALIMER_API Int3
    {
        /// X coordinate.
        int32_t x;
        /// Y coordinate.
        int32_t y;
        /// Z coordinate.
        int32_t z;

        Int3() = default;

        Int3(const Int3&) = default;
        Int3& operator=(const Int3&) = default;

        Int3(Int3&&) = default;
        Int3& operator=(Int3&&) = default;

        constexpr Int3(int32_t x_, int32_t y_, int32_t z_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
        {
        }

        /// Construct from a float array.
        explicit Int3(_In_reads_(3) const int32_t* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
        {
        }
    };

    /// Three-dimensional vector with 32 bit unsigned integer components.
    struct ALIMER_API UInt3
    {
        /// X coordinate.
        uint32_t x;
        /// Y coordinate.
        uint32_t y;
        /// Z coordinate.
        uint32_t z;

        UInt3() = default;

        UInt3(const UInt3&) = default;
        UInt3& operator=(const UInt3&) = default;

        UInt3(UInt3&&) = default;
        UInt3& operator=(UInt3&&) = default;

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
    };

    /// Multiply Vector3 with a scalar.
    inline Vector3 operator * (float lhs, const Vector3& rhs) { return rhs * lhs; }
}

namespace std
{
    template <> struct hash<Alimer::Vector3>
    {
        size_t operator()(const Alimer::Vector3& value) const noexcept
        {
            size_t h = 0;
            Alimer::HashCombine(h, value.x);
            Alimer::HashCombine(h, value.y);
            Alimer::HashCombine(h, value.z);
            return h;
        }
    };

    template <> struct hash<Alimer::Int3>
    {
        size_t operator()(const Alimer::Int3& value) const noexcept
        {
            size_t h = 0;
            Alimer::HashCombine(h, value.x);
            Alimer::HashCombine(h, value.y);
            Alimer::HashCombine(h, value.z);
            return h;
        }
    };

    template <> struct hash<Alimer::UInt3>
    {
        size_t operator()(const Alimer::UInt3& value) const noexcept
        {
            size_t h = 0;
            Alimer::HashCombine(h, value.x);
            Alimer::HashCombine(h, value.y);
            Alimer::HashCombine(h, value.z);
            return h;
        }
    };
}

