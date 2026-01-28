// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
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
    private readonly GraphicsBuffer _spriteVertexBuffer;
    private readonly GraphicsBuffer _spriteIndexBuffer;

    public unsafe SpriteBatch(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        BufferDescriptor spriteBufferDesc = new((ulong)(sizeof(SpriteDrawData) * MaxBatchSize), BufferUsage.ShaderRead, MemoryType.Upload)
        {
            //spriteBufferDesc.stride = sizeof(SpriteDrawData);
            Label = "SpriteBatch Buffer"
        };
        _spriteVertexBuffer = device.CreateBuffer(in spriteBufferDesc);

        // Create the index buffer
        ReadOnlySpan<ushort> indices = [0, 1, 2, 3, 0, 2];
        _spriteIndexBuffer = device.CreateBuffer(indices, BufferUsage.Index, label: "SpriteBatch Index Buffer");
    }

    public GraphicsDevice Device { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="SpriteBatch" /> class.
    /// </summary>
    ~SpriteBatch() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _spriteVertexBuffer.Dispose();
            _spriteIndexBuffer.Dispose();
        }
    }
}
