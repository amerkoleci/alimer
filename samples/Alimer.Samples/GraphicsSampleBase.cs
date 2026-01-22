// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Assets;
using Alimer.Graphics;
using Alimer.Input;
using Alimer.Shaders;
using Slangc.NET;

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

    protected ShaderModule CompileShaderModuleNew(
        string fileName,
        ShaderStages stage,
        Utf8String entryPoint,
        Dictionary<string, string>? defines = default,
        string[]? searchPaths = null)
    {
        // https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/08-compiling.html
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSourceFileName = Path.ChangeExtension(Path.Combine(shadersPath, fileName), ".slang");

        // TODO: ShaderLibrary (without -entry and -stage)
        List<string> arguments =
        [
            shaderSourceFileName,
            "-entry", entryPoint.ToString()!,
            "-stage", stage.ToString().ToLowerInvariant(),
            "-matrix-layout-row-major",
            "-preserve-params"
        ];

        if (defines is not null)
        {
            foreach (KeyValuePair<string, string> definePair in defines)
            {
                arguments.AddRange(["-D", $"{definePair.Key}={definePair.Value}"]);
            }
        }

        if (searchPaths is not null)
        {
            foreach (string path in searchPaths)
            {
                arguments.AddRange(["-I", path]);
            }
        }


        switch (GraphicsDevice.Backend)
        {
            case GraphicsBackendType.D3D12:
                arguments.AddRange(["-profile", "sm_6_6", "-target", "dxil"]);
                break;

            case GraphicsBackendType.Metal:
                arguments.AddRange(["-target", "metal"]);
                break;

            case GraphicsBackendType.Vulkan:
                arguments.AddRange(["-fvk-use-dx-layout", "-fvk-use-dx-position-w", "-fvk-use-entrypoint-name", "-target", "spirv"]);



                const uint ShiftSpaceCount = 8;

                const uint SpirvBShift = 0;
                const uint SpirvTShift = 100;

                const uint SpirvUShift = 200;
                const uint SpirvSShift = 300;

                for (int space = 0; space < ShiftSpaceCount; space++)
                {
                    arguments.Add("-fvk-b-shift");
                    arguments.Add($"{SpirvBShift}");
                    arguments.Add($"{space}");

                    arguments.Add("-fvk-t-shift");
                    arguments.Add($"{SpirvTShift}");
                    arguments.Add($"{space}");

                    arguments.Add("-fvk-u-shift");
                    arguments.Add($"{SpirvUShift}");
                    arguments.Add($"{space}");

                    arguments.Add("-fvk-s-shift");
                    arguments.Add($"{SpirvSShift}");
                    arguments.Add($"{space}");
                }
                break;
        }

        byte[] bytecode = SlangCompiler.CompileWithReflection([.. arguments], out SlangReflection reflection);
        //var result = SlangCompiler.Compile([.. arguments]);

        reflection.Deserialize();

        ShaderModuleDescriptor descriptor = new(stage, bytecode, entryPoint);
        return GraphicsDevice.CreateShaderModule(in descriptor);
    }
}
