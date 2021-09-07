// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Vector4.h"

namespace Alimer
{
    /// Represents a vector that is used to encode three-dimensional physical rotations.
    class ALIMER_API Quaternion
    {
    public:
        /// The X value of the vector component of the quaternion.
        float x;
        /// The Y value of the vector component of the quaternion.
        float y;
        /// The Z value of the vector component of the quaternion.
        float z;
        /// The W value of the vector component of the quaternion.
        float w;

        Quaternion() noexcept
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
            , w(1.0f)
        {
        }

        Quaternion(const Quaternion&) = default;
        Quaternion& operator=(const Quaternion&) = default;
        Quaternion(Quaternion&&) = default;
        Quaternion& operator=(Quaternion&&) = default;

        /// Construct from coordinates.
        constexpr Quaternion(float x_, float y_, float z_, float w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        /// Construct from a 3-dimensional vector and the W coordinate.
        Quaternion(const Vector3& vector, float scalar) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(vector.z)
            , w(scalar)
        {
        }

        explicit Quaternion(const Vector4& vector) noexcept
            : x(vector.x)
            , y(vector.y)
            , z(vector.z)
            , w(vector.w)
        {
        }

        /// Construct from a float array.
        explicit Quaternion(_In_reads_(4) const float* data)
            : x(data[0])
            , y(data[1])
            , z(data[2])
            , w(data[3])
        {
        }

        /// Test for equality with another vector without epsilon.
        bool operator == (const Quaternion& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Quaternion& rhs) const { return !(*this == rhs); }
        /// Add a quaternion.
        Quaternion operator + (const Quaternion& rhs) const { return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
        /// Return negation.
        Quaternion operator - () const { return Quaternion(-x, -y, -z, -w); }
        /// Subtract a quaternion.
        Quaternion operator - (const Quaternion& rhs) const { return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
        /// Multiply with a scalar.
        Quaternion operator * (float rhs) const { return Quaternion(x * rhs, y * rhs, z * rhs, w * rhs); }
        /// Multiply with a quaternion.
        Quaternion operator * (const Quaternion& rhs) const { return Quaternion(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
        /// Divide by a scalar.
        Quaternion operator / (float rhs) const { return Quaternion(x / rhs, y / rhs, z / rhs, w / rhs); }
        /// Divide by a quaternion.
        Quaternion operator / (const Quaternion& rhs) const { return Quaternion(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

        /// Add-assign a quaternion.
        Quaternion& operator += (const Quaternion& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }

        /// Subtract-assign a quaternion.
        Quaternion& operator -= (const Quaternion& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }

        /// Multiply-assign a scalar.
        Quaternion& operator *= (float rhs)
        {
            x *= rhs;
            y *= rhs;
            z *= rhs;
            w *= rhs;
            return *this;
        }

        /// Multiply-assign a quaternion.
        Quaternion& operator *= (const Quaternion& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }

        /// Divide-assign a scalar.
        Quaternion& operator /= (float rhs)
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            z *= invRhs;
            w *= invRhs;
            return *this;
        }

        /// Divide-assign a quaternion.
        Quaternion& operator /= (const Quaternion& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        /// Test for equality with another vector with epsilon.
        bool Equals(const Quaternion& rhs) const { return Alimer::Equals(x, rhs.x) && Alimer::Equals(y, rhs.y) && Alimer::Equals(z, rhs.z) && Alimer::Equals(w, rhs.w); }
        /// Return whether is NaN.
        bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z) || Alimer::IsNaN(w); }

        /// Return float data.
        const float* Data() const { return &x; }

        /// Return as string.
        std::string ToString() const;

        /// Return hash value of the quaternion.
        size_t ToHash() const;

        /// (0, 0, 0, 0) quaternion.
        static const Quaternion Zero;
        /// (1, 1, 1, 1) quaternion.
        static const Quaternion One;
        /// (0, 0, 0, 1) quaternion.
        static const Quaternion Identity;
    };

    /// Multiply Quaternion with a scalar.
    inline Quaternion operator * (float lhs, const Quaternion& rhs) { return rhs * lhs; }
}

namespace std
{
    template <>
    struct hash<Alimer::Quaternion> {
        size_t operator()(const Alimer::Quaternion& value) const noexcept {
            size_t hash = 0;
            Alimer::HashCombine(hash, value.x, value.y, value.z, value.w);
            return hash;
        }
    };
}
