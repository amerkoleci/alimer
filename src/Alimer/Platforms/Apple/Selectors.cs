// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Platforms.Apple;

internal static class Selectors
{
    public static Selector Alloc => "alloc"u8;
    public static Selector Init => "init"u8;
    public static Selector Release => "release"u8;
    public static Selector Retain => "retain"u8;
    public static Selector RetainCount => "retainCount"u8;
}
