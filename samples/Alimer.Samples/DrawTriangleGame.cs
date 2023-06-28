// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Engine;
using Alimer.Graphics;

namespace Alimer.Samples;

// https://github.com/dotnet/runtime/tree/main/src/tests/nativeaot
public sealed class DrawTriangleGame : GameApplication
{
    protected override void Initialize()
    {
        base.Initialize();

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));
        //
        ReadOnlySpan<VertexPositionColor> vertexData = stackalloc VertexPositionColor[] {
            new(new Vector3(0.0f, 0.5f, 0.5f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, -0.5f, 0.5f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
            new(new Vector3(-0.5f, -0.5f, 0.5f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
        };
        using GraphicsBuffer vertexBuffer = GraphicsDevice.CreateBuffer(vertexData, BufferUsage.Vertex);

        //
        //Entity cameraEntity = new Entity();
        //cameraEntity.GetOrCreate<CameraComponent>();
        //
        //Entity rootEntity = new Entity();
        //rootEntity.Children.Add(cameraEntity);
        //
        //SceneSystem.RootEntity = rootEntity;
    }

    protected override void Draw(AppTime time)
    {
        CommandBuffer commandBuffer = GraphicsDevice.BeginCommandBuffer(CommandQueue.Graphics, "Frame");
        using Texture? swapChainTexture = commandBuffer.AcquireSwapChainTexture(MainView.SwapChain!);
        if (swapChainTexture is not null)
        {
        //    commandBuffer.BeginRenderPass(swapChainTexture, new Vector4(0.3f, 0.3f, 0.3f, 1.0f));
        //    commandBuffer.EndRenderPass();
        }
        
        //GraphicsDevice.Submit(commandBuffer);
        commandBuffer.Commit();

        base.Draw(time);
    }

    public static void Main()
    {
        using DrawTriangleGame game = new();
        game.Run();
    }
}


[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly struct VertexPositionColor
{
    public static unsafe readonly int SizeInBytes = sizeof(VertexPositionColor);

    public VertexPositionColor(in Vector3 position, in Vector4 color)
    {
        Position = position;
        Color = color;
    }

    public readonly Vector3 Position;
    public readonly Vector4 Color;
}
