// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Platforms.Apple;

internal static class Selectors
{
    public static Selector Alloc => "alloc";
    public static Selector Init => "init";
    public static Selector Release => "release";
    public static Selector Retain => "retain";
    public static Selector RetainCount => "retainCount";
}
