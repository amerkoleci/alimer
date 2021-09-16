// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Math/Rect.h"
#include <spdlog/fmt/fmt.h>

namespace Alimer
{
    const Rect Rect::Empty(0.0f, 0.0f, 0.0f, 0.0f);

    Rect Rect::Intersect(const Rect& ra, const Rect& rb) noexcept
    {
        float righta = ra.x + ra.width;
        float rightb = rb.x + rb.width;

        float bottoma = ra.y + ra.height;
        float bottomb = rb.y + rb.height;

        float maxX = ra.x > rb.x ? ra.x : rb.x;
        float maxY = ra.y > rb.y ? ra.y : rb.y;

        float minRight = righta < rightb ? righta : rightb;
        float minBottom = bottoma < bottomb ? bottoma : bottomb;

        if ((minRight > maxX) && (minBottom > maxY))
        {
            Rect result;
            result.x = maxX;
            result.y = maxY;
            result.width = minRight - maxX;
            result.height = minBottom - maxY;
            return result;
        }

        return Rect::Empty;
    }

    Rect Rect::Union(const Rect& ra, const Rect& rb) noexcept
    {
        float righta = ra.x + ra.width;
        float rightb = rb.x + rb.width;

        float bottoma = ra.y + ra.height;
        float bottomb = rb.y + rb.height;

        float minX = ra.x < rb.x ? ra.x : rb.x;
        float minY = ra.y < rb.y ? ra.y : rb.y;

        float maxRight = righta > rightb ? righta : rightb;
        float maxBottom = bottoma > bottomb ? bottoma : bottomb;

        Rect result;
        result.x = minX;
        result.y = minY;
        result.width = maxRight - minX;
        result.height = maxBottom - minY;
        return result;
    }

    std::string Rect::ToString() const
    {
        return fmt::format("{} {} {} {}", x, y, width, height);
    }

    size_t Rect::GetHashCode() const
    {
        size_t hash = 0;
        Alimer::HashCombine(hash, x);
        Alimer::HashCombine(hash, y);
        Alimer::HashCombine(hash, width);
        Alimer::HashCombine(hash, height);
        return hash;
    }
}
