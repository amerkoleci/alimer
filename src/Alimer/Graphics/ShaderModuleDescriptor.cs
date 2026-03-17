// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="ShaderModule"/>.
/// </summary>
public ref struct ShaderModuleDescriptor
{
    public ShaderStages Stage;
    public ReadOnlySpan<byte> ByteCode;

    /// <summary>
    /// The name of the entry point.
    /// </summary>
    public string EntryPoint;

    /// <summary>
    /// Gets or sets the label of <see cref="ShaderModule"/>.
    /// </summary>
    public string? Label;

    public ShaderModuleDescriptor(ShaderStages stage, ReadOnlySpan<byte> byteCode)
    {
        Stage = stage;
        ByteCode = byteCode;
        EntryPoint = "main";
    }
    public ShaderModuleDescriptor(ShaderStages stage, ReadOnlySpan<byte> byteCode, string entryPoint)
    {
        Stage = stage;
        ByteCode = byteCode;
        EntryPoint = entryPoint;
    }
}
