// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Graphics;

namespace Vortice.Samples;

public static class Program
{
    public static void Main(string[] args)
    {
        using DrawTriangleGame game = new(new SDL2GameContext(new D3D11GraphicsDevice()));
        //using DrawTriangleGame game = new(new SDL2GameContext(new D3D12GraphicsDevice()));
        //using DrawTriangleGame game = new(new SDL2GameContext(new VulkanGraphicsDevice()));
        game.Run();
    }
}
