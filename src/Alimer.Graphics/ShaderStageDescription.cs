// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Pipeline"/> single shader stage.
/// </summary>
public readonly record struct ShaderStageDescription
{
    public ShaderStageDescription(ShaderStage stage, byte[] byteCode, string entryPoint)
    {
        Guard.IsNotNullOrEmpty(entryPoint, nameof(entryPoint));
        Guard.IsTrue(stage != ShaderStage.All);

        Stage = stage;
        ByteCode = byteCode;
        EntryPoint = entryPoint;
    }

    public ShaderStage Stage { get; init; }

    public byte[] ByteCode { get; init; }
    public string EntryPoint { get; init; }
}
