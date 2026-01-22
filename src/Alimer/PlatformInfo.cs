// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer;

public static partial class PlatformInfo
{
    public static PlatformID ID { get; }
    public static PlatformFamily Family { get; }

    public static bool IsWindows => ID == PlatformID.Windows;
    public static bool IsIOS => ID == PlatformID.iOS;
    public static bool IsMacOS => ID == PlatformID.MacOS;
    public static bool IsMacCatalyst { get; }
    public static bool IsLinux => ID == PlatformID.Linux;
    public static bool IsAndroid => ID == PlatformID.Android;
    public static bool IsWinUI { get; }
    public static bool IsUnix => IsLinux || IsAndroid || IsMacOS || IsMacCatalyst;

    public static bool IsArm => ProcessArchitecture == Architecture.Arm || ProcessArchitecture == Architecture.Arm64;

    /// <summary>
	/// Gets a string that represents the type of device the application is running on.
	/// </summary>
    public static string DeviceFamily { get; }

    public static Architecture ProcessArchitecture { get; private set; } = RuntimeInformation.ProcessArchitecture;
}
