// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Graphics;

namespace Alimer.Rendering;

public struct SpriteTransform
{
    public Vector2 Position = Vector2.Zero;
    public Vector2 Scale = new(1.0f, 1.0f);
    public Vector2 SinCosRotation = new(0.0f, 1.0f);

    public SpriteTransform()
    {
    }

    public SpriteTransform(in Vector2 position)
    {
        Position = position;
    }

    public SpriteTransform(in Vector2 position, float rotation)
    {
        Position = position;
        SinCosRotation = new(float.Sin(rotation), float.Cos(rotation));
    }
}

public struct SpriteDrawData
{
    public SpriteTransform Transform;
    public Color Color;
    public Vector4 SourceRect;
}

public class SpriteBatch : DisposableObject
{
    private const uint MaxBatchSize = 1024;
    private readonly GraphicsBuffer _spriteBuffer;
    private readonly GraphicsBufferView _spriteBufferView;
    private readonly GraphicsBuffer _spriteIndexBuffer;
    private readonly RenderPipeline _renderPipeline;
    private RenderPassEncoder? _encoder;
    private Vector2 _viewportSize;

    public unsafe SpriteBatch(IServiceRegistry services, PixelFormat colorFormat, PixelFormat depthStencilFormat)
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        GraphicsDevice = services.GetService<GraphicsDevice>();
        ShaderSystem = services.GetService<ShaderSystem>();

        GraphicsBufferDescriptor spriteBufferDesc = new((ulong)(sizeof(SpriteDrawData) * MaxBatchSize), GraphicsBufferUsage.ShaderRead, MemoryType.Upload)
        {
            //spriteBufferDesc.stride = sizeof(SpriteDrawData);
            Label = "SpriteBatch Buffer"
        };
        _spriteBuffer = GraphicsDevice.CreateBuffer(in spriteBufferDesc);
        _spriteBufferView = _spriteBuffer.CreateStructuredView<SpriteDrawData>();

        // Create the index buffer
        ReadOnlySpan<ushort> indices = [0, 1, 2, 3, 0, 2];
        _spriteIndexBuffer = GraphicsDevice.CreateBuffer(indices, GraphicsBufferUsage.Index, label: "SpriteBatch Index Buffer");

        ShaderModule vertexShader = ShaderSystem.GetShaderModule("Sprite", ShaderStages.Vertex);
        ShaderModule fragmentShader = ShaderSystem.GetShaderModule("Sprite", ShaderStages.Fragment);
        RenderPipelineDescriptor renderPipelineDesc = new([colorFormat], depthStencilFormat)
        {
            Label = "SpriteBatch RenderPipeline",
            VertexShader = vertexShader,
            FragmentShader = fragmentShader,
            RasterizerState = RasterizerState.CullNone,
            BlendState = BlendState.NonPremultiplied,
            DepthStencilState = DepthStencilState.DepthDefault
        };
        _renderPipeline = GraphicsDevice.CreateRenderPipeline(renderPipelineDesc);
    }

    public GraphicsDevice GraphicsDevice { get; }
    public ShaderSystem ShaderSystem { get; }

    /// <summary>Finalizes an instance of the <see cref="SpriteBatch" /> class.</summary>
    ~SpriteBatch() => Dispose(disposing: false);

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _spriteBuffer.Dispose();
            _spriteIndexBuffer.Dispose();
            _renderPipeline.Dispose();
        }
    }

    public void Begin(RenderPassEncoder encoder, in Vector2 viewportSize)
    {
        _encoder = encoder;
        _viewportSize = viewportSize;

        encoder.SetPipeline(_renderPipeline);
        encoder.SetIndexBuffer(_spriteIndexBuffer, IndexFormat.Uint16, 0);
    }

    public unsafe void Render(Texture texture, in SpriteTransform transform, in Color color, in Vector4? drawRect = null)
    {
        //if (texture == nullptr)
        //    texture = &defaultTexture;

        SpriteDrawData drawData;
        drawData.Transform = transform;
        drawData.Color = color;
        if (drawRect.HasValue)
        {
            drawData.SourceRect = drawRect.Value;
        }
        else
        {
            drawData.SourceRect = new Vector4(0.0f, 0.0f, (float)texture.Width, (float)texture.Height);
        }

        RenderBatch(texture, new Span<SpriteDrawData>(&drawData, 1));
    }

    public void RenderBatch(Texture texture, Span<SpriteDrawData> drawData)
    {
        if (_encoder == null)
            throw new InvalidOperationException("SpriteBatch.Begin must be called before rendering.");

        if (drawData.Length == 0)
            return;


#if DEBUG
        // Make sure the draw rects are all valid
        for (int i = 0; i < drawData.Length; ++i)
        {
            Vector4 drawRect = drawData[i].SourceRect;
            Debug.Assert(drawRect.X >= 0 && drawRect.X < texture.Width);
            Debug.Assert(drawRect.Y >= 0 && drawRect.Y < texture.Height);
            Debug.Assert(drawRect.Z > 0 && drawRect.X + drawRect.Z <= texture.Width);
            Debug.Assert(drawRect.W > 0 && drawRect.Y + drawRect.W <= texture.Height);
        }
#endif

        GPUSpriteBatchData perBatchData = new()
        {
            ViewportSize = _viewportSize,
            TextureSize = new Vector2(texture.Width, texture.Height),
            SpriteBufferIndex = _spriteBufferView.BindlessReadIndex,
            SpriteTextureIndex = texture.DefaultView!.BindlessReadIndex
        };

        _encoder.SetPushConstants(perBatchData);

        uint numSpritesLeft = (uint)drawData.Length;
        for (uint offset = 0; offset < numSpritesLeft; offset += MaxBatchSize)
        {
            uint spritesToDraw = Math.Min(MaxBatchSize, numSpritesLeft);

            // Fill up the instance buffer
            //GPUAllocation instanceBuffer = context->AllocateGPU(spritesToDraw * sizeof(SpriteDrawData));
            //TempBuffer instanceBuffer = DX12::TempStructuredBuffer(spritesToDraw, sizeof(SpriteDrawData));
            //memcpy(instanceBuffer.data, drawData + offset, spritesToDraw * sizeof(SpriteDrawData));

            _spriteBuffer.SetData(drawData, offset);
            //NativeMemory.Copy(drawData + offset, _spriteBuffer.GetMappedData(), spritesToDraw * sizeof(SpriteDrawData));

            // Draw
            _encoder.DrawIndexed(6, spritesToDraw);

            numSpritesLeft -= spritesToDraw;
        }
    }
}
