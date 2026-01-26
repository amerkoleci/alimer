// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Shaders;

namespace Alimer.Graphics.Tests;

public static class TestUtilities
{
    public static ShaderModule CompileShader(GraphicsDevice device, string fileName, ShaderStages stage, Utf8String entryPoint)
    {
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSource = File.ReadAllText(Path.Combine(shadersPath, fileName));

        ShaderCompilationOptions options = new()
        {
            ShaderFormat = device.Backend == GraphicsBackend.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL,
            ShaderStage = stage,
            EntryPoint = entryPoint.ToString()!,
#if DEBUG
            Debug = true,
#endif
        };

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderSource, in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        ShaderModuleDescriptor descriptor = new(stage, result.GetByteCode(), entryPoint);
        return device.CreateShaderModule(in descriptor);
    }
}
