// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Windows.ApplicationModel;
using Windows.System;
using Windows.System.Profile;

namespace Alimer;

partial class Platform
{
    static Platform()
    {
        ID = PlatformID.Windows;

        switch (AnalyticsInfo.VersionInfo.DeviceFamily)
        {
            case "Windows.Desktop":
                Family = PlatformFamily.Desktop;
                break;
            case "Windows.Mobile":
                Family = PlatformFamily.Mobile;
                break;
            case "Windows.Xbox":
                Family = PlatformFamily.Console;
                break;
            case "Windows.Holographic":
                //Platform = PlatformType.WindowsHolographic;
                break;
            case "Windows.Team":
                //Platform = PlatformType.WindowsTeam;
                break;
            default:
                //Platform = PlatformType.WindowsIoT;
                break;
        }

        DeviceFamily = AnalyticsInfo.VersionInfo.DeviceFamily;
        IsWinUI = true;
        //ProcessArchitecture = Convert(Package.Current.Id.Architecture);
    }

#if TODO
    private static Architecture Convert(ProcessorArchitecture value)
    {
        return value switch
        {
            ProcessorArchitecture.X86 => Architecture.X86,
            ProcessorArchitecture.X64 => Architecture.X64,
            ProcessorArchitecture.Arm => Architecture.Arm,
            ProcessorArchitecture.Arm64 => Architecture.Arm64,
            ProcessorArchitecture.Neutral => (Architecture)9,
            ProcessorArchitecture.X86OnArm64 => (Architecture)9,
            ProcessorArchitecture.Unknown => (Architecture)9,
            _ => (Architecture)9,
        };
    } 
#endif
}
