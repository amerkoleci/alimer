// Copyright © Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#pragma once

#include "Math/Quaternion.h"
#include "Math/Vector4.h"

namespace Alimer
{
    // Defines a 3x3 Matrix: 32 bit floating point components
    struct ALIMER_API Float3x3
    {
        union
        {
            struct
            {
                float m11, m12, m13;
                float m21, m22, m23;
                float m31, m32, m33;
            };
            float m[3][3];
        };

        Float3x3() noexcept
            : m11(1.0f)
            , m12(0.0f)
            , m13(0.0f)
            , m21(0.0f)
            , m22(1.0f)
            , m23(0.0f)
            , m31(0.0f)
            , m32(0.0f)
            , m33(1.0f)
        {
        }

        Float3x3(const Float3x3&) = default;
        Float3x3& operator=(const Float3x3&) = default;

        Float3x3(Float3x3&&) = default;
        Float3x3& operator=(Float3x3&&) = default;

        constexpr Float3x3(float m00, float m01, float m02,
            float m10, float m11, float m12,
            float m20, float m21, float m22) noexcept
            : m11(m00), m12(m01), m13(m02)
            , m21(m10), m22(m11), m23(m12)
            , m31(m20), m32(m21), m33(m22)
        {
        }

        explicit Float3x3(_In_reads_(9) const float* pData) noexcept;

        float   operator() (size_t row, size_t column) const  noexcept { return m[row][column]; }
        float&  operator() (size_t row, size_t column) noexcept { return m[row][column]; }

        bool operator==(const Float3x3& other) const;
        bool operator!=(const Float3x3& other) const;

        /// Return as string.
        std::string ToString() const;

        // Constants
        static const Float3x3 Zero;
        static const Float3x3 Identity;
    };

    /// Defines a 3x4 column-major Matrix: 32 bit floating point components
    struct ALIMER_API Float3x4
    {
        union
        {
            struct
            {
                float m11, m12, m13, m14;
                float m21, m22, m23, m24;
                float m31, m32, m33, m34;
            };
            float m[3][4];
            float f[12];
        };

        /// Construct an identity matrix.
        Float3x4() noexcept
            : m11(1.0f), m12(0.0f), m13(0.0f) , m14(0.0f)
            , m21(0.0f), m22(1.0f), m23(0.0f), m24(0.0f)
            , m31(0.0f), m32(0.0f), m33(1.0f), m34(0.0f)
        {
        }

        constexpr Float3x4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23) noexcept :
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
            m34(m23)
        {}

        explicit Float3x4(_In_reads_(12) const float* data) noexcept;

        Float3x4(const Float3x4&) = default;
        Float3x4& operator=(const Float3x4&) = default;

        Float3x4(Float3x4&&) = default;
        Float3x4& operator=(Float3x4&&) = default;

        /// Return matrix element.
        float operator()(size_t row, size_t column) const noexcept { return m[row][column]; }

        /// Return matrix element.
        float Element(size_t row, size_t column) const { return m[row][column]; }

        /// Return matrix row.
        Vector4 Row(size_t i) const { return Vector4(Element(i, 0), Element(i, 1), Element(i, 2), Element(i, 3)); }

        /// Return matrix column.
        Vector3 Column(size_t j) const { return Vector3(Element(0, j), Element(1, j), Element(2, j)); }

        // Comparison operators
        bool operator==(const Float3x4& rhs) const noexcept
        {
            const float* leftData = reinterpret_cast<const float*>(this);
            const float* rightData = reinterpret_cast<const float*>(&rhs);

            for (uint32_t i = 0; i < 12; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
        }

        bool operator!=(const Float3x4& rhs) const noexcept
        {
            const float* leftData = reinterpret_cast<const float*>(this);
            const float* rightData = reinterpret_cast<const float*>(&rhs);

            for (uint32_t i = 0; i < 12; ++i)
            {
                if (leftData[i] != rightData[i])
                    return true;
            }

            return true;
        }

        /// Return as string.
        std::string ToString() const;

        // Constants
        static const Float3x4 Zero;
        static const Float3x4 Identity;
    };

    /// Defines a 4x4 Matrix: 32 bit floating point components
    class ALIMER_API Float4x4
    {
    public:
        union
        {
            struct
            {
                float m11, m12, m13, m14;
                float m21, m22, m23, m24;
                float m31, m32, m33, m34;
                float m41, m42, m43, m44;
            };
            float m[4][4];
        };

        /// Construct an identity matrix.
        Float4x4() noexcept :
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

        constexpr Float4x4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33) noexcept :
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

        explicit Float4x4(_In_reads_(16) const float* data) noexcept;

        Float4x4(const Float4x4&) = default;
        Float4x4& operator=(const Float4x4&) = default;

        Float4x4(Float4x4&&) = default;
        Float4x4& operator=(Float4x4&&) = default;

        static Float4x4 CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane) noexcept;
        static void CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane, Float4x4* result);
        static void CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane, Float4x4* result);
        static void CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane, Float4x4* result);

        static Float4x4 CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept;
        static void CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up, Float4x4* result);

        static Float4x4 CreateRotationX(float radians) noexcept;
        static Float4x4 CreateRotationY(float radians) noexcept;
        static Float4x4 CreateRotationZ(float radians) noexcept;

        static Float4x4 Multiply(const Float4x4& value1, const Float4x4& value2) noexcept;
        static void Multiply(const Float4x4& value1, const Float4x4& value2, Float4x4* result);

        float  operator()(size_t row, size_t column) const noexcept { return m[row][column]; }
        float& operator()(size_t row, size_t column) noexcept { return m[row][column]; }

        /// Return matrix row.
        Vector4 Row(size_t i) const { return Vector4(m[i][0], m[i][1], m[i][2], m[i][3]); }

        /// Return matrix column.
        Vector4 Column(size_t j) const { return Vector4(m[0][j], m[1][j], m[2][j], m[3][j]); }

        // Comparison operators
        bool operator==(const Float4x4& rhs) const noexcept
        {
            const float* leftData = reinterpret_cast<const float*>(this);
            const float* rightData = reinterpret_cast<const float*>(&rhs);

            for (uint32_t i = 0; i < 16u; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
        }

        bool operator!=(const Float4x4& rhs) const noexcept
        {
            const float* leftData = reinterpret_cast<const float*>(this);
            const float* rightData = reinterpret_cast<const float*>(&rhs);

            for (uint32_t i = 0; i < 16u; ++i)
            {
                if (leftData[i] != rightData[i])
                    return true;
            }

            return true;
        }

        /// Return as string.
        std::string ToString() const;

        // Constants
        static const Float4x4 Zero;
        static const Float4x4 Identity;
    };

    using float3x4 = Float3x4;
    using float4x4 = Float4x4;
}
