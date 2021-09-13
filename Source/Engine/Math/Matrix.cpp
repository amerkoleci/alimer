// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Math/Matrix.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    /* Float3x3 */
    const Float3x3 Float3x3::Zero = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };
    const Float3x3 Float3x3::Identity = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    Float3x3::Float3x3(_In_reads_(9) const float* pData) noexcept
    {
        ALIMER_ASSERT(pData != nullptr);

        m11 = pData[0];
        m12 = pData[1];
        m13 = pData[2];

        m21 = pData[3];
        m22 = pData[4];
        m23 = pData[5];

        m31 = pData[6];
        m32 = pData[7];
        m33 = pData[8];
    }

    bool Float3x3::operator==(const Float3x3& rhs) const
    {
        const float* leftData = reinterpret_cast<const float*>(this);
        const float* rightData = reinterpret_cast<const float*>(&rhs);
        for (u32 i = 0; i < 12; ++i)
        {
            if (leftData[i] != rightData[i])
                return false;
        }

        return true;
    }

    bool Float3x3::operator!=(const Float3x3& rhs) const
    {
        const float* leftData = reinterpret_cast<const float*>(this);
        const float* rightData = reinterpret_cast<const float*>(&rhs);
        for (u32 i = 0; i < 12; ++i)
        {
            if (leftData[i] != rightData[i])
                return false;
        }

        return false;
    }

    std::string Float3x3::ToString() const
    {
        return fmt::format("{} {} {} {} {} {} {} {} {}",
            m11, m12, m13,
            m21, m22, m23,
            m31, m32, m33);
    }

    /* Float3x4 */
    const Float3x4 Float3x4::Zero = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };
    const Float3x4 Float3x4::Identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };

    Float3x4::Float3x4(_In_reads_(12) const float* data) noexcept
    {
        ALIMER_ASSERT(data != nullptr);

        m11 = data[0];
        m12 = data[1];
        m13 = data[2];
        m14 = data[3];

        m21 = data[4];
        m22 = data[5];
        m23 = data[6];
        m24 = data[7];

        m31 = data[8];
        m32 = data[9];
        m33 = data[10];
        m34 = data[11];
    }

    std::string Float3x4::ToString() const
    {
        return fmt::format("{} {} {} {} {} {} {} {} {} {} {} {}",
            m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34);
    }

    /* Float4x4 */
    const Float4x4 Float4x4::Zero = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                       0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    const Float4x4 Float4x4::Identity = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    Float4x4::Float4x4(_In_reads_(16) const float* data) noexcept
    {
        ALIMER_ASSERT(data != nullptr);

        m[0][0] = data[0];
        m[0][1] = data[1];
        m[0][2] = data[2];
        m[0][3] = data[3];

        m[1][0] = data[4];
        m[1][1] = data[5];
        m[1][2] = data[6];
        m[1][3] = data[7];

        m[2][0] = data[8];
        m[2][1] = data[9];
        m[2][2] = data[10];
        m[2][3] = data[11];

        m[3][0] = data[12];
        m[3][1] = data[13];
        m[3][2] = data[14];
        m[3][3] = data[15];
    }

    Float4x4 Float4x4::CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane) noexcept
    {
        Float4x4 result;
        CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, zNearPlane, zFarPlane, &result);
        return result;
    }

    void Float4x4::CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane, Float4x4* result)
    {
        ALIMER_ASSERT(result);
        ALIMER_ASSERT(zFarPlane != zNearPlane);

        float yScale = 1.0f / tan(fieldOfView * 0.5f);
        float xScale = yScale / aspectRatio;
        const float negFarRange = IsInf(zFarPlane) ? -1.0f : zFarPlane / (zNearPlane - zFarPlane);

        result->m11 = xScale;
        result->m12 = 0.0f;
        result->m13 = 0.0f;
        result->m14 = 0.0f;

        result->m21 = 0.0f;
        result->m22 = yScale;
        result->m23 = 0.0f;
        result->m24 = 0.0f;

        result->m31 = 0.0f;
        result->m32 = 0.0;
        result->m33 = negFarRange;
        result->m34 = -1.0f;

        result->m41 = 0.0f;
        result->m42 = 0.0;
        result->m43 = zNearPlane * negFarRange;
        result->m44 = 0.0f;
    }

    void Float4x4::CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane, Float4x4* result)
    {
        ALIMER_ASSERT(result);
        result->m11 = 2.0f / width;
        result->m12 = 0.0f;
        result->m13 = 0.0f;
        result->m14 = 0.0f;

        result->m21 = 0.0f;
        result->m22 = 2.0f / height;
        result->m23 = 0.0f;
        result->m24 = 0.0f;

        result->m31 = 0.0f;
        result->m32 = 0.0;
        result->m33 = 1.0f / (zNearPlane - zFarPlane);
        result->m34 = 0.0f;

        result->m41 = 0.0f;
        result->m42 = 0.0;
        result->m43 = zNearPlane / (zNearPlane - zFarPlane);
        result->m44 = 1.0f;
    }

    void Float4x4::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane, Float4x4* result)
    {
        ALIMER_ASSERT(result);
        ALIMER_ASSERT(right != left);
        ALIMER_ASSERT(top != bottom);
        ALIMER_ASSERT(zFarPlane != zNearPlane);

        result->m11 = 2.0f / (right - left);
        result->m12 = result->m13 = result->m14 = 0.0f;

        result->m22 = 2.0f / (top - bottom);
        result->m21 = result->m23 = result->m24 = 0.0f;

        result->m33 = 1.0f / (zNearPlane - zFarPlane);
        result->m31 = result->m32 = result->m34 = 0.0f;

        result->m41 = (left + right) / (left - right);
        result->m42 = (top + bottom) / (bottom - top);
        result->m43 = zNearPlane / (zNearPlane - zFarPlane);
        result->m44 = 1.0f;
    }

    Float4x4 Float4x4::CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept
    {
        Float4x4 result;
        CreateLookAt(position, target, up, &result);
        return result;
    }

    void Float4x4::CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up, Float4x4* result)
    {
        ALIMER_ASSERT(result);

        Vector3 zaxis = position - target;
        zaxis.Normalize();

        Vector3 xaxis = up.CrossProduct(zaxis);
        xaxis.Normalize();
        Vector3 yaxis = zaxis.CrossProduct(xaxis);

        result->m11 = xaxis.x;
        result->m12 = yaxis.x;
        result->m13 = zaxis.x;
        result->m14 = 0.0f;
        result->m21 = xaxis.y;
        result->m22 = yaxis.y;
        result->m23 = zaxis.y;
        result->m24 = 0.0f;
        result->m31 = xaxis.z;
        result->m32 = yaxis.z;
        result->m33 = zaxis.z;
        result->m34 = 0.0f;
        result->m41 = -xaxis.DotProduct(position);
        result->m42 = -yaxis.DotProduct(position);
        result->m43 = -zaxis.DotProduct(position);
        result->m44 = 1.0f;
    }

    Float4x4 Float4x4::CreateRotationX(float radians) noexcept
    {
        float c = (float)cos(radians);
        float s = (float)sin(radians);

        // [  1  0  0  0 ]
        // [  0  c  s  0 ]
        // [  0 -s  c  0 ]
        // [  0  0  0  1 ]
        Float4x4 result;
        result.m11 = 1.0f;
        result.m12 = 0.0f;
        result.m13 = 0.0f;
        result.m14 = 0.0f;
        result.m21 = 0.0f;
        result.m22 = c;
        result.m23 = s;
        result.m24 = 0.0f;
        result.m31 = 0.0f;
        result.m32 = -s;
        result.m33 = c;
        result.m34 = 0.0f;
        result.m41 = 0.0f;
        result.m42 = 0.0f;
        result.m43 = 0.0f;
        result.m44 = 1.0f;

        return result;
    }

    Float4x4 Float4x4::CreateRotationY(float radians) noexcept
    {
        float c = (float)cos(radians);
        float s = (float)sin(radians);

        // [  c  0 -s  0 ]
        // [  0  1  0  0 ]
        // [  s  0  c  0 ]
        // [  0  0  0  1 ]
        Float4x4 result;
        result.m11 = c;
        result.m12 = 0.0f;
        result.m13 = -s;
        result.m14 = 0.0f;
        result.m21 = 0.0f;
        result.m22 = 1.0f;
        result.m23 = 0.0f;
        result.m24 = 0.0f;
        result.m31 = s;
        result.m32 = 0.0f;
        result.m33 = c;
        result.m34 = 0.0f;
        result.m41 = 0.0f;
        result.m42 = 0.0f;
        result.m43 = 0.0f;
        result.m44 = 1.0f;

        return result;
    }

    Float4x4 Float4x4::CreateRotationZ(float radians) noexcept
    {
        const float c = (float)cos(radians);
        const float s = (float)sin(radians);

        // [  c  s  0  0 ]
        // [ -s  c  0  0 ]
        // [  0  0  1  0 ]
        // [  0  0  0  1 ]
        Float4x4 result;
        result.m11 = c;
        result.m12 = s;
        result.m13 = 0.0f;
        result.m14 = 0.0f;
        result.m21 = -s;
        result.m22 = c;
        result.m23 = 0.0f;
        result.m24 = 0.0f;
        result.m31 = 0.0f;
        result.m32 = 0.0f;
        result.m33 = 1.0f;
        result.m34 = 0.0f;
        result.m41 = 0.0f;
        result.m42 = 0.0f;
        result.m43 = 0.0f;
        result.m44 = 1.0f;

        return result;
    }

    Float4x4 Float4x4::Multiply(const Float4x4& value1, const Float4x4& value2) noexcept
    {
        Float4x4 result;
        Float4x4::Multiply(value1, value2, &result);
        return result;
    }

    void Float4x4::Multiply(const Float4x4& value1, const Float4x4& value2, Float4x4* result)
    {
        ALIMER_ASSERT(result);

        // First row
        result->m11 = value1.m11 * value2.m11 + value1.m12 * value2.m21 + value1.m13 * value2.m31 + value1.m14 * value2.m41;
        result->m12 = value1.m11 * value2.m12 + value1.m12 * value2.m22 + value1.m13 * value2.m32 + value1.m14 * value2.m42;
        result->m13 = value1.m11 * value2.m13 + value1.m12 * value2.m23 + value1.m13 * value2.m33 + value1.m14 * value2.m43;
        result->m14 = value1.m11 * value2.m14 + value1.m12 * value2.m24 + value1.m13 * value2.m34 + value1.m14 * value2.m44;

        // Second row
        result->m21 = value1.m21 * value2.m11 + value1.m22 * value2.m21 + value1.m23 * value2.m31 + value1.m24 * value2.m41;
        result->m22 = value1.m21 * value2.m12 + value1.m22 * value2.m22 + value1.m23 * value2.m32 + value1.m24 * value2.m42;
        result->m23 = value1.m21 * value2.m13 + value1.m22 * value2.m23 + value1.m23 * value2.m33 + value1.m24 * value2.m43;
        result->m24 = value1.m21 * value2.m14 + value1.m22 * value2.m24 + value1.m23 * value2.m34 + value1.m24 * value2.m44;

        // Third row
        result->m31 = value1.m31 * value2.m11 + value1.m32 * value2.m21 + value1.m33 * value2.m31 + value1.m34 * value2.m41;
        result->m32 = value1.m31 * value2.m12 + value1.m32 * value2.m22 + value1.m33 * value2.m32 + value1.m34 * value2.m42;
        result->m33 = value1.m31 * value2.m13 + value1.m32 * value2.m23 + value1.m33 * value2.m33 + value1.m34 * value2.m43;
        result->m34 = value1.m31 * value2.m14 + value1.m32 * value2.m24 + value1.m33 * value2.m34 + value1.m34 * value2.m44;

        // Fourth row
        result->m41 = value1.m41 * value2.m11 + value1.m42 * value2.m21 + value1.m43 * value2.m31 + value1.m44 * value2.m41;
        result->m42 = value1.m41 * value2.m12 + value1.m42 * value2.m22 + value1.m43 * value2.m32 + value1.m44 * value2.m42;
        result->m43 = value1.m41 * value2.m13 + value1.m42 * value2.m23 + value1.m43 * value2.m33 + value1.m44 * value2.m43;
        result->m44 = value1.m41 * value2.m14 + value1.m42 * value2.m24 + value1.m43 * value2.m34 + value1.m44 * value2.m44;
    }

    std::string Float4x4::ToString() const
    {
        return fmt::format("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
            m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34,
            m41, m42, m43, m44);
    }
}
