// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Vector4.h"

namespace Alimer
{
    /// Two-dimensional rectangle with 32 bit floating point components.
    struct ALIMER_API Rect
    {
    public:
        /// Specifies the x-coordinate of the rectangle.
        float x;
        /// Specifies the y-coordinate of the rectangle.
        float y;
        /// Specifies the width of the rectangle.
        float width;
        /// Specifies the height of the rectangle.
        float height;

        Rect() noexcept
            : x(0.0f)
            , y(0.0f)
            , width(0.0f)
            , height(0.0f)
        {
        }

        constexpr Rect(float x_, float y_, float width_, float height_) noexcept
            : x(x_)
            , y(y_)
            , width(width_)
            , height(height_)
        {
        }

        constexpr Rect(float width_, float height_) noexcept
            : x(0.0f)
            , y(0.0f)
            , width(width_)
            , height(height_)
        {
        }

        constexpr Rect(const Vector2& size) noexcept
            : x(0.0f)
            , y(0.0f)
            , width(size.x)
            , height(size.y)
        {
        }

        constexpr Rect(const Vector2& location, const Vector2& size) noexcept
            : x(location.x)
            , y(location.y)
            , width(size.x)
            , height(size.y)
        {
        }

        Rect(const Rect&) = default;
        Rect& operator=(const Rect&) = default;
        Rect(Rect&&) = default;
        Rect& operator=(Rect&&) = default;

        // Comparison operators
        bool operator == (const Rect& rhs) const noexcept { return (x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height); }
        bool operator != (const Rect& rhs) const noexcept { return (x != rhs.x) || (y != rhs.y) || (width != rhs.width) || (height != rhs.height); }

        void Offset(float ox, float oy) noexcept
        {
            x += ox;
            y += oy;
        }

        void Inflate(float horizAmount, float vertAmount) noexcept
        {
            x -= horizAmount;
            y -= vertAmount;
            width += horizAmount;
            height += vertAmount;
        }

        bool Contains(float ix, float iy) const noexcept
        {
            return (x <= ix) && (ix < (x + width)) && (y <= iy) && (iy < (y + height));
        }

        bool Contains(const Vector2& point) const noexcept
        {
            return (x <= point.x) && (point.x < x + width) && (y <= point.y) && (point.y < y + height);
        }

        bool Contains(const Rect& rect) const noexcept
        {
            return (x <= rect.x) && ((rect.x + rect.width) <= (x + width)) && (y <= rect.y) && ((rect.y + rect.height) <= (y + height));
        }

        bool Intersects(const Rect& rect) const noexcept
        {
            return (rect.x < (x + width)) && (x < (rect.x + rect.width)) && (rect.y < (y + height)) && (y < (rect.y + rect.height));
        }

        /// Return left coordinate.
        float Left() const noexcept { return x; }

        /// Return top coordinate.
        float Top() const noexcept { return y; }

        /// Return right coordinate.
        float Right() const noexcept { return x + width; }

        /// Return bottom coordinate.
        float Bottom() const noexcept { return y + height; }

        Vector2 Location() const noexcept { return Vector2(x, y); }
        Vector2 Center() const noexcept { return Vector2(x + (width / 2.0f), y+ (height / 2.0f)); }
        Vector2 Size() const noexcept { return Vector2(width, height); }

        /// Return float data.
        const float* Data() const { return &x; }

        /// Gets a value that indicates whether the rectangle is empty.
        bool IsEmpty() const noexcept { return (width == 0 && height == 0 && x == 0 && y == 0); }

        /// Return as a vector.
        Vector4 ToVector4() const { return Vector4(x, y, width, height); }

        /// Return as string.
        std::string ToString() const;

        /// Get hash code.
        size_t GetHashCode() const;

        static Rect Intersect(const Rect& ra, const Rect& rb) noexcept;

        static Rect Union(const Rect& ra, const Rect& rb) noexcept;

        /// Empty rectangle.
        static const Rect Empty;
    };
}

namespace std
{
    template <> struct hash<Alimer::Rect>
    {
        size_t operator()(const Alimer::Rect& value) const noexcept
        {
            return value.GetHashCode();
        }
    };
}
