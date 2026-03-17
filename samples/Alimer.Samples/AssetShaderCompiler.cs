// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Rendering;
using Alimer.Shaders;
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

        string entryPoint = "main";
        switch (stage)
        {
            case ShaderStages.Vertex:
                entryPoint = "vertexMain";
                break;
            case ShaderStages.Fragment:
                entryPoint = "fragmentMain";
                break;
            default:
                throw new NotSupportedException($"Shader stage '{stage}' is not supported.");
        }

        // TODO: ShaderLibrary (without -entry and -stage)
        bool useSlangCompiler = false;
        if (useSlangCompiler)
        {
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

                    for (int space = 0; space < ShiftSpaceCount; space++)
                    {
                        arguments.Add("-fvk-b-shift");
                        arguments.Add($"{VulkanRegisterShift.ContantBuffer}");
                        arguments.Add($"{space}");

                        arguments.Add("-fvk-t-shift");
                        arguments.Add($"{VulkanRegisterShift.SRV}");
                        arguments.Add($"{space}");

                        arguments.Add("-fvk-u-shift");
                        arguments.Add($"{VulkanRegisterShift.UAV}");
                        arguments.Add($"{space}");

                        arguments.Add("-fvk-s-shift");
                        arguments.Add($"{VulkanRegisterShift.Sampler}");
                        arguments.Add($"{space}");
                    }
                    break;
            }

            // bindless-space-index

            byte[] bytecode = SlangCompiler.CompileWithReflection([.. arguments], out SlangReflection reflection);
            //var result = SlangCompiler.Compile([.. arguments]);

            ShaderModuleDescriptor descriptor = new(stage, bytecode, entryPoint);
            return _system.Device.CreateShaderModule(in descriptor);
        }
        else
        {
            // DXC
            ShaderFormat shaderFormat = ShaderFormat.SPIRV;
            switch (_system.Backend)
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
                PackMatrixRowMajor = true,
                ShiftSpaceCount = 4,
                SpirvBShift = VulkanRegisterShift.ContantBuffer,
                SpirvTShift = VulkanRegisterShift.SRV,
                SpirvUShift = VulkanRegisterShift.UAV,
                SpirvSShift = VulkanRegisterShift.Sampler
            };

            if (_system.Paths.Count > 0)
            {
                options.IncludeDirs.AddRange(_system.Paths);
            }

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
            return _system.Device.CreateShaderModule(in descriptor);
        }
    }
}
