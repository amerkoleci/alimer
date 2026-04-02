// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.


namespace Alimer.Graphics;

public abstract class Sampler(in SamplerDescriptor descriptor) : GraphicsObject(descriptor.Label), IGraphicsBindableResource
{
    /// <summary>
    /// Gets the min filter of this <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MinFilter { get; } = descriptor.MinFilter;

    /// <summary>
    /// Gets the mag filter of this <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MagFilter { get; } = descriptor.MagFilter;

    /// <summary>
    /// Gets the mip filter of this <see cref="Sampler"/>
    /// </summary>
    public SamplerMipFilter MipFilter { get; } = descriptor.MipFilter;

    /// <summary>
    /// Gets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a u texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeU { get; } = descriptor.AddressModeU;

    /// <summary>
    /// Gets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a v texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeV { get; } = descriptor.AddressModeV;

    /// <summary>
    /// Gets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a w texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeW { get; } = descriptor.AddressModeW;

    /// <summary>
    /// Gets the minimum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMinClamp { get; } = descriptor.LodMinClamp;

    /// <summary>
    /// Gets the maximum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMaxClamp { get; } = descriptor.LodMaxClamp;

    /// <summary>
    /// Gets the <see cref="Graphics.CompareFunction"/> value that specifies a function that compares sampled data against existing sampled data.
    /// </summary>
    public CompareFunction CompareFunction { get; } = descriptor.CompareFunction;

    /// <summary>
    /// Gets the number of samples that can be taken to improve the quality of sample footprints that are anisotropic.
    /// </summary>
    public ushort MaxAnisotropy { get; } = descriptor.MaxAnisotropy;

    /// <summary>
    /// Gets the <see cref="SamplerReductionType"/> value.
    /// </summary>
    public SamplerReductionType ReductionType { get; } = descriptor.ReductionType;

    /// <summary>
    /// Gets the <see cref="SamplerBorderColor"/> border color for clamped texture values.
    /// </summary>
    public SamplerBorderColor BorderColor { get; } = descriptor.BorderColor;

    /// <summary>
    /// Gets the bindless index of the sampler.
    /// </summary>
    public abstract int BindlessIndex { get; }
}
