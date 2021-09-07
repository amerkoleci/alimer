// Copyright © Amer Koleci.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#pragma once

#include "Math/Quaternion.h"
#include "Math/Vector4.h"

namespace Alimer
{
    /// Defines a 3x4 floating-point matrix.
    class ALIMER_API Matrix3x4
    {
    public:
        struct
        {
            float m11, m12, m13, m14;
            float m21, m22, m23, m24;
            float m31, m32, m33, m34;
        };

        /// Construct an identity matrix.
        Matrix3x4() noexcept :
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
            m34(0.0f)
        {}

        constexpr Matrix3x4(
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

        explicit Matrix3x4(_In_reads_(12) const float* data) noexcept;

        Matrix3x4(const Matrix3x4&) = default;
        Matrix3x4& operator=(const Matrix3x4&) = default;

        Matrix3x4(Matrix3x4&&) = default;
        Matrix3x4& operator=(Matrix3x4&&) = default;

        /// Return float data.
        const float* Data() const { return &m11; }

        float operator()(size_t row, size_t column) const noexcept { return Data()[row * 4 + column]; }

        /// Return matrix element.
        float Element(size_t row, size_t column) const { return Data()[row * 4 + column]; }

        /// Return matrix row.
        Vector4 Row(size_t i) const { return Vector4(Element(i, 0), Element(i, 1), Element(i, 2), Element(i, 3)); }

        /// Return matrix column.
        Vector3 Column(size_t j) const { return Vector3(Element(0, j), Element(1, j), Element(2, j)); }

        // Comparison operators
        bool operator==(const Matrix3x4& rhs) const noexcept
        {
            const float* leftData = Data();
            const float* rightData = rhs.Data();

            for (uint32_t i = 0; i < 12; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
        }

        bool operator!=(const Matrix3x4& rhs) const noexcept { return !(*this == rhs); }

        /// Return as string.
        std::string ToString() const;

        /// Return hash value of the matrix.
        size_t ToHash() const;

        // Constants
        static const Matrix3x4 Zero;
        static const Matrix3x4 Identity;
    };
}

namespace std
{
    template <>
    struct hash<Alimer::Matrix3x4> {
        size_t operator()(const Alimer::Matrix3x4& value) const noexcept {
            return value.ToHash();
        }
    };
}
