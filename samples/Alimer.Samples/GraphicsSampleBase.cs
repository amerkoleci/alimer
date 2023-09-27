// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Graphics;
using Alimer.Shaders;

namespace Alimer.Samples;

public abstract class GraphicsSampleBase : SampleBase
{
    protected GraphicsSampleBase(string name, IServiceRegistry services, Window mainWindow)
        : base(name)
    {
        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        MainWindow = mainWindow;
    }

    public IServiceRegistry Services { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public Window MainWindow { get; }
    public PixelFormat[] ColorFormats => new[] { MainWindow.ColorFormat };
    public PixelFormat DepthStencilFormat => MainWindow.DepthStencilFormat;
    public Texture? DepthStencilTexture => MainWindow.DepthStencilTexture;
    public float AspectRatio => MainWindow.AspectRatio;

    protected GraphicsBuffer CreateBuffer<T>(List<T> initialData,
       BufferUsage usage = BufferUsage.ShaderReadWrite,
       CpuAccessMode cpuAccess = CpuAccessMode.None)
       where T : unmanaged
    {
        Span<T> dataSpan = CollectionsMarshal.AsSpan(initialData);

        return GraphicsDevice.CreateBuffer(dataSpan, usage, cpuAccess);
    }

    protected ShaderStageDescription CompileShader(string fileName, string entryPoint, ShaderStage stage)
    {
        ShaderFormat shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.Combine(shadersPath, fileName);
        string shaderSource = File.ReadAllText(shaderSourceFileName);


        ShaderCompilationOptions options = new()
        {
            SourceFileName = shaderSourceFileName,
            ShaderStage = stage,
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
