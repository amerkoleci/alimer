// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Alimer;
using Alimer.Graphics;
using Alimer.Graphics.D3D12;
using Alimer.Graphics.Vulkan;
using Microsoft.Extensions.DependencyInjection;

namespace DrawTriangle
{
    public sealed class DrawTriangleGame : Game
    {
        class WindowsGameContext : NetStandardGameContext
        {
            private GraphicsDevice? graphicsDevice;

            public GraphicsDevice? GraphicsDevice { get => graphicsDevice ??= CreateGraphicsDevice(); set => graphicsDevice = value; }

            public override void ConfigureServices(IServiceCollection services)
            {
                base.ConfigureServices(services);

                services.AddSingleton(GraphicsDevice);
            }

            private static GraphicsDevice? CreateGraphicsDevice()
            {
                if (RuntimePlatform.PlatformType == PlatformType.Windows)
                {
                    if (D3D12GraphicsDevice.IsSupported())
                    {
                        return new D3D12GraphicsDevice();
                    }
                }

                return new VulkanGraphicsDevice(nameof(DrawTriangleGame));
            }
        }


        public DrawTriangleGame()
            : base(new WindowsGameContext())
        {
        }
    }
}
