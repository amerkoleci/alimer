// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Shaders;

public readonly struct ShaderCompilationOptions
{
    public ShaderStage ShaderStage { get; init; } = ShaderStage.Vertex;
    public string EntryPoint { get; init; } = "main";
    public Dictionary<string, string>? Defines { get; init; }
    public List<string>? IncludeDirs { get; init; }

    public DxcShaderModel ShaderModel { get; init; } = DxcShaderModel.Model6_5;

    public bool SkipOptimizations { get; init; }
    public int OptimizationLevel { get; init; } = 3;

    public bool WarningsAreErrors { get; init; }
    public bool AllResourcesBound { get; init; }

    public bool PackMatrixRowMajor { get; init; }
    public bool PackMatrixColumnMajor { get; init; }

    //public StringView vulkanVersion = "1.3";

    public int SpvTargetEnvMajor { get; init; } = 1;
    public int SpvTargetEnvMinor { get; init; } = 2;

    /// <summary>
    /// SPIRV: register shift for constant (b#) resources
    /// </summary>
    public uint ShiftSpaceCount { get; init; } = 8;

    /// <summary>
    /// SPIRV: register shift for constant (b#) resources
    /// </summary>
    public uint SpirvBShift { get; init; } = 0;

    /// <summary>
    /// SPIRV: register shift for texture (t#) resources
    /// </summary>
    public uint SpirvTShift { get; init; } = 100;

    /// <summary>
    /// SPIRV: register shift for UAV (u#) resources
    /// </summary>
    public uint SpirvUShift { get; init; } = 200;

    /// <summary>
    /// SPIRV: register shift for sampler (s#) resources
    /// </summary>
    public uint SpirvSShift { get; init; } = 300;

    public ShaderCompilationOptions()
    {

    }
}

public readonly record struct DxcShaderModel(int Major, int Minor)
{
    public static readonly DxcShaderModel Model6_0 = new(6, 0);
    public static readonly DxcShaderModel Model6_1 = new(6, 1);
    public static readonly DxcShaderModel Model6_2 = new(6, 2);
    public static readonly DxcShaderModel Model6_3 = new(6, 3);
    public static readonly DxcShaderModel Model6_4 = new(6, 4);
    public static readonly DxcShaderModel Model6_5 = new(6, 5);
    public static readonly DxcShaderModel Model6_6 = new(6, 6);
    public static readonly DxcShaderModel Model6_7 = new(6, 7);
    public static readonly DxcShaderModel Model6_8 = new(6, 8);
}
