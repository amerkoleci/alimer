// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer;

partial class PlatformInfo
{
    static PlatformInfo()
    {
        DeviceFamily = $"{Environment.OSVersion.Platform}.Desktop";
        if (OperatingSystem.IsWindows())
        {
            DeviceFamily = "Windows.Desktop";
            ID = PlatformID.Windows;
        }
        else if (OperatingSystem.IsMacCatalyst())
        {
            ID = PlatformID.MacOS;
            IsMacCatalyst = true;
        }
        else if (OperatingSystem.IsMacOS())
        {
            ID = PlatformID.MacOS;
        }
        else if (OperatingSystem.IsAndroid())
        {
            ID = PlatformID.Android;
        }
        else if (OperatingSystem.IsIOS())
        {
            ID = PlatformID.iOS;
        }
        else if (OperatingSystem.IsBrowser())
        {
            ID = PlatformID.Browser;
        }
        else if (OperatingSystem.IsLinux())
        {
            ID = PlatformID.Linux;
        }
    }
}
