// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CGFloat = System.Double;

namespace Alimer.Platforms.Apple;

public readonly record struct CGPoint(CGFloat x, CGFloat y);

public readonly record struct CGSize(CGFloat width, CGFloat height);

public readonly record struct CGRect(in CGPoint origin, in CGSize size);
