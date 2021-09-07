// Copyright © Amer Koleci.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#pragma once

#include "Math/Quaternion.h"
#include "Math/Vector4.h"

namespace Alimer
{
    /// Defines a 4x4 floating-point matrix.
    class ALIMER_API Matrix4x4
    {
    public:
        union
        {
            float m[4][4];
            struct
            {
                float m11, m12, m13, m14;
                float m21, m22, m23, m24;
                float m31, m32, m33, m34;
                float m41, m42, m43, m44;
            };
        };

        /// Construct an identity matrix.
        Matrix4x4() noexcept :
            m11(1.0f),
            m12(0.0f),
            m13(0.0f),
            m14(0.0f),
            m21(0.0f),
            m22(1.0f),
            m23(0.0f),
            m24(0.0f),
            m31(0.0f),
            m32(0.0f),
            m33(1.0f),
            m34(0.0f),
            m41(0.0f),
            m42(0.0f),
            m43(0.0f),
            m44(1.0f)
        {}

        constexpr Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21,
            float m22, float m23, float m30, float m31, float m32, float m33) noexcept :
            m11(m00),
            m12(m01),
            m13(m02),
            m14(m03),
            m21(m10),
            m22(m11),
            m23(m12),
            m24(m13),
            m31(m20),
            m32(m21),
            m33(m22),
            m34(m23),
            m41(m30),
            m42(m31),
            m43(m32),
            m44(m33)
        {}

        explicit Matrix4x4(_In_reads_(16) const float* data) noexcept;

        Matrix4x4(const Matrix4x4&) = default;
        Matrix4x4& operator=(const Matrix4x4&) = default;

        Matrix4x4(Matrix4x4&&) = default;
        Matrix4x4& operator=(Matrix4x4&&) = default;

        static Matrix4x4 CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane) noexcept;
        static void CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane, Matrix4x4* result);
        static void CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane, Matrix4x4* result);
        static void CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane, Matrix4x4* result);

        static Matrix4x4 CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept;
        static void CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up, Matrix4x4* result);

        static Matrix4x4 CreateRotationX(float radians) noexcept;
        static Matrix4x4 CreateRotationY(float radians) noexcept;
        static Matrix4x4 CreateRotationZ(float radians) noexcept;

        static Matrix4x4 Multiply(const Matrix4x4& value1, const Matrix4x4& value2) noexcept;
        static void Multiply(const Matrix4x4& value1, const Matrix4x4& value2, Matrix4x4* result);

        float  operator()(size_t row, size_t column) const noexcept { return m[row][column]; }
        float& operator()(size_t row, size_t column) noexcept { return m[row][column]; }

        /// Return matrix row.
        Vector4 Row(size_t i) const { return Vector4(m[i][0], m[i][1], m[i][2], m[i][3]); }

        /// Return matrix column.
        Vector4 Column(size_t j) const { return Vector4(m[0][j], m[1][j], m[2][j], m[3][j]); }

        // Comparison operators
        bool operator==(const Matrix4x4& rhs) const noexcept
        {
            const float* leftData = Data();
            const float* rightData = rhs.Data();

            for (uint32_t i = 0; i < 16u; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
        }

        bool operator!=(const Matrix4x4& rhs) const noexcept { return !(*this == rhs); }

        /// Return float data.
        const float* Data() const { return &m11; }

        /// Return as string.
        std::string ToString() const;

        /// Return hash value of the matrix.
        size_t ToHash() const;

        // Constants
        static const Matrix4x4 Zero;
        static const Matrix4x4 Identity;
    };
}

namespace std
{
    template <>
    struct hash<Alimer::Matrix4x4> {
        size_t operator()(const Alimer::Matrix4x4& value) const noexcept {
            return value.ToHash();
        }
    };
}
