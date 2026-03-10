// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CGFloat = System.Double;

namespace Alimer.Platforms.Apple;

public readonly record struct CGPoint
{
    public readonly CGFloat x;
    public readonly CGFloat y;

    public CGPoint(CGFloat x, CGFloat y)
    {
        this.x = x;
        this.y = y;
    }
}

public readonly record struct CGSize
{
    public readonly CGFloat width;
    public readonly CGFloat height;

    public CGSize(CGFloat width, CGFloat height)
    {
        this.width = width;
        this.height = height;
    }
}

public readonly record struct CGRect
{
    public readonly CGPoint origin;
    public readonly CGSize size;

    public CGRect(in CGPoint origin, in CGSize size)
    {
        this.origin = origin;
        this.size = size;
    }
}
