// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets;
using Alimer.Graphics;
using Alimer.Input;
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
        AssetManager = services.GetService<IAssetManager>();
        Input = services.GetService<InputManager>();
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
    public InputManager Input { get; }
    public IAssetManager AssetManager { get; }

    protected void Resize()
    {
        if (DepthStencilFormat != PixelFormat.Undefined)
        {
            TextureDescriptor depthStencilTextureDesc = TextureDescriptor.Texture2D(DepthStencilFormat,
                (uint)MainWindow.SizeInPixels.Width, (uint)MainWindow.SizeInPixels.Height,
                usage: TextureUsage.RenderTarget
                );
            DepthStencilTexture = ToDispose(GraphicsDevice.CreateTexture(in depthStencilTextureDesc));
        }
    }

    protected Stream OpenEmbeddedAssetStream(string name) => typeof(GraphicsSampleBase).Assembly!.GetManifestResourceStream(name)!;
    protected byte[] ReadEmbeddedAssetBytes(string name)
    {
        using Stream stream = OpenEmbeddedAssetStream(name);
        byte[] bytes = new byte[stream.Length];
        using (MemoryStream ms = new(bytes))
        {
            stream.CopyTo(ms);
            return bytes;
        }
    }

    protected ShaderModule LoadShader(string name, ShaderStages stage, string entryPoint)
    {
        string shaderFormat = GraphicsDevice.Backend == GraphicsBackend.Vulkan ? "spirv" : "dxil";

        string entryName = $"{name}_{entryPoint}_{shaderFormat}.bin";
        byte[] bytecode = ReadEmbeddedAssetBytes(entryName);

        ShaderModuleDescriptor descriptor = new(stage, bytecode, entryPoint);
        return GraphicsDevice.CreateShaderModule(in descriptor);
    }

    protected ShaderModule CompileShaderModule(
        string fileName,
        ShaderStages stage,
        string entryPoint,
        Dictionary<string, string>? defines = default)
    {
        // https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/08-compiling.html
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.ChangeExtension(Path.Combine(shadersPath, fileName), ".hlsl");
        // DXC
        ShaderFormat shaderFormat = ShaderFormat.SPIRV;
        switch (GraphicsDevice.Backend)
        {
            case GraphicsBackend.D3D12:
                shaderFormat = ShaderFormat.DXIL;
                break;

            case GraphicsBackend.Metal:
                shaderFormat = ShaderFormat.Metal;
                break;

            case GraphicsBackend.Vulkan:
                shaderFormat = ShaderFormat.SPIRV;
                break;
        }

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
            PackMatrixRowMajor = true,
            ShiftSpaceCount = 4,
            SpirvBShift = VulkanRegisterShift.ContantBuffer,
            SpirvTShift = VulkanRegisterShift.SRV,
            SpirvUShift = VulkanRegisterShift.UAV,
            SpirvSShift = VulkanRegisterShift.Sampler
        };

        if (defines != null && defines.Count > 0)
        {
            foreach (KeyValuePair<string, string> pair in defines)
            {
                options.Defines.Add(pair.Key, pair.Value);
            }
        }

        string shaderSource = File.ReadAllText(shaderSourceFileName);
        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderSource, in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        ShaderModuleDescriptor descriptor = new(stage, result.GetByteCode(), entryPoint);
        return GraphicsDevice.CreateShaderModule(in descriptor);
    }
}
