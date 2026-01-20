// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Sampler"/>.
/// </summary>
public ref struct ShaderModuleDescriptor
{
    public ShaderStages Stage;
    public ReadOnlySpan<byte> ByteCode;

    /// <summary>
    /// The name of the entry point.
    /// </summary>
    public Utf8String EntryPoint;

    public ShaderModuleDescriptor(ShaderStages stage, ReadOnlySpan<byte> byteCode)
    {
        Stage = stage;
        ByteCode = byteCode;
        EntryPoint = "main"u8;
    }
    public ShaderModuleDescriptor(ShaderStages stage, ReadOnlySpan<byte> byteCode, Utf8String entryPoint)
    {
        Stage = stage;
        ByteCode = byteCode;
        EntryPoint = entryPoint;
    }
}
