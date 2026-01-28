// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Rendering;
using Alimer.Samples.Graphics;
using Slangc.NET;

namespace Alimer.Samples;

public sealed class AssetShaderCompiler : IShaderCompiler
{
    private readonly ShaderSystem _system;

    public AssetShaderCompiler(ShaderSystem system)
    {
        _system = system;
    }

    public ShaderModule Compile(string fileName, ShaderStages stage, Dictionary<string, string>? defines = default)
    {
        string? shaderSourceFileName = default;
        string extension = ".slang";
        foreach (string path in _system.Paths)
        {
            string fullPath = Path.Combine(path, Path.ChangeExtension(fileName, extension));
            if (File.Exists(fullPath))
            {
                shaderSourceFileName = fullPath;
                break;
            }
        }

        if (string.IsNullOrEmpty(shaderSourceFileName))
        {
            throw new FileNotFoundException($"Shader source file '{fileName}' with extension '{extension}' not found in any of the specified paths.");
        }

        Utf8String entryPoint = "main"u8;
        switch (stage)
        {
            case ShaderStages.Vertex:
                entryPoint = "vertexMain"u8;
                break;
            case ShaderStages.Fragment:
                entryPoint = "fragmentMain"u8;
                break;
            default:
                throw new NotSupportedException($"Shader stage '{stage}' is not supported.");
        }

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

        if (_system.Paths.Count > 0)
        {
            foreach (string path in _system.Paths)
            {
                arguments.AddRange(["-I", path]);
            }
        }

        switch (_system.Backend)
        {
            case GraphicsBackend.D3D12:
                arguments.AddRange(["-profile", "sm_6_6", "-target", "dxil"]);
                break;

            case GraphicsBackend.Metal:
                arguments.AddRange(["-target", "metal"]);
                break;

            case GraphicsBackend.Vulkan:
                arguments.Add("-D__spirv__");

                arguments.AddRange([
                    "-fvk-use-dx-layout",
                    "-fvk-use-dx-position-w",
                    "-fvk-use-entrypoint-name",
                    "-target",
                    "spirv"]
                    );

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
        return _system.Device.CreateShaderModule(in descriptor);
    }
}
