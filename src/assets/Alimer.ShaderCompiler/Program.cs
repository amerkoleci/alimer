// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Slangc.NET;

namespace Alimer.Shaders;

class Program
{
    public static int Main(string[] args)
    {
        string inputDirectory = args[0];

        string outputDirectory;
        if (args.Length > 1)
        {
            outputDirectory = args[1];
            
        }
        else
        {
            outputDirectory = Path.Combine(AppContext.BaseDirectory, "Compiled");

        }

        if (!Directory.Exists(outputDirectory))
        {
            Directory.CreateDirectory(outputDirectory);
        }
        //else
        //{
        //    Directory.Delete(outputDirectory, true);
        //    Directory.CreateDirectory(outputDirectory);
        //}
        Compile(inputDirectory, outputDirectory);

#if FILES
        for (int i = 2; i < args.Length; i++)
        {
            string arg = args[i];

            Console.WriteLine($"Compile SPIRV: {arg}");
            Compile(outputDirectory, arg, ShaderFormat.SPIRV, ShaderStages.Vertex, "vertexMain");
            Compile(outputDirectory, arg, ShaderFormat.SPIRV, ShaderStages.Fragment, "fragmentMain");

            Console.WriteLine($"Compile DXIL: {arg}");
            Compile(outputDirectory, arg, ShaderFormat.DXIL, ShaderStages.Vertex, "vertexMain");
            Compile(outputDirectory, arg, ShaderFormat.DXIL, ShaderStages.Fragment, "fragmentMain");
        } 
#endif


        return 0;
    }

    private static void Compile(string inputDirectory, string outputDirectory)
    {
        string[] shaderFiles = Directory.GetFiles(inputDirectory, "*.slang", SearchOption.AllDirectories);
        foreach (string shaderFile in shaderFiles)
        {
            Console.WriteLine($"Compile SPIRV: {shaderFile}");
            CompileSlang(outputDirectory, shaderFile, ShaderFormat.SPIRV, ShaderStages.Vertex);
            CompileSlang(outputDirectory, shaderFile, ShaderFormat.SPIRV, ShaderStages.Fragment);

            Console.WriteLine($"Compile DXIL: {shaderFile}");
            CompileSlang(outputDirectory, shaderFile, ShaderFormat.DXIL, ShaderStages.Vertex);
            CompileSlang(outputDirectory, shaderFile, ShaderFormat.DXIL, ShaderStages.Fragment);

            Console.WriteLine($"Compile Metal: {shaderFile}");
            CompileSlang(outputDirectory, shaderFile, ShaderFormat.Metal, ShaderStages.Vertex);
            CompileSlang(outputDirectory, shaderFile, ShaderFormat.Metal, ShaderStages.Fragment);
        }
    }

    // https://www.khronos.org/assets/uploads/developers/presentations/Vulkan_BOF_Using_Slang_with_Vulkan_SIGG24.pdf
    private static void CompileSlang(string outputDirectory, string shaderSourceFileName,
        ShaderFormat shaderFormat, ShaderStages stage, Dictionary<string, string>? defines = default)
    {
        string shadersPath = Path.GetDirectoryName(shaderSourceFileName)!;
        //string shaderSource = File.ReadAllText(shaderSourceFileName);

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

        //if (_system.Paths.Count > 0)
        //{
        //    foreach (string path in _system.Paths)
        //    {
        //        arguments.AddRange(["-I", path]);
        //    }
        //}
        arguments.AddRange(["-I", shadersPath]);

        switch (shaderFormat)
        {
            case ShaderFormat.DXIL:
                arguments.AddRange(["-profile", "sm_6_6", "-target", "dxil"]);
                break;

            case ShaderFormat.SPIRV:
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

            case ShaderFormat.Metal:
                arguments.AddRange(["-target", "metal"]);
                break;
        }

        byte[] byteCode = SlangCompiler.CompileWithReflection([.. arguments], out SlangReflection reflection);
        //var result = SlangCompiler.Compile([.. arguments]);

        reflection.Deserialize();

        string shaderFile = Path.GetFileNameWithoutExtension(shaderSourceFileName);
        string outputFile = Path.GetFileNameWithoutExtension(shaderFile) + "_" + entryPoint + "_" + shaderFormat.ToString().ToLower();
        switch (shaderFormat)
        {
            case ShaderFormat.DXIL:
                outputFile = Path.ChangeExtension(outputFile, ".bin");
                break;
            case ShaderFormat.SPIRV:
                outputFile = Path.ChangeExtension(outputFile, ".bin");
                break;
            case ShaderFormat.Metal:
                outputFile = Path.ChangeExtension(outputFile, ".bin");
                break;
        }

        File.WriteAllBytes(Path.Combine(outputDirectory, outputFile), byteCode);
    }

    private static void Compile(string outputDirectory, string shaderSourceFileName,
        ShaderFormat shaderFormat, ShaderStages stage, string entryPoint)
    {
        string shadersPath = Path.GetDirectoryName(shaderSourceFileName)!;
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

        //f (defines != null && defines.Count > 0)
        //
        //   foreach (KeyValuePair<string, string> pair in defines)
        //   {
        //       options.Defines.Add(pair.Key, pair.Value);
        //   }
        //

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderSource, in options);
        if (result.Failed)
        {
            Console.Error.WriteLine($"Shader compilation failed: {result.ErrorMessage}.");
            return;
            //throw new GraphicsException(result.ErrorMessage);
        }

        string shaderFile = Path.GetFileNameWithoutExtension(shaderSourceFileName);
        string outputFile = Path.GetFileNameWithoutExtension(shaderFile) + "_" + options.EntryPoint + "_" + options.ShaderFormat.ToString().ToLower();
        switch (options.ShaderFormat)
        {
            case ShaderFormat.DXIL:
                //outputFile = Path.ChangeExtension(outputFile, ".cso");
                outputFile = Path.ChangeExtension(outputFile, ".bin");
                break;
            case ShaderFormat.SPIRV:
                //outputFile = Path.ChangeExtension(outputFile, ".spv");
                outputFile = Path.ChangeExtension(outputFile, ".bin");
                break;
        }

        File.WriteAllBytes(Path.Combine(outputDirectory, outputFile), result.GetByteCode().ToArray());
    }
}
