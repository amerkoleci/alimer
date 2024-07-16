// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Shaders;

namespace Alimer.Shaders;

class Program
{
    public static int Main(string[] args)
    {
        string outputDirectory = args[0];
        if (!Directory.Exists(outputDirectory))
        {
            Directory.CreateDirectory(outputDirectory);
        }
        //else
        //{
        //    Directory.Delete(outputDirectory, true);
        //    Directory.CreateDirectory(outputDirectory);
        //}

        for (int i = 1; i < args.Length; i++)
        {
            string arg = args[i];

            Console.WriteLine($"Compile SPIRV: {arg}");
            Compile(outputDirectory, arg, ShaderFormat.SPIRV, ShaderStages.Vertex, "vertexMain");
            Compile(outputDirectory, arg, ShaderFormat.SPIRV, ShaderStages.Fragment, "fragmentMain");

            Console.WriteLine($"Compile DXIL: {arg}");
            Compile(outputDirectory, arg, ShaderFormat.DXIL, ShaderStages.Vertex, "vertexMain");
            Compile(outputDirectory, arg, ShaderFormat.DXIL, ShaderStages.Fragment, "fragmentMain");
        }


        return 0;
    }

    private static void Compile(string outputDirectory, string shaderSourceFileName,
        ShaderFormat shaderFormat, ShaderStages stage, string entryPoint)
    {
        string shadersPath = Path.GetDirectoryName(shaderSourceFileName);
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
