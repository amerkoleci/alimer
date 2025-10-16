// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Alimer.Platforms.Apple;

internal static class Selectors
{
    public static readonly Selector Alloc = "alloc";
    public static readonly Selector Init = "init";
    public static readonly Selector Release = "release";
    public static readonly Selector Retain = "retain";
    public static readonly Selector RetainCount = "retainCount";
}
