// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class ShaderModule : GraphicsObject
{
    protected ShaderModule(in ShaderModuleDescriptor description)
        : base()
    {
        Stage = description.Stage;
        EntryPoint = description.EntryPoint;
        //ByteCode = description.ByteCode.ToArray();
    }

    public ShaderStages Stage { get; }

    /// <summary>
    /// The name of the entry point function.
    /// </summary>
    public Utf8String EntryPoint { get; }
}
