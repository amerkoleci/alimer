// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Pipeline"/> single shader stage.
/// </summary>
public readonly record struct ShaderStageDescription
{
    public ShaderStageDescription(ShaderStages stage, byte[] byteCode, string entryPoint)
    {
        Guard.IsNotNullOrEmpty(entryPoint, nameof(entryPoint));
        Guard.IsTrue(stage != ShaderStages.All);

        Stage = stage;
        ByteCode = byteCode;
        EntryPoint = entryPoint;
    }

    public ShaderStages Stage { get; init; }

    public byte[] ByteCode { get; init; }
    public string EntryPoint { get; init; }
}
