// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

partial class Platform
{
    static Platform()
    {
        ID = PlatformID.Android;
        Family = PlatformFamily.Mobile;
        DeviceFamily = $"Android.{Family}";
    }
}
