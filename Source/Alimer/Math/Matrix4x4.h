// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#pragma once

#include "Alimer/Math/Quaternion.h"
#include "Alimer/Math/Vector4.h"

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
    /// Defines a 4x4 Matrix: 32 bit floating point components
    struct ALIMER_API Matrix4x4
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
            float data[16];
        };

        /// Construct an identity matrix.
        Matrix4x4() noexcept
            : m11(1.0f)
            , m12(0.0f)
            , m13(0.0f)
            , m14(0.0f)
            , m21(0.0f)
            , m22(1.0f)
            , m23(0.0f)
            , m24(0.0f)
            , m31(0.0f)
            , m32(0.0f)
            , m33(1.0f)
            , m34(0.0f)
            , m41(0.0f)
            , m42(0.0f)
            , m43(0.0f)
            , m44(1.0f)
        {
        }

        constexpr Matrix4x4(
            float m11_, float m12_, float m13_, float m14_,
            float m21_, float m22_, float m23_, float m24_,
            float m31_, float m32_, float m33_, float m34_,
            float m41_, float m42_, float m43_, float m44_) noexcept
            : m11(m11_), m12(m12_), m13(m13_), m14(m14_)
            , m21(m21_), m22(m22_), m23(m23_), m24(m24_)
            , m31(m31_), m32(m32_), m33(m33_), m34(m34_)
            , m41(m41_), m42(m42_), m43(m43_), m44(m44_)
        {
        }

        explicit Matrix4x4(const Vector3& r0, const Vector3& r1, const Vector3& r2) noexcept
            : m11(r0.x), m12(r0.y), m13(r0.z), m14(0)
            , m21(r1.x), m22(r1.y), m23(r1.z), m24(0)
            , m31(r2.x), m32(r2.y), m33(r2.z), m34(0)
            , m41(0), m42(0), m43(0), m44(1.0f)
        {
        }

        explicit Matrix4x4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3) noexcept
            : m11(r0.x), m12(r0.y), m13(r0.z), m14(r0.w)
            , m21(r1.x), m22(r1.y), m23(r1.z), m24(r1.w)
            , m31(r2.x), m32(r2.y), m33(r2.z), m34(r2.w)
            , m41(r3.x), m42(r3.y), m43(r3.z), m44(r3.w)
        {
        }

        explicit Matrix4x4(_In_reads_(16) const float* data) noexcept;

        Matrix4x4(const Matrix4x4&) = default;
        Matrix4x4& operator=(const Matrix4x4&) = default;

        Matrix4x4(Matrix4x4&&) = default;
        Matrix4x4& operator=(Matrix4x4&&) = default;

        /// Sets the values of this matrix to those of the specified matrix.
        void Set(const Matrix4x4& other);

        /// Sets this matrix to the identity matrix.
        void SetIdentity();

        /// Sets all elements of the current matrix to zero.
        void SetZero();

        /// Calculates the determinant of the current matrix.
        float GetDeterminant() const noexcept;

        /// Computes rotation about y-axis (y), then x-axis (x), then z-axis (z) in degrees.
        Vector3 ToEuler() const noexcept;

        bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const;

        [[nodiscard]] static Matrix4x4 CreateTranslation(const Vector3& position) noexcept;
        [[nodiscard]] static Matrix4x4 CreateTranslation(float x, float y, float z) noexcept;

        [[nodiscard]] static Matrix4x4 CreateRotationX(float degrees) noexcept;
        [[nodiscard]] static Matrix4x4 CreateRotationY(float degrees) noexcept;
        [[nodiscard]] static Matrix4x4 CreateRotationZ(float degrees) noexcept;

        [[nodiscard]] static Matrix4x4 CreateScale(const Vector3& scale) noexcept;
        [[nodiscard]] static Matrix4x4 CreateScale(float scaleX, float scaleY, float scaleZ) noexcept;
        [[nodiscard]] static Matrix4x4 CreateScale(float scale) noexcept;
        [[nodiscard]] static Matrix4x4 CreateScale(const Vector3& scale, const Vector3& centerPoint) noexcept;
        [[nodiscard]] static Matrix4x4 CreateScale(float scale, const Vector3& centerPoint) noexcept;

        [[nodiscard]] static Matrix4x4 CreateFromAxisAngle(const Vector3& axis, float degrees) noexcept;

        [[nodiscard]] static Matrix4x4 CreatePerspectiveInfiniteReverseZ(float fieldOfView, float aspectRatio, float zNearPlane) noexcept;
        [[nodiscard]] static Matrix4x4 CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane) noexcept;
        [[nodiscard]] static Matrix4x4 CreatePerspective(float width, float height, float zNearPlane, float zFarPlane) noexcept;
        [[nodiscard]] static Matrix4x4 CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

        [[nodiscard]] static Matrix4x4 CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
        [[nodiscard]] static Matrix4x4 CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

        [[nodiscard]] static Matrix4x4 CreateLookToLH(const Vector3& cameraPosition, const Vector3& cameraDirection, const Vector3& cameraUpVector) noexcept;
        [[nodiscard]] static Matrix4x4 CreateLookTo(const Vector3& cameraPosition, const Vector3& cameraDirection, const Vector3& cameraUpVector) noexcept;
        [[nodiscard]] static Matrix4x4 CreateLookAt(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector) noexcept;

        [[nodiscard]] static Matrix4x4 CreateFromQuaternion(const Quaternion& rotation) noexcept;

        /// Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
        [[nodiscard]] static Matrix4x4 CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

        [[nodiscard]] static Matrix4x4 CreateViewport(float x, float y, float width, float height, float minDepth, float maxDepth) noexcept;

        static bool Invert(const Matrix4x4& matrix, Matrix4x4* result) noexcept;

        [[nodiscard]] static Matrix4x4 Transpose(const Matrix4x4& value);
        static void Transpose(const Matrix4x4& value, Matrix4x4& result);

        static void Lerp(const Matrix4x4& matrix1, const Matrix4x4& matrix2, float amount, Matrix4x4& result) noexcept;
        [[nodiscard]] static Matrix4x4 Lerp(const Matrix4x4& matrix1, const Matrix4x4& matrix2, float amount) noexcept;

        static void Transform(const Matrix4x4& matrix, const Quaternion& rotation, Matrix4x4& result) noexcept;
        [[nodiscard]] static Matrix4x4 Transform(const Matrix4x4& matrix, const Quaternion& rotation) noexcept;

        // Properties
        [[nodiscard]] Vector3 GetUp() const noexcept { return Vector3(m21, m22, m23); }
        void SetUp(const Vector3& value) noexcept { m21 = value.x; m22 = value.y; m23 = value.z; }

        [[nodiscard]] Vector3 GetDown() const  noexcept { return Vector3(-m21, -m22, -m23); }
        void SetDown(const Vector3& v) noexcept { m21 = -v.x; m22 = -v.y; m23 = -v.z; }

        [[nodiscard]] Vector3 GetRight() const noexcept { return Vector3(m11, m12, m13); }
        void GetRight(const Vector3& v) noexcept { m11 = v.x; m12 = v.y; m13 = v.z; }

        [[nodiscard]] Vector3 GetLeft() const noexcept { return Vector3(-m11, -m12, -m13); }
        void GetLeft(const Vector3& v) noexcept { m11 = -v.x; m12 = -v.y; m13 = -v.z; }

        /// Matrix forward in the default (right-handed coordinate) system.
        [[nodiscard]] Vector3 GetForward() const noexcept { return Vector3(-m31, -m32, -m33); }
        void SetForward(const Vector3& v) noexcept { m31 = -v.x; m32 = -v.y; m33 = -v.z; }

        /// Matrix backward in the default (right-handed coordinate) system.
        [[nodiscard]] Vector3 GetBackward() const noexcept { return Vector3(m31, m32, m33); }
        void SetBackward(const Vector3& v) noexcept { m31 = v.x; m32 = v.y; m33 = v.z; }

        [[nodiscard]] Vector3 GetTranslation() const noexcept { return Vector3(m41, m42, m43); }
        void SetTranslation(const Vector3& value) noexcept { m41 = value.x; m42 = value.y; m43 = value.z; }

        float  operator()(size_t row, size_t column) const noexcept { return m[row][column]; }
        float& operator()(size_t row, size_t column) noexcept { return m[row][column]; }

        /// Return matrix row.
        Vector4 GetRow(size_t row) const
        {
            ALIMER_ASSERT(row < 4);

            return Vector4(m[row][0], m[row][1], m[row][2], m[row][3]);
        }

        /// Sets matrix row.
        void SetRow(size_t row, const Vector4& value)
        {
            ALIMER_ASSERT(row < 3);

            m[row][0] = value.x;
            m[row][1] = value.y;
            m[row][2] = value.z;
            m[row][3] = value.w;
        }

        /// Return matrix column.
        Vector4 GetColumn(size_t column) const
        {
            ALIMER_ASSERT(column < 4);

            return Vector4(m[0][column], m[1][column], m[2][column], m[3][column]);
        }

        void SetColumn(size_t column, const Vector4& value)
        {
            ALIMER_ASSERT(column < 3);

            m[0][column] = value.x;
            m[1][column] = value.y;
            m[2][column] = value.z;
            m[3][column] = value.w;
        }

        /// Return whether any element is NaN.
        bool IsNaN() const
        {
            for (uint32_t i = 0; i < 16; ++i)
            {
                if (Alimer::IsNaN(data[i]))
                    return true;
            }

            return false;
        }

        /// Return whether any element is Inf.
        bool IsInf() const
        {
            for (uint32_t i = 0; i < 16; ++i)
            {
                if (Alimer::IsInf(data[i]))
                    return true;
            }

            return false;
        }

        /// Test for equality with another matrix with epsilon.
        bool Equals(const Matrix4x4& rhs, float epsilon = M_EPSILON) const
        {
            for (uint32_t i = 0; i < 16; ++i)
            {
                if (!MathF::Equals(data[i], rhs.data[i], epsilon))
                {
                    return false;
                }
            }

            return true;
        }

        // Comparison operators
        bool operator==(const Matrix4x4& other) const noexcept
        {
            for (uint32_t i = 0; i < 16; i++)
            {
                if (!MathF::NearEqual(other.data[i], data[i], M_EPSILON))
                    return false;
            }
            return true;
        }

        bool operator!=(const Matrix4x4& other) const noexcept
        {
            return !operator==(other);
        }

        /// Multiply a matrix.
        Matrix4x4 operator *(const Matrix4x4& rhs) const noexcept;

        /// Multiply with a scalar.
        Matrix4x4 operator *(float scalar) const noexcept;

        /// Return as string.
        std::string ToString() const;

        // Constants
        static const Matrix4x4 Zero;
        static const Matrix4x4 Identity;
    };

    /// Defines a 4x4 Matrix: 32 bit floating point components aligned on a 16 byte boundary
    struct alignas(16) Matrix4x4A : public Matrix4x4
    {
        using Matrix4x4::Matrix4x4;
    };

    /// Multiply a 4x4 matrix with a scalar.
    inline Matrix4x4 operator *(float lhs, const Matrix4x4& rhs) { return rhs * lhs; }
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif

