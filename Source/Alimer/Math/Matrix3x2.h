// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

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
    /// Defines a 3x2 Matrix: 32 bit floating point components
    struct ALIMER_API Matrix3x2
    {
    public:
        union
        {
            struct
            {
                float m11, m12;
                float m21, m22;
                float m31, m32;
            };
            float m[3][2];
            float data[6];
        };

        /// Construct an identity matrix.
        Matrix3x2() noexcept
            : m11(1.0f)
            , m12(0.0f)
            , m21(0.0f)
            , m22(1.0f)
            , m31(0.0f)
            , m32(0.0f)
        {
        }

        constexpr Matrix3x2(
            float m11_, float m12_,
            float m21_, float m22_,
            float m31_, float m32_) noexcept
            : m11(m11_), m12(m12_)
            , m21(m21_), m22(m22_)
            , m31(m31_), m32(m32_)
        {
        }

        explicit Matrix3x2(_In_reads_(6) const float* pData) noexcept;

        Matrix3x2(const Matrix3x2&) = default;
        Matrix3x2& operator=(const Matrix3x2&) = default;
        Matrix3x2(Matrix3x2&&) = default;
        Matrix3x2& operator=(Matrix3x2&&) = default;

        /// Sets the values of this matrix to those of the specified matrix.
        void Set(const Matrix3x2& other);

        /// Sets this matrix to the identity matrix.
        void SetIdentity();

        /// Sets all elements of the current matrix to zero.
        void SetZero();

        /// Determines if this matrix is equal to the identity matrix.
        bool IsIdentity() const noexcept;

        /// Calculates the determinant of the current matrix.
        float Determinant() const noexcept;

        static bool Invert(const Matrix3x2& matrix, Matrix3x2& result) noexcept;

        static Matrix3x2 CreateTranslation(const Vector2& position) noexcept;
        static Matrix3x2 CreateTranslation(float x, float y) noexcept;

        static Matrix3x2 CreateRotation(float degrees) noexcept;
        static Matrix3x2 CreateRotation(float degrees, const Vector2& centerPoint) noexcept;

        static Matrix3x2 CreateScale(const Vector2& scale) noexcept;
        static Matrix3x2 CreateScale(float scaleX, float scaleY) noexcept;
        static Matrix3x2 CreateScale(float scale) noexcept;
        static Matrix3x2 CreateScale(const Vector2& scale, const Vector2& centerPoint) noexcept;
        static Matrix3x2 CreateScale(float scale, const Vector2& centerPoint) noexcept;

        static Matrix3x2 CreateSkew(float degreesX, float degreesY) noexcept;
        static Matrix3x2 CreateSkew(float degreesX, float degreesY, const Vector2& centerPoint) noexcept;

        static Matrix3x2 Lerp(const Matrix3x2& matrix1, const Matrix3x2& matrix2, float amount) noexcept;

        bool operator==(const Matrix3x2& other) const noexcept
        {
            return (m11 == other.m11
                && m22 == other.m22
                && m12 == other.m12
                && m21 == other.m21
                && m31 == other.m31
                && m32 == other.m32);
        }

        bool operator!=(const Matrix3x2& other) const noexcept
        {
            return !operator==(other);
        }

        /// Add a matrix.
        Matrix3x2 operator +(const Matrix3x2& rhs) const
        {
            return Matrix3x2(
                m11 + rhs.m11, m12 + rhs.m12,
                m21 + rhs.m21, m22 + rhs.m22,
                m31 + rhs.m31, m32 + rhs.m32
            );
        }

        /// Subtract a matrix.
        Matrix3x2 operator -(const Matrix3x2& rhs) const
        {
            return Matrix3x2(
                m11 - rhs.m11, m12 - rhs.m12,
                m21 - rhs.m21, m22 - rhs.m22,
                m31 - rhs.m31, m32 - rhs.m32
            );
        }

        /// Return negation.
        Matrix3x2 operator - () const
        {
            return Matrix3x2(-m11, -m12, -m21, -m22, -m31, -m32);
        }

        /// Multiply a matrix.
        Matrix3x2 operator *(const Matrix3x2& rhs) const
        {
            return Matrix3x2(
                m11 * rhs.m11 + m12 * rhs.m21,
                m11 * rhs.m12 + m12 * rhs.m22,
                m21 * rhs.m11 + m22 * rhs.m21,
                m21 * rhs.m12 + m22 * rhs.m22,
                m31 * rhs.m11 + m32 * rhs.m21 + rhs.m31,
                m31 * rhs.m12 + m32 * rhs.m22 + rhs.m32
            );
        }

        /// Multiply with a scalar.
        Matrix3x2 operator *(float rhs) const
        {
            return Matrix3x2(
                m11 * rhs,
                m12 * rhs,
                m21 * rhs,
                m22 * rhs,
                m31 * rhs,
                m32 * rhs
            );
        }

        /// Return whether is NaN.
        bool IsNaN() const { return Alimer::IsNaN(m11) || Alimer::IsNaN(m12) || Alimer::IsNaN(m21) || Alimer::IsNaN(m22) || Alimer::IsNaN(m31) || Alimer::IsNaN(m32); }

        /// Return whether any component is Inf.
        bool IsInf() const { return Alimer::IsInf(m11) || Alimer::IsInf(m12) || Alimer::IsInf(m21) || Alimer::IsInf(m22) || Alimer::IsInf(m31) || Alimer::IsInf(m32); }

        /// Return as string.
        std::string ToString() const;

        float  operator()(size_t row, size_t column) const noexcept { return m[row][column]; }
        float& operator()(size_t row, size_t column) noexcept { return m[row][column]; }

        Vector2 GetRow(size_t row) const
        {
            ALIMER_ASSERT(row < 3);

            return Vector2(m[row][0], m[row][1]);
        }

        void SetRow(size_t row, const Vector2& value)
        {
            ALIMER_ASSERT(row < 3);

            m[row][0] = value.x;
            m[row][1] = value.y;
        }

        Vector3 GetColumn(size_t column) const
        {
            ALIMER_ASSERT(column < 2);

            return Vector3(m[0][column], m[1][column], m[2][column]);
        }

        void SetColumn(size_t column, const Vector3& value)
        {
            ALIMER_ASSERT(column < 2);

            m[0][column] = value.x;
            m[1][column] = value.y;
            m[2][column] = value.z;
        }

        Vector2 GetTranslation() const noexcept { return Vector2(m31, m32); }
        void SetTranslation(const Vector2& value) noexcept { m31 = value.x; m32 = value.y; }

        // Constants
        static const Matrix3x2 Zero;
        static const Matrix3x2 Identity;
    };

    /// Multiply a 4x4 matrix with a scalar.
    inline Matrix3x2 operator *(float lhs, const Matrix3x2& rhs) { return rhs * lhs; }
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif
