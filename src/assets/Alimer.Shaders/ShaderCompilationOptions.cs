// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Shaders;

public struct ShaderCompilationOptions
{
    public string SourceFileName { get; set; } = string.Empty;
    public ShaderStages ShaderStage { get; set; } = ShaderStages.Vertex;
    public string EntryPoint { get; set; } = "main";
    public Dictionary<string, string> Defines { get; } = [];
    public List<string> IncludeDirs { get; } = [];

    public DxcShaderModel ShaderModel { get; set; } = DxcShaderModel.Model6_5;

    /// <summary>
    /// -Od
    /// </summary>
    public bool SkipOptimizations { get; set; }

    /// <summary>
    /// -Vd
    /// </summary>
    public bool SkipValidation { get; set; }

    public int OptimizationLevel { get; set; } = 3;

    public bool WarningsAreErrors { get; set; }

    /// <summary>
    /// -Ges
    /// </summary>
    public bool EnableStrictness { get; set; }

    /// <summary>
    /// -Zi
    /// </summary>
    public bool Debug { get; set; }
    public bool AllResourcesBound { get; set; }

    public bool PackMatrixRowMajor { get; set; }
    public bool PackMatrixColumnMajor { get; set; }

    public bool StripReflection { get; set; } = true;
    public bool StripDebug { get; set; } = true;

    //public StringView vulkanVersion = "1.3";

    public int SpvTargetEnvMajor { get; set; } = 1;
    public int SpvTargetEnvMinor { get; set; } = 2;

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
    public uint SpirvTShift { get; set; } = 100;

    /// <summary>
    /// SPIRV: register shift for UAV (u#) resources
    /// </summary>
    public uint SpirvUShift { get; set; } = 200;

    /// <summary>
    /// SPIRV: register shift for sampler (s#) resources
    /// </summary>
    public uint SpirvSShift { get; set; } = 300;

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
