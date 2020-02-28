// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Windows.Media;

namespace Alimer
{
    internal sealed class WindowsRuntimePlatform : IRuntimePlatform
    {
        /// <inheritdoc/>
        public PlatformType PlatformType { get; }

        /// <inheritdoc/>
        public PlatformFamily PlatformFamily { get; }

        /// <inheritdoc/>
        public string FrameworkDescription => RuntimeInformation.FrameworkDescription;

        /// <inheritdoc/>
        public string OSDescription => RuntimeInformation.OSDescription;

        /// <inheritdoc/>
        public string DefaultAppDirectory => AppContext.BaseDirectory;

        public WindowsRuntimePlatform()
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
        }
    }
}
