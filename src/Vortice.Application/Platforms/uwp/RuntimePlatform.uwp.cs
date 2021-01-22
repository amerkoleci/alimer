// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.Extensions.DependencyInjection;
using Windows.System.Profile;
using Windows.UI.ViewManagement;

namespace Vortice
{
    partial class RuntimePlatform
    {
        static RuntimePlatform()
        {
            PlatformType = PlatformType.Uwp;

            switch (AnalyticsInfo.VersionInfo.DeviceFamily)
            {
                case "Windows.Mobile":
                    PlatformFamily = PlatformFamily.Mobile;
                    break;

                case "Windows.Universal":
                case "Windows.Desktop":
                    {
                        try
                        {
                            var uiMode = UIViewSettings.GetForCurrentView().UserInteractionMode;
                            if (uiMode == UserInteractionMode.Mouse)
                            {
                                PlatformFamily = PlatformFamily.Desktop;
                            }
                            else
                            {
                                PlatformFamily = PlatformFamily.Mobile;
                            }
                        }
                        catch (Exception ex)
                        {
                            Debug.WriteLine($"Unable to get device . {ex.Message}");
                        }
                    }
                    break;
                case "Windows.Xbox":
                case "Windows.Team":
                    PlatformFamily = PlatformFamily.Console;
                    break;

                case "Windows.Holographic":
                    break;
                case "Windows.IoT":
                    break;
            }

            FrameworkDescription = RuntimeInformation.FrameworkDescription;
            OSDescription = RuntimeInformation.OSDescription;
            DefaultAppDirectory = AppContext.BaseDirectory;
        }
    }
}
