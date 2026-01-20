// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Graphics;
using Alimer.Shaders;

namespace Alimer.Samples;

public abstract class GraphicsSampleBase : SampleBase
{
    protected GraphicsSampleBase(string name, IServiceRegistry services, Window mainWindow, PixelFormat depthStencilFormat = PixelFormat.Depth32Float)
        : base(name)
    {
        Services = services;
        GraphicsManager = services.GetService<GraphicsManager>();
        GraphicsDevice = services.GetService<GraphicsDevice>();
        MainWindow = mainWindow;
        DepthStencilFormat = depthStencilFormat;
        Resize();
    }

    public IServiceRegistry Services { get; }

    public GraphicsManager GraphicsManager { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public Window MainWindow { get; }
    public PixelFormat[] ColorFormats => [MainWindow.ColorFormat];
    public PixelFormat DepthStencilFormat { get; set; } = PixelFormat.Depth32Float;
    public Texture? DepthStencilTexture { get; private set; }
    public float AspectRatio => MainWindow.AspectRatio;

    protected void Resize()
    {
        if (DepthStencilFormat != PixelFormat.Undefined)
        {
            TextureDescriptor depthStencilTextureDesc = TextureDescriptor.Texture2D(DepthStencilFormat,
                (uint)MainWindow.ClientSize.Width, (uint)MainWindow.ClientSize.Height,
                usage: TextureUsage.RenderTarget
                );
            DepthStencilTexture = ToDispose(GraphicsDevice.CreateTexture(in depthStencilTextureDesc));
        }
    }

    protected GraphicsBuffer CreateBuffer<T>(List<T> initialData,
       BufferUsage usage = BufferUsage.ShaderReadWrite,
       MemoryType cpuAccess = MemoryType.Private)
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

    protected ShaderModule LoadShader(string name, ShaderStages stage, Utf8String entryPoint)
    {
        string shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? "spirv" : "dxil";

        string entryName = $"{name}_{entryPoint}_{shaderFormat}.bin";
        byte[] bytecode = ReadEmbeddedAssetBytes(entryName);

        ShaderModuleDescriptor descriptor = new(stage, bytecode, entryPoint);
        return GraphicsDevice.CreateShaderModule(in descriptor);
    }

    protected ShaderModule CompileShaderModule(
        string fileName,
        ShaderStages stage,
        Utf8String entryPoint,
        Dictionary<string, string>? defines = default)
    {
        ShaderFormat shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.ChangeExtension(Path.Combine(shadersPath, fileName), ".hlsl");
        string shaderSource = File.ReadAllText(shaderSourceFileName);

        ShaderCompilationOptions options = new()
        {
            SourceFileName = shaderSourceFileName,
            ShaderFormat = shaderFormat,
            ShaderStage = stage,
            EntryPoint = entryPoint.ToString()!,
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

        ShaderModuleDescriptor descriptor = new(stage, result.GetByteCode(), entryPoint);
        return GraphicsDevice.CreateShaderModule(in descriptor);
    }
}
