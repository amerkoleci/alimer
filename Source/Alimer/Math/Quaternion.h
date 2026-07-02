// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Math/Vector4.h"

namespace Alimer
{
    struct Matrix4x4;

    /// Represents a vector that is used to encode three-dimensional physical rotations.
    struct ALIMER_API Quaternion final
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

        Quaternion() noexcept
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
            , w(1.0f)
        {
        }

        /// Construct from coordinates.
        constexpr Quaternion(float x_, float y_, float z_, float w_) noexcept
            : x(x_)
            , y(y_)
            , z(z_)
            , w(w_)
        {
        }

        Quaternion(const Vector3& v, float scalar) noexcept
            : x(v.x)
            , y(v.y)
            , z(v.z)
            , w(scalar)
        {
        }

        explicit Quaternion(const Vector4& v) noexcept
            : x(v.x)
            , y(v.y)
            , z(v.z)
            , w(v.w)
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

        Quaternion(const Quaternion&) = default;
        Quaternion& operator=(const Quaternion&) = default;

        Quaternion(Quaternion&&) = default;
        Quaternion& operator=(Quaternion&&) = default;

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
        Quaternion operator * (float rhs) const
        {
            return Quaternion(x * rhs, y * rhs, z * rhs, w * rhs);
        }

        /// Multiply with a quaternion.
        Quaternion operator * (const Quaternion& rhs) const
        {
            float q1x = x;
            float q1y = y;
            float q1z = z;
            float q1w = w;

            float q2x = rhs.x;
            float q2y = rhs.y;
            float q2z = rhs.z;
            float q2w = rhs.w;

            // cross(av, bv)
            float cx = q1y * q2z - q1z * q2y;
            float cy = q1z * q2x - q1x * q2z;
            float cz = q1x * q2y - q1y * q2x;

            float dot = q1x * q2x + q1y * q2y + q1z * q2z;

            return Quaternion(
                q1x * q2w + q2x * q1w + cx,
                q1y * q2w + q2y * q1w + cy,
                q1z * q2w + q2z * q1w + cz,
                q1w * q2w - dot
            );
        }

        /// Divide by a quaternion.
        Quaternion operator / (const Quaternion& rhs) const
        {
            float q1x = x;
            float q1y = y;
            float q1z = z;
            float q1w = w;

            //-------------------------------------
            // Inverse part.
            float ls = rhs.x * rhs.x + rhs.y * rhs.y + rhs.z * rhs.z + rhs.w * rhs.w;
            float invNorm = 1.0f / ls;

            float q2x = -rhs.x * invNorm;
            float q2y = -rhs.y * invNorm;
            float q2z = -rhs.z * invNorm;
            float q2w = rhs.w * invNorm;

            //-------------------------------------
            // Multiply part.

            // cross(av, bv)
            float cx = q1y * q2z - q1z * q2y;
            float cy = q1z * q2x - q1x * q2z;
            float cz = q1x * q2y - q1y * q2x;

            float dot = q1x * q2x + q1y * q2y + q1z * q2z;

            return Quaternion(
                q1x * q2w + q2x * q1w + cx,
                q1y * q2w + q2y * q1w + cy,
                q1z * q2w + q2z * q1w + cz,
                q1w * q2w - dot
            );
        }

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

        /// Test for equality with another vector with epsilon.
        bool Equals(const Quaternion& rhs, float epsilon = M_EPSILON) const
        {
            return
                MathF::Equals(x, rhs.x, epsilon) &&
                MathF::Equals(y, rhs.y, epsilon) &&
                MathF::Equals(z, rhs.z, epsilon) &&
                MathF::Equals(w, rhs.w, epsilon);
        }

        /// Test for equality with another vector with epsilon.
        bool EqualRotation(const Quaternion& rhs, float epsilon = M_EPSILON) const
        {
            return Equals(rhs, epsilon) || Equals(-rhs, epsilon);
        }

        /// Return whether all components are zeros.
        [[nodiscard]] bool IsZero() const { return x == 0.f && y == 0.f && z == 0.f && w == 0.f; }

        /// Return whether all components are ones.
        [[nodiscard]] bool IsOne() const { return x == 1.f && y == 1.f && z == 1.f && w == 1.f; }

        /// Return whether this quaternion is equal to the identity quaternion.
        [[nodiscard]] bool IsIdentity() const { return x == 0.f && y == 0.f && z == 0.f && w == 1.f; }

        /// Return whether any component is NaN.
        bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y) || Alimer::IsNaN(z) || Alimer::IsNaN(w); }

        /// Return whether any component is Inf.
        bool IsInf() const { return Alimer::IsInf(x) || Alimer::IsInf(y) || Alimer::IsInf(z) || Alimer::IsInf(w); }

        /// Parse quaternion from a string. Return identity quanterion on failure.
        static Quaternion Parse(StringView str);

        /// Try parse from a string. Return true on success.
        static bool TryParse(StringView str, Quaternion* result);

        /// Return as string.
        std::string ToString() const;

        /// Return hash value of the quaternion.
        size_t GetHashCode() const;

#if defined(ALIMER_USE_SSE) || defined(ALIMER_USE_NEON)
        explicit Quaternion(simd_float4_param xyzw);

        /// Return SIMD vector.
        simd_float4 ToSIMD() const;

        operator simd_float4() const noexcept { return ToSIMD(); }
#endif

        // Quaternion operations
        float Length() const noexcept;
        float LengthSquared() const noexcept;

        /// Normalize to unit length.
        void Normalize() noexcept;
        /// Return normalized to unit length.
        Quaternion Normalized() const noexcept;

        /// Return Euler angles in degrees.
        Vector3 EulerAngles() const noexcept;
        /// Return yaw angle in degrees.
        float YawAngle() const noexcept;
        /// Return pitch angle in degrees.
        float PitchAngle() const noexcept;
        /// Return roll angle in degrees.
        float RollAngle() const noexcept;
        /// Return rotation axis.
        Vector3 Axis() const noexcept;
        /// Return rotation angle.
        float Angle() const noexcept;

        static void Lerp(const Quaternion& value1, const Quaternion& value2, float amount, Quaternion& result) noexcept;
        static Quaternion Lerp(const Quaternion& value1, const Quaternion& value2, float amount) noexcept;

        static void Slerp(const Quaternion& q1, const Quaternion& q2, float amount, Quaternion& result) noexcept;
        static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float amount) noexcept;

        static void Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept;
        static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept;

        void Conjugate() noexcept;
        static void Conjugate(const Quaternion& value, Quaternion& result) noexcept;
        static Quaternion Conjugate(const Quaternion& value) noexcept;

        /// Return normalized to unit length.
        static Quaternion Normalize(const Quaternion& quaternion) noexcept;

        /// Define from an axis and angle (in degrees).
        static Quaternion CreateFromAxisAngle(const Vector3& axis, float degrees) noexcept;

        /// Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
        static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

        /// Rotates about y-axis (angles.y), then x-axis (angles.x), then z-axis (angles.z)
        static Quaternion CreateFromYawPitchRoll(const Vector3& angles) noexcept;

        /// Creates a quaternion from the specified rotation matrix.
        static Quaternion CreateFromRotationMatrix(const Matrix4x4& matrix) noexcept;

        /// Creates from Euler angles (in degrees). Equivalent to Y*X*Z.
        static Quaternion CreateFromEulerAngles(float x, float y, float z) noexcept;

        /// Creates from Euler angles (in degrees). Equivalent to Y*X*Z.
        static Quaternion CreateFromEulerAngles(const Vector3& angles) noexcept;

        /// Define from the rotation difference between two direction vectors.
        static Quaternion CreateFromRotationTo(const Vector3& start, const Vector3& end);

        /// Define from orthonormal axes.
        static Quaternion CreateFromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

        static bool CreateFromLookRotation(const Vector3& direction, Quaternion& result);
        static bool CreateFromLookRotation(const Vector3& direction, const Vector3& up, Quaternion& result);

        static Quaternion Inverse(const Quaternion& value) noexcept;

        /// Angle in radians
        static float Angle(const Quaternion& q1, const Quaternion& q2) noexcept;

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
