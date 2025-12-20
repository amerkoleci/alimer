// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Math/Matrix3x2.h"

using namespace Alimer;

/* Matrix3x2 */
const Matrix3x2 Matrix3x2::Zero = {
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
};
const Matrix3x2 Matrix3x2::Identity = {
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
};

Matrix3x2::Matrix3x2(_In_reads_(6) const float* pData) noexcept
{
    ALIMER_ASSERT(pData != nullptr);

    m11 = pData[0];
    m12 = pData[1];
    m21 = pData[2];
    m22 = pData[3];
    m31 = pData[4];
    m32 = pData[5];
}

void Matrix3x2::Set(const Matrix3x2& other)
{
    memcpy(this->data, other.data, sizeof(float) * 6);
}

void Matrix3x2::SetIdentity()
{
    m11 = 1.0f;
    m12 = 0.0f;
    m21 = 0.0f;
    m22 = 1.0f;
    m31 = 0.0f;
    m32 = 0.0f;
}

void Matrix3x2::SetZero()
{
    memset(data, 0, sizeof(Matrix3x2));
}

bool Matrix3x2::IsIdentity() const noexcept
{
    return m11 == 1.f && m12 == 0.f
        && m21 == 0.f && m22 == 1.f
        && m31 == 0.f && m32 == 0.f;
}

float Matrix3x2::Determinant() const noexcept
{
    return (m11 * m22) - (m12 * m21);
}

std::string Matrix3x2::ToString() const
{
    char tempBuffer[kMatrixConversionBufferLength];
    snprintf(tempBuffer, kMatrixConversionBufferLength, "%g %g %g %g %g %g", m11, m12, m21, m22, m31, m32);
    return std::string(tempBuffer);
}

bool Matrix3x2::Invert(const Matrix3x2& matrix, Matrix3x2& result) noexcept
{
    float det = (matrix.m11 * matrix.m22) - (matrix.m21 * matrix.m12);

    if (Alimer::Abs(det) < std::numeric_limits<float>::epsilon())
    {
        constexpr float NaN = std::numeric_limits<float>::quiet_NaN();

        result.m11 = NaN;
        result.m12 = NaN;
        result.m21 = NaN;
        result.m22 = NaN;
        result.m31 = NaN;
        result.m32 = NaN;

        return false;
    }

    float invDet = 1.0f / det;

    result.m11 = +matrix.m22 * invDet;
    result.m12 = -matrix.m12 * invDet;

    result.m21 = -matrix.m21 * invDet;
    result.m22 = +matrix.m11 * invDet;

    result.m31 = (matrix.m21 * matrix.m32 - matrix.m31 * matrix.m22) * invDet;
    result.m32 = (matrix.m31 * matrix.m12 - matrix.m11 * matrix.m32) * invDet;

    return true;
}

Matrix3x2 Matrix3x2::CreateTranslation(const Vector2& position) noexcept
{
    return CreateTranslation(position.x, position.y);
}

Matrix3x2 Matrix3x2::CreateTranslation(float x, float y) noexcept
{
    Matrix3x2 result;
    result.m11 = 1.0f; result.m12 = 0.0f;
    result.m21 = 0.0f; result.m22 = 1.0f;
    result.m31 = x;
    result.m32 = y;
    return result;
}

Matrix3x2 Matrix3x2::CreateRotation(float degrees) noexcept
{
    return CreateRotation(degrees, Vector2::Zero);
}

Matrix3x2 Matrix3x2::CreateRotation(float degrees, const Vector2& centerPoint) noexcept
{
    float fSinAngle;
    float fCosAngle;
    SinCos(ToRadians(degrees), &fSinAngle, &fCosAngle);

    const float dx = (centerPoint.x * (1.0f - fCosAngle)) + (centerPoint.y * fSinAngle);
    const float dy = (centerPoint.y * (1.0f - fCosAngle)) - (centerPoint.x * fSinAngle);

    Matrix3x2 result;
    result.m11 = fCosAngle; result.m12 = fSinAngle;
    result.m21 = -fSinAngle; result.m22 = fCosAngle;
    result.m31 = dx;
    result.m32 = dy;
    return result;
}

Matrix3x2 Matrix3x2::CreateScale(const Vector2& scale) noexcept
{
    return CreateScale(scale.x, scale.y);
}

Matrix3x2 Matrix3x2::CreateScale(float scaleX, float scaleY) noexcept
{
    Matrix3x2 result;
    result.m11 = scaleX;
    result.m12 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = scaleY;

    result.m31 = 0.0f;
    result.m32 = 0.0f;
    return result;
}

Matrix3x2 Matrix3x2::CreateScale(float scale) noexcept
{
    return CreateScale(scale, scale);
}

Matrix3x2 Matrix3x2::CreateScale(const Vector2& scale, const Vector2& centerPoint) noexcept
{
    float tx = centerPoint.x * (1 - scale.x);
    float ty = centerPoint.y * (1 - scale.y);

    Matrix3x2 result;
    result.m11 = scale.x;
    result.m12 = 0.0f;

    result.m21 = 0.0f;
    result.m22 = scale.y;

    result.m31 = tx;
    result.m32 = ty;

    return result;
}

Matrix3x2 Matrix3x2::CreateScale(float scale, const Vector2& centerPoint) noexcept
{
    return CreateScale({ scale, scale }, centerPoint);
}

Matrix3x2 Matrix3x2::CreateSkew(float degreesX, float degreesY) noexcept
{
    Matrix3x2 result;
    result.m11 = 1.0f;
    result.m12 = MathF::Tan(ToRadians(degreesY));

    result.m21 = MathF::Tan(ToRadians(degreesX));
    result.m22 = 1.0f;

    result.m31 = 0.0f;
    result.m32 = 0.0f;

    return result;
}

Matrix3x2 Matrix3x2::CreateSkew(float degreesX, float degreesY, const Vector2& centerPoint) noexcept
{
    const float xTan = MathF::Tan(ToRadians(degreesX));
    const float yTan = MathF::Tan(ToRadians(degreesY));

    const float tx = -centerPoint.y * xTan;
    const float ty = -centerPoint.x * yTan;

    Matrix3x2 result;
    result.m11 = 1.0f;
    result.m12 = yTan;

    result.m21 = xTan;
    result.m22 = 1.0f;

    result.m31 = tx;
    result.m32 = ty;

    return result;
}

Matrix3x2 Matrix3x2::Lerp(const Matrix3x2& matrix1, const Matrix3x2& matrix2, float amount) noexcept
{
    Matrix3x2 result;
    result.m11 = MathF::Lerp(matrix1.m11, matrix2.m11, amount);
    result.m12 = MathF::Lerp(matrix1.m12, matrix2.m12, amount);

    result.m21 = MathF::Lerp(matrix1.m21, matrix2.m21, amount);
    result.m22 = MathF::Lerp(matrix1.m22, matrix2.m22, amount);

    result.m31 = MathF::Lerp(matrix1.m31, matrix2.m31, amount);
    result.m32 = MathF::Lerp(matrix1.m32, matrix2.m32, amount);

    return result;
}
