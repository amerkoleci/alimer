// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Shaders;

namespace Alimer.Graphics.Tests;

public static class TestUtilities
{
    public static ShaderStageDescription CompileShader(string fileName, string entryPoint, GraphicsBackendType backendType, ShaderStages stage)
    {
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSource = File.ReadAllText(Path.Combine(shadersPath, fileName));

        ShaderCompilationOptions options = new()
        {
            ShaderFormat = backendType == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL,
            ShaderStage = stage,
            EntryPoint = entryPoint,
#if DEBUG
            Debug = true,
#endif
        };

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderSource, in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        return new ShaderStageDescription(stage, result.GetByteCode().ToArray(), entryPoint);
    }
}
