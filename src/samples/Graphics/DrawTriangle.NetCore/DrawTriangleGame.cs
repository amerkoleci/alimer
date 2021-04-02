// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice;
using Vortice.Graphics;
using Vortice.Graphics.D3D12;
using Vortice.Graphics.Vulkan;

namespace DrawTriangle
{
    public sealed class DrawTriangleGame : Application
    {
        public DrawTriangleGame()
            : base(new TestGameContext())
        {
        }

        class TestGameContext : NetStandardGameContext
        {
            protected override GraphicsDevice? CreateGraphicsDevice()
            {
                //PowerPreference powerPreference = PowerPreference.HighPerformance;

                if (OperatingSystem.IsWindows())
                {
                    if (GraphicsDevice.IsBackendSupported(BackendType.Direct3D12))
                    {
                        return GraphicsDevice.CreateSystemDefault(BackendType.Direct3D12);
                    }
                }

                // Just created best supported.
                return GraphicsDevice.CreateSystemDefault();
            }
        }
    }
}
