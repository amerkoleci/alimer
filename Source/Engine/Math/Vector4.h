// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Vector3.h"

namespace Alimer
{
    /// Four-dimensional vector.
    class ALIMER_API Vector4
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
                /// W coordinate.
                float w;
            };

            float data[4];
        };

        Vector4() noexcept
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
            , w(0.0f)
        {
        }

        Vector4(const Vector4&) = default;
        Vector4& operator=(const Vector4&) = default;
        Vector4(Vector4&&) = default;
        Vector4& operator=(Vector4&&) = default;

        constexpr explicit Vector4(float value) noexcept
            : x(value)
            , y(value)
            , z(value)
            , w(value)
        {
        }

        /// Construct from a 3-dimensional vector and the W coordinate.
        Vector4(const Vector3& vector, float w_) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(vector.z)
            , w(w_)
        {
        }

        /// Construct from coordinates.
        Vector4(float x_, float y_, float z_, float w_) noexcept
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

        /// Calculate dot product.
        float DotProduct(const Vector4& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; }
        /// Calculate absolute dot product.
        float AbsDotProduct(const Vector4& rhs) const { return Alimer::Abs(x * rhs.x) + Alimer::Abs(y * rhs.y) + Alimer::Abs(z * rhs.z) + Alimer::Abs(w * rhs.w); }
        /// Return absolute vector.
        Vector4 Abs() const { return Vector4(Alimer::Abs(x), Alimer::Abs(y), Alimer::Abs(z), Alimer::Abs(w)); }
        /// Linear interpolation with another vector.
        Vector4 Lerp(const Vector4& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }
        /// Test for equality with another vector with epsilon.
        bool Equals(const Vector4& rhs) const { return Alimer::Equals(x, rhs.x) && Alimer::Equals(y, rhs.y) && Alimer::Equals(z, rhs.z) && Alimer::Equals(w, rhs.w); }
        /// Return whether is NaN.
        bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z) || Alimer::IsNaN(w); }

        /// Return as string.
        std::string ToString() const;

        /// Zero vector.
        static const Vector4 Zero;
        /// (1,1,1) vector.
        static const Vector4 One;
        static const Vector4 UnitX;
        static const Vector4 UnitY;
        static const Vector4 UnitZ;
        static const Vector4 UnitW;
    };

    /// Multiply Vector4 with a scalar.
    inline Vector4 operator * (float lhs, const Vector4& rhs) { return rhs * lhs; }

}
