﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Shaders;
using ShaderStage = Alimer.Graphics.ShaderStage;
using CompilerShaderStage = Alimer.Shaders.ShaderStage;

namespace Alimer.Samples;

public abstract class GraphicsSampleBase : SampleBase
{
    protected GraphicsSampleBase(string name, GraphicsDevice graphicsDevice, Window mainWindow)
        : base(name)
    {
        GraphicsDevice = graphicsDevice;
        MainWindow = mainWindow;
    }

    public GraphicsDevice GraphicsDevice { get; }
    public Window MainWindow { get; }
    public PixelFormat[] ColorFormats => new[] { MainWindow.ColorFormat };
    public PixelFormat DepthStencilFormat => MainWindow.DepthStencilFormat;
    public Texture? DepthStencilTexture => MainWindow.DepthStencilTexture;
    public float AspectRatio => MainWindow.AspectRatio;

    protected ShaderStageDescription CompileShader(string fileName, string entryPoint, ShaderStage stage)
    {
        ShaderFormat shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.Combine(shadersPath, fileName);
        string shaderSource = File.ReadAllText(shaderSourceFileName);

        CompilerShaderStage shaderCompilerStage = CompilerShaderStage.Vertex;
        switch (stage)
        {
            case ShaderStage.Vertex:
                shaderCompilerStage = CompilerShaderStage.Vertex;
                break;

            case ShaderStage.Fragment:
                shaderCompilerStage = CompilerShaderStage.Fragment;
                break;

            case ShaderStage.Compute:
                shaderCompilerStage = CompilerShaderStage.Compute;
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
