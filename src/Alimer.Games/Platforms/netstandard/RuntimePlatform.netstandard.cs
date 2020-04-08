// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;

namespace Alimer
{
    partial class RuntimePlatform
    {
        static RuntimePlatform()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                PlatformType = PlatformType.Windows;
                PlatformFamily = PlatformFamily.Desktop;
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                PlatformType = PlatformType.macOS;
                PlatformFamily = PlatformFamily.Desktop;
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                PlatformType = PlatformType.Linux;
                PlatformFamily = PlatformFamily.Desktop;
            }

            FrameworkDescription = RuntimeInformation.FrameworkDescription;
            OSDescription = RuntimeInformation.OSDescription;
            DefaultAppDirectory = AppContext.BaseDirectory;
        }
    }
}
