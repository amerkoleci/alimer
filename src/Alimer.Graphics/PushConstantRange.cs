// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a push constant range in <see cref="PipelineLayout"/>.
/// </summary>
public readonly record struct PushConstantRange
{
    [SetsRequiredMembers]
    public PushConstantRange(uint shaderRegister, uint size)
    {
        ShaderRegister = shaderRegister;
        Size = size;
    }

    [SetsRequiredMembers]
    public PushConstantRange(uint shaderRegister, int size)
    {
        ShaderRegister = shaderRegister;
        Size = (uint)size;
    }

    /// <summary>
    /// Register index to bind to (supplied in shader).
    /// </summary>
    public required uint ShaderRegister { get; init; }

    /// Size in bytes.
    public required uint Size { get; init; }

};
