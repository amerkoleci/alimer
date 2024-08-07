// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer;

partial class Platform
{
    static Platform()
    {
        ID = PlatformID.Windows;
        Family = PlatformFamily.Desktop;
        DeviceFamily = "Windows.Desktop";
    }
}
