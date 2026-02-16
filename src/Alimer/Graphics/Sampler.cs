// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Sampler : GraphicsObject, IGraphicsBindableResource
{
    protected Sampler(in SamplerDescriptor descriptor)
        : base(descriptor.Label)
    {
        MinFilter = descriptor.MinFilter;
        MagFilter = descriptor.MagFilter;
        MipFilter = descriptor.MipFilter;
        AddressModeU = descriptor.AddressModeU;
        AddressModeV = descriptor.AddressModeV;
        AddressModeW = descriptor.AddressModeW;
        LodMinClamp = descriptor.LodMinClamp;
        LodMaxClamp = descriptor.LodMaxClamp;
        CompareFunction = descriptor.CompareFunction;
        MaxAnisotropy = descriptor.MaxAnisotropy;
        ReductionType = descriptor.ReductionType;
        BorderColor = descriptor.BorderColor;
    }

    /// <summary>
    /// Gets the min filter of this <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MinFilter { get; }

    /// <summary>
    /// Gets the mag filter of this <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MagFilter { get; }

    /// <summary>
    /// Gets the mip filter of this <see cref="Sampler"/>
    /// </summary>
    public SamplerMipFilter MipFilter { get; }

    /// <summary>
    /// Gets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a u texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeU { get; }

    /// <summary>
    /// Gets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a v texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeV { get; }

    /// <summary>
    /// Gets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a w texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeW { get; }

    /// <summary>
    /// Gets the minimum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMinClamp { get; }

    /// <summary>
    /// Gets the maximum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMaxClamp { get; }

    /// <summary>
    /// Gets the <see cref="Graphics.CompareFunction"/> value that specifies a function that compares sampled data against existing sampled data.
    /// </summary>
    public CompareFunction CompareFunction { get; }

    /// <summary>
    /// Gets the number of samples that can be taken to improve the quality of sample footprints that are anisotropic.
    /// </summary>
    public ushort MaxAnisotropy { get; }

    /// <summary>
    /// Gets the <see cref="SamplerReductionType"/> value.
    /// </summary>
    public SamplerReductionType ReductionType { get; }

    /// <summary>
    /// Gets the <see cref="SamplerBorderColor"/> border color for clamped texture values.
    /// </summary>
    public SamplerBorderColor BorderColor { get; }
}
