// Copyright (c) Amer Koleci and Contributors.
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
        GraphicsManager = services.GetService<GraphicsManager>();
        GraphicsDevice = services.GetService<GraphicsDevice>();
        MainWindow = mainWindow;
    }

    public IServiceRegistry Services { get; }

    public GraphicsManager GraphicsManager { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public Window MainWindow { get; }
    public PixelFormat[] ColorFormats => [MainWindow.ColorFormat];
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

    protected Stream OpenEmbeddedAssetStream(string name) => typeof(GraphicsSampleBase).Assembly!.GetManifestResourceStream(name)!;
    protected byte[] ReadEmbeddedAssetBytes(string name)
    {
        using Stream stream = OpenEmbeddedAssetStream(name);
        byte[] bytes = new byte[stream.Length];
        using (MemoryStream ms = new MemoryStream(bytes))
        {
            stream.CopyTo(ms);
            return bytes;
        }
    }

    protected ShaderStageDescription LoadShader(string name, ShaderStages stage, string entryPoint)
    {
        string shaderFormat = GraphicsManager.BackendType == GraphicsBackendType.Vulkan ? "spirv" : "dxil";

        string entryName = $"{name}_{entryPoint}_{shaderFormat}.bin";
        byte[] bytecode = ReadEmbeddedAssetBytes(entryName);
        return new ShaderStageDescription(stage, bytecode, entryPoint);
    }

    protected ShaderStageDescription CompileShader(
        string fileName,
        string entryPoint,
        ShaderStages stage,
        Dictionary<string, string>? defines = default)
    {
        ShaderFormat shaderFormat = GraphicsManager.BackendType == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.ChangeExtension(Path.Combine(shadersPath, fileName), ".hlsl");
        string shaderSource = File.ReadAllText(shaderSourceFileName);

        ShaderCompilationOptions options = new()
        {
            SourceFileName = shaderSourceFileName,
            ShaderFormat = shaderFormat,
            ShaderStage = stage,
            EntryPoint = entryPoint,
            IncludeDirs =
            {
                shadersPath
            },
        };

        if (defines != null && defines.Count > 0)
        {
            foreach (KeyValuePair<string, string> pair in defines)
            {
                options.Defines.Add(pair.Key, pair.Value);
            }
        }

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderSource, in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        return new ShaderStageDescription(stage, result.GetByteCode().ToArray(), entryPoint);
    }
}
