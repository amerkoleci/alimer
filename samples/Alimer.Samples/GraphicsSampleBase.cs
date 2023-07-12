// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Shaders;

namespace Alimer.Samples;

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
    public float AspectRatio => MainView.AspectRatio;

    public abstract void Draw(RenderContext context, Texture swapChainTexture);

    protected ShaderStageDescription CompileShader(string fileName, string entryPoint, ShaderStages stage)
    {
        ShaderFormat shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.Combine(shadersPath, fileName);
        string shaderSource = File.ReadAllText(shaderSourceFileName);

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
            SourceFileName = shaderSourceFileName,
            ShaderStage = shaderCompilerStage,
            EntryPoint = entryPoint,
            IncludeDirs =
            {
                shadersPath
            }
        };

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderFormat, shaderSource, in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        return new ShaderStageDescription(stage, result.GetByteCode().ToArray(), entryPoint);
    }
}
