// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Rect.h"

namespace Alimer
{
    struct ALIMER_API Viewport
    {
    public:
        /// Specifies the x-coordinate of the viewport.
        float x;

        /// Specifies the y-coordinate of the viewport.
        float y;

        /// Specifies the width of the viewport.
        float width;

        /// Specifies the height of the viewport.
        float height;

        /// Specifies the minimum depth of the viewport. Ranges between 0 and 1.
        float minDepth;

        /// Specifies the maximum depth of the viewport. Ranges between 0 and 1.
        float maxDepth;

        constexpr Viewport() noexcept
            : x(0.0f), y(0.0f), width(0.0f), height(0.0f), minDepth(0.0f), maxDepth(1.0f)
        {
        }

        constexpr Viewport(float x_, float y_, float width_, float height_, float minDepth_ = 0.0f, float maxDepth_ = 1.0f) noexcept
            : x(x_), y(y_), width(width_), height(height_), minDepth(minDepth_), maxDepth(maxDepth_)
        {
        }

        explicit Viewport(const Rect& rect) noexcept
            : x(rect.x)
            , y(rect.y)
            , width(rect.width)
            , height(rect.height)
            , minDepth(0.0f)
            , maxDepth(1.0f)
        {
        }

        explicit Viewport(const Vector4& rect) noexcept
            : x(rect.x)
            , y(rect.y)
            , width(rect.z)
            , height(rect.w)
            , minDepth(0.0f)
            , maxDepth(1.0f)
        {
        }

        explicit Viewport(float width, float height) noexcept
            : x(0.0f)
            , y(0.0f)
            , width(width)
            , height(height)
            , minDepth(0.0f)
            , maxDepth(1.0f)
        {
        }

        Viewport(const Viewport&) = default;
        Viewport& operator=(const Viewport&) = default;
        Viewport(Viewport&&) = default;
        Viewport& operator=(Viewport&&) = default;

        bool operator ==(const Viewport& rhs) const noexcept
        {
            return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height && minDepth == rhs.minDepth && maxDepth == rhs.maxDepth;
        }

        bool operator != (const Viewport& rhs) const noexcept
        {
            return x != rhs.x || y != rhs.y || width != rhs.width || height != rhs.height || minDepth != rhs.minDepth || maxDepth != rhs.maxDepth;
        }

        // Viewport operations
        float GetAspectRatio() const noexcept
        {
            if (width == 0.f || height == 0.f)
                return 0.f;

            return (width / height);
        }

        /// Return as string.
        std::string ToString() const;
    };
}
