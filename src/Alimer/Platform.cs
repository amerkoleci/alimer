// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if WINDOWS
using Windows.ApplicationModel;
using Windows.System;
#endif

namespace Alimer;

public static partial class Platform
{
    public static PlatformID ID { get; }
    public static PlatformFamily Family { get; }

    public static bool IsUnix { get; }

    public static bool IsWindows { get; }

    public static bool IsMacOS { get; }

    public static bool IsLinux { get; }

    public static bool IsArm { get; }

    public static bool IsFreeBSD { get; }

    public static bool IsAndroid { get; }

    /// <summary><c>true</c> if the current running in a 32-bit process; otherwise, <c>false</c>.</summary>
    public static readonly bool Is32BitProcess = Unsafe.SizeOf<nuint>() == 4;

    /// <summary><c>true</c> if the current running in a 64-bit process; otherwise, <c>false</c>.</summary>
    public static readonly bool Is64BitProcess = Unsafe.SizeOf<nuint>() == 8;

    static Platform()
    {
#if WINDOWS
        IsMacOS = false;
		IsLinux = false;
		IsUnix = false;
		IsWindows = true;
        IsFreeBSD = false;
        IsAndroid = false;

        // ProcessorArchitecture.X86OnArm64
		var arch = Package.Current.Id.Architecture;
		IsArm = arch == ProcessorArchitecture.Arm || arch == ProcessorArchitecture.Arm64;
#else
        IsMacOS = OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst() || OperatingSystem.IsIOS() || OperatingSystem.IsTvOS();
        IsLinux = OperatingSystem.IsLinux();
        IsUnix = IsMacOS || IsLinux;
        IsWindows = OperatingSystem.IsWindows();
        IsAndroid = OperatingSystem.IsAndroid();
        IsFreeBSD = OperatingSystem.IsFreeBSD();

        var arch = RuntimeInformation.ProcessArchitecture;
        IsArm = arch == Architecture.Arm || arch == Architecture.Arm64;
#endif
    }
}
