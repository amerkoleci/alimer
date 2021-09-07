// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/MathHelper.h"

namespace Alimer
{
    /// Two-dimensional vector with 32 bit floating point components.
    struct ALIMER_API Vector2
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
            };

            float data[2];
        };

        Vector2() noexcept
            : x(0.0f)
            , y(0.0f)
        {
        }

        constexpr explicit Vector2(float value) noexcept
            : x(value)
            , y(value)
        {
        }

        constexpr Vector2(float x_, float y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        /// Construct from a float array.
        explicit Vector2(_In_reads_(2) const float* data)
            : x(data[0])
            , y(data[1])
        {
        }

        Vector2(const Vector2&) = default;
        Vector2& operator=(const Vector2&) = default;
        Vector2(Vector2&&) = default;
        Vector2& operator=(Vector2&&) = default;

        /// Add-assign a vector.
        Vector2& operator += (const Vector2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        /// Subtract-assign a vector.
        Vector2& operator -= (const Vector2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        /// Multiply-assign a scalar.
        Vector2& operator *= (float rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Vector2& operator *= (const Vector2& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }

        /// Divide-assign a scalar.
        Vector2& operator /= (float rhs)
        {
            float invRhs = 1.0f / rhs;
            x *= invRhs;
            y *= invRhs;
            return *this;
        }

        /// Divide-assign a vector.
        Vector2& operator /= (const Vector2& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        /// Normalize to unit length.
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x *= invLen;
                y *= invLen;
            }
        }

        /// Test for equality with another vector without epsilon.
        bool operator == (const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Vector2& rhs) const { return !(*this == rhs); }
        /// Add a vector.
        Vector2 operator + (const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }
        /// Return negation.
        Vector2 operator - () const { return Vector2(-x, -y); }
        /// Subtract a vector.
        Vector2 operator - (const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }
        /// Multiply with a scalar.
        Vector2 operator * (float rhs) const { return Vector2(x * rhs, y * rhs); }
        /// Multiply with a vector.
        Vector2 operator * (const Vector2& rhs) const { return Vector2(x * rhs.x, y * rhs.y); }
        /// Divide by a scalar.
        Vector2 operator / (float rhs) const { return Vector2(x / rhs, y / rhs); }
        /// Divide by a vector.
        Vector2 operator / (const Vector2& rhs) const { return Vector2(x / rhs.x, y / rhs.y); }

        /// Return length.
        float Length() const { return sqrtf(x * x + y * y); }
        /// Return squared length.
        float LengthSquared() const { return x * x + y * y; }
        /// Calculate dot product.
        float DotProduct(const Vector2& rhs) const { return x * rhs.x + y * rhs.y; }
        /// Calculate absolute dot product.
        float AbsDotProduct(const Vector2& rhs) const { return Alimer::Abs(x * rhs.x) + Alimer::Abs(y * rhs.y); }
        /// Return absolute vector.
        Vector2 Abs() const { return Vector2(Alimer::Abs(x), Alimer::Abs(y)); }
        /// Linear interpolation with another vector.
        Vector2 Lerp(const Vector2& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }
        /// Test for equality with another vector with epsilon.
        bool Equals(const Vector2& rhs) const { return Alimer::Equals(x, rhs.x) && Alimer::Equals(y, rhs.y); }
        /// Return whether is NaN.
        bool IsNaN() const { return Alimer::IsNaN(x) || Alimer::IsNaN(y); }

        /// Return normalized to unit length.
        Vector2 Normalized() const
        {
            float lenSquared = LengthSquared();
            if (!Alimer::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }

            return *this;
        }

        /// Return as string.
        std::string ToString() const;

        /// Zero vector.
        static const Vector2 Zero;
        /// (-1,0) vector.
        static const Vector2 Left;
        /// (1,0) vector.
        static const Vector2 Right;
        /// (0,1) vector.
        static const Vector2 Up;
        /// (0,-1) vector.
        static const Vector2 Down;
        /// (1,1) vector.
        static const Vector2 One;
    };

    /// Two-dimensional vector with 32 bit signed integer components
    struct ALIMER_API Int2
    {
    public:
        union
        {
            struct
            {
                /// X coordinate.
                int32_t x;
                /// Y coordinate.
                int32_t y;
            };

            int32_t data[2];
        };

        Int2() noexcept
            : x(0)
            , y(0)
        {
        }

        constexpr explicit Int2(int32_t value) noexcept
            : x(value)
            , y(value)
        {
        }

        constexpr Int2(int32_t x_, int32_t y_) noexcept
            : x(x_)
            , y(y_)
        {
        }

        /// Construct from a float array.
        explicit Int2(_In_reads_(2) const int32_t* data)
            : x(data[0])
            , y(data[1])
        {
        }

        Int2(const Int2&) = default;
        Int2& operator=(const Int2&) = default;

        Int2(Int2&&) = default;
        Int2& operator=(Int2&&) = default;

        /// Add-assign a vector.
        Int2& operator += (const Int2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        /// Subtract-assign a vector.
        Int2& operator -= (const Int2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        /// Multiply-assign a scalar.
        Int2& operator *= (int32_t rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }

        /// Multiply-assign a vector.
        Int2& operator *= (const Int2& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }

        /// Divide-assign a scalar.
        Int2& operator /= (int32_t scalar)
        {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        /// Divide-assign a vector.
        Int2& operator /= (const Int2& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        /// Test for equality with another vector without epsilon.
        bool operator == (const Int2& rhs) const { return x == rhs.x && y == rhs.y; }
        /// Test for inequality with another vector without epsilon.
        bool operator != (const Int2& rhs) const { return !(*this == rhs); }
        /// Add a vector.
        Int2 operator + (const Int2& rhs) const { return Int2(x + rhs.x, y + rhs.y); }
        /// Return negation.
        Int2 operator - () const { return Int2(-x, -y); }
        /// Subtract a vector.
        Int2 operator - (const Int2& rhs) const { return Int2(x - rhs.x, y - rhs.y); }
        /// Multiply with a scalar.
        Int2 operator * (int32_t rhs) const { return Int2(x * rhs, y * rhs); }
        /// Multiply with a vector.
        Int2 operator * (const Int2& rhs) const { return Int2(x * rhs.x, y * rhs.y); }
        /// Divide by a scalar.
        Int2 operator / (int32_t rhs) const { return Int2(x / rhs, y / rhs); }
        /// Divide by a vector.
        Int2 operator / (const Int2& rhs) const { return Int2(x / rhs.x, y / rhs.y); }

        /// Return as string.
        std::string ToString() const;

        /// Zero vector.
        static const Int2 Zero;
        /// (-1,0) vector.
        static const Int2 Left;
        /// (1,0) vector.
        static const Int2 Right;
        /// (0,1) vector.
        static const Int2 Up;
        /// (0,-1) vector.
        static const Int2 Down;
        /// (1,1) vector.
        static const Int2 One;
    };

    /// Multiply Vector2 with a scalar
    inline Vector2 operator * (float lhs, const Vector2& rhs) { return rhs * lhs; }

    /// Multiply Int2 with a scalar
    inline Int2 operator * (int32_t lhs, const Int2& rhs) { return rhs * lhs; }
}

namespace std
{
    template <>
    struct hash<Alimer::Vector2> {
        size_t operator()(const Alimer::Vector2& value) const noexcept {
            size_t h = 0;
            Alimer::HashCombine(h, value.x, value.y);
            return h;
        }
    };

    template <>
    struct hash<Alimer::Int2> {
        size_t operator()(const Alimer::Int2& value) const noexcept {
            size_t h = 0;
            Alimer::HashCombine(h, value.x, value.y);
            return h;
        }
    };
}
