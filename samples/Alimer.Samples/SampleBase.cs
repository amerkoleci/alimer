// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Shaders;
using CommunityToolkit.Diagnostics;

namespace Alimer.Samples;

public abstract class SampleBase : IDisposable
{
    protected SampleBase(string name)
    {
        Guard.IsNotNullOrEmpty(name, nameof(name));

        Name = name;
    }

    public string Name { get; }

    /// <summary>
    /// Gets or sets the disposables.
    /// </summary>
    /// <value>The disposables.</value>
    protected DisposeCollector? DisposeCollector { get; set; }

    public virtual void Dispose()
    {
        DisposeCollector?.Dispose();
        DisposeCollector = null;
    }

    /// <summary>
    /// Adds a disposable object to the list of the objects to dispose.
    /// </summary>
    /// <param name="disposable">To dispose.</param>
    protected internal T ToDispose<T>(T disposable)
        where T : IDisposable
    {
        Guard.IsNotNull(disposable, nameof(disposable));

        DisposeCollector ??= new DisposeCollector();
        return DisposeCollector.Collect(disposable);
    }

    /// <summary>
    /// Dispose a disposable object and set the reference to null. Removes this object from the ToDispose list.
    /// </summary>
    /// <param name="disposable">Object to dispose.</param>
    protected internal void RemoveAndDispose<T>(ref T? disposable)
        where T : IDisposable
    {
        DisposeCollector?.RemoveAndDispose(ref disposable);
    }

    /// <summary>
    /// Removes a disposable object to the list of the objects to dispose.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="disposable">To dispose.</param>
    protected internal void RemoveToDispose<T>(T disposable)
        where T : IDisposable
    {
        Guard.IsNotNull(disposable, nameof(disposable));

        DisposeCollector?.Remove(disposable);
    }
}

public abstract class GraphicsSampleBase : SampleBase
{
    protected GraphicsSampleBase(string name, GraphicsDevice graphicsDevice, AppView mainView)
        : base(name)
    {
        GraphicsDevice = graphicsDevice;
        MainView = mainView;
    }

    public GraphicsDevice GraphicsDevice { get; }
    public AppView MainView { get; }
    public PixelFormat[] ColorFormats => new[] { MainView.ColorFormat };
    public PixelFormat DepthStencilFormat => MainView.DepthStencilFormat;

    public abstract void Draw(RenderContext context, Texture swapChainTexture);

    protected ShaderStageDescription CompileShader(string fileName, string entryPoint, ShaderStages stage)
    {
        ShaderFormat shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSource = File.ReadAllText(Path.Combine(shadersPath, fileName));

        ShaderStage shaderCompilerStage = ShaderStage.Vertex;
        switch (stage)
        {
            case ShaderStages.Vertex:
                shaderCompilerStage = ShaderStage.Vertex;
                break;

            case ShaderStages.Fragment:
                shaderCompilerStage = ShaderStage.Fragment;
                break;

            case ShaderStages.Compute:
                shaderCompilerStage = ShaderStage.Compute;
                break;

            default:
                throw new NotImplementedException();
        }

        ShaderCompilationOptions options = new()
        {
            ShaderStage = shaderCompilerStage,
            EntryPoint = entryPoint,
        };

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderFormat, shaderSource, in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        return new ShaderStageDescription(stage, result.GetByteCode().ToArray(), entryPoint);
    }
}
