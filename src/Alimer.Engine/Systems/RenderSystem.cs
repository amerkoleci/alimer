// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Vortice.Mathematics;

namespace Alimer.Engine;

public sealed class RenderSystem : EntitySystem<MeshComponent>
{
    private readonly Texture _blackTexture;
    private readonly Texture _whiteTexture;
    private readonly Texture _defaultNormalTexture;
    private readonly Sampler _defaultSampler;

    public RenderSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        Scene = services.GetService<SceneSystem>();
        MainWindow = services.GetService<Window>();

        _blackTexture = CreateTextureFromColor(Colors.Transparent);
        _whiteTexture = CreateTextureFromColor(Colors.White);
        _defaultNormalTexture = CreateTextureFromColor(new Color4(0.5f, 0.5f, 1.0f, 0f));
        _defaultSampler = GraphicsDevice.CreateSampler(SamplerDescriptor.LinearWrap);

        ColorFormat = MainWindow.SwapChain.ColorFormat;
        DepthStencilFormat = PixelFormat.Depth24UnormStencil8;
        SampleCount = TextureSampleCount.Count1; // 4u
        ResolutionMultiplier = 1;

        MainWindow.SizeChanged += OnCanvasSizeChanged;
        Resize(MainWindow.ClientSize.Width, MainWindow.ClientSize.Height);
    }

    public IServiceRegistry Services { get; }

    public GraphicsDevice GraphicsDevice { get; }
    public SceneSystem Scene { get; }
    public Window MainWindow { get; }

    public PixelFormat ColorFormat { get; }
    public PixelFormat DepthStencilFormat { get; }
    public TextureSampleCount SampleCount { get; }
    public int ResolutionMultiplier { get; } = 1;
    public int Width { get; private set; }
    public int Height { get; private set; }

    public bool ShadowsEnabled { get; set; } = true;
    public int ShadowResolutionMultiplier { get; set; } = 1;

    // Bloom
    public bool BloomEnabled { get; set; } = true;

    public Texture? MultisampleColorTexture { get; private set; }
    public Texture? DepthStencilTexture { get; private set; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            MultisampleColorTexture?.Dispose();
            DepthStencilTexture?.Dispose();

            _blackTexture.Dispose();
            _whiteTexture.Dispose();
            _defaultNormalTexture.Dispose();
            _defaultSampler.Dispose();
        }

        base.Dispose(disposing);
    }

    public override void Draw(RenderContext renderContext, Texture outputTexture, AppTime time)
    {
        Render(renderContext, outputTexture, Scene.CurrentCamera!);
    }

    public void Render(RenderContext renderContext, Texture output, CameraComponent camera)
    {
        UpdateCamera(camera);

        RenderPassColorAttachment colorAttachment = new(output, Colors.Black);

        RenderPassDescription outputPass = new(colorAttachment)
        {
            Label = "Output pass"
        };

        using (renderContext.PushScopedPassPass(outputPass))
        {
            //renderContext.SetBindGroup(0, camera.bindGroup);

            // Loop through all the renderable entities and store them by pipeline.
            //for (const pipeline of this.renderBatch.sortedPipelines) {
            //    passEncoder.setPipeline(pipeline.pipeline);
            //}
        }
    }

    public void Resize(int pixelWidth, int pixelHeight)
    {
        Width = pixelWidth * ResolutionMultiplier;
        Height = pixelHeight * ResolutionMultiplier;

        if (SampleCount > TextureSampleCount.Count1)
        {
            MultisampleColorTexture?.Dispose();

            TextureDescriptor desc = TextureDescriptor.Texture2D(ColorFormat, (uint)Width, (uint)Height, 1, 1, TextureUsage.RenderTarget, SampleCount, initialLayout: ResourceStates.DepthWrite);
            MultisampleColorTexture = GraphicsDevice.CreateTexture(in desc);
        }

        if (DepthStencilFormat != PixelFormat.Undefined &&
            DepthStencilFormat.IsDepthStencilFormat())
        {
            DepthStencilTexture?.Dispose();

            TextureDescriptor desc = TextureDescriptor.Texture2D(DepthStencilFormat, (uint)Width, (uint)Height, 1, 1, 
                usage: TextureUsage.RenderTarget, 
                initialLayout: ResourceStates.DepthWrite,
                sampleCount: SampleCount
                );
            DepthStencilTexture = GraphicsDevice.CreateTexture(in desc);
        }
    }

    private void OnCanvasSizeChanged(object? sender, EventArgs e)
    {
        Resize(MainWindow.ClientSize.Width, MainWindow.ClientSize.Height);
    }

    private void UpdateCamera(CameraComponent camera)
    {
    }

    private Texture CreateTextureFromColor(in Color4 color)
    {
        Span<uint> pixels = [(uint)color.ToRgba()];
        return GraphicsDevice.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 1, 1);
    }
}
