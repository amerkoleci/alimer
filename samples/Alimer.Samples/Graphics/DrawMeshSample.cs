// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using Alimer.Assets.Graphics;
using Alimer.Graphics;
using Alimer.Rendering;

namespace Alimer.Samples;

[Description("Graphics - Draw Mesh")]
public sealed unsafe class DrawMeshSample : GraphicsSampleBase
{
    private readonly Mesh _mesh;
    private readonly Texture _texture;
    private readonly RenderPipeline _renderPipeline;

    private Stopwatch _clock;

    public DrawMeshSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Mesh", services, mainWindow)
    {
        string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        string meshesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Meshes");

        //TextureImporter textureImporter = new();
        //TextureAsset textureAsset = textureImporter.Import(Path.Combine(texturesPath, "10points.png"), Services).Result;
        //_texture = ToDispose(textureAsset.CreateRuntime(GraphicsDevice));
        _texture = ToDispose(Texture.FromFile(GraphicsDevice, Path.Combine(texturesPath, "10points.png")));
        //_texture = ToDispose(Texture.FromFile(GraphicsDevice, Path.Combine(texturesPath, "environment.hdr")));

        MeshImporter meshImporter = new(GraphicsDevice);
        MeshMetadata meshMetadata = new()
        {
            FileFullPath = Path.Combine(meshesPath, "DamagedHelmet.glb")
        };
        MeshAsset meshAsset = meshImporter.Import(meshMetadata).Result;

        _mesh = ToDispose(meshAsset.Mesh);

        using ShaderModule vertexShader = CompileShaderModule("TexturedCube", ShaderStages.Vertex, "vertexMain");
        using ShaderModule fragmentShader = CompileShaderModule("TexturedCube", ShaderStages.Fragment, "fragmentMain");


        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new(VertexPositionNormalTexture.SizeInBytes, VertexPositionNormalTexture.VertexAttributes)
        };

        RenderPipelineDescriptor renderPipelineDesc = new(vertexBufferLayout, ColorFormats, DepthStencilFormat)
        {
            Label = "RenderPipeline",
            VertexShader = vertexShader,
            FragmentShader = fragmentShader
        };
        _renderPipeline = ToDispose(GraphicsDevice.CreateRenderPipeline(renderPipelineDesc));

        _clock = Stopwatch.StartNew();
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        float time = _clock.ElapsedMilliseconds / 1000.0f;
        Matrix4x4 world = Matrix4x4.CreateRotationX(time) * Matrix4x4.CreateRotationY(time * 2) * Matrix4x4.CreateRotationZ(time * .7f);

        Matrix4x4 view = Matrix4x4.CreateLookAt(new Vector3(0, 0, 25), new Vector3(0, 0, 0), Vector3.UnitY);
        Matrix4x4 projection = Matrix4x4.CreatePerspectiveFieldOfView((float)Math.PI / 4, AspectRatio, 0.1f, 100);
        Matrix4x4 viewProjection = Matrix4x4.Multiply(view, projection);
        Matrix4x4 worldViewProjection = Matrix4x4.Multiply(world, viewProjection);

        RenderPassColorAttachment colorAttachment = new(swapChainTexture.DefaultView!, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"u8
        };

        RenderPassEncoder renderPassEncoder = context.BeginRenderPass(backBufferRenderPass);
        renderPassEncoder.SetPipeline(_renderPipeline!);
        renderPassEncoder.SetDynamicConstantBuffer(0, worldViewProjection);

        PushConstants pushConstants = new()
        {
            textureIndex = _texture.DefaultView!.BindlessReadIndex
        };
        renderPassEncoder.SetPushConstants(pushConstants);

        _mesh.Draw(renderPassEncoder, 1u);
        renderPassEncoder.EndEncoding();
    }
    struct PushConstants
    {
        public int textureIndex;
    }
}
